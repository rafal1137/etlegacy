/* forwardLighting_fp.glsl */
/*
	VSM		Variance Shadow Mapping					// not implemented
	ESM		Exponential Shadow Maps					// not implemented
	EVSM	Exponential Variance Shadow Mapping
	PCF		Percentage-Closer Filtering
	PCSS	Percentage-Closer Soft Shadow			// not implemented
*/


/*
Some explanations by Marco Salvi about exponential shadow mapping:

Now you are filtering exponential values which rapidly go out of range,
to avoid this issue you can filter the logarithm of these values (and the go back to exp space)

For example if you averaging two exponential value such as exp(A) and exp(B) you have:

a*exp(A) + b*exp(B) (a and b are some filter weights)

but you can rewrite the same expression as:

exp(A) * (a + b*exp(B-A)) ,

exp(A) * exp( log (a + b*exp(B-A)))),

and:

exp(A + log(a + b*exp(B-A))

Now your sum of exponential is written as a single exponential, if you take the logarithm of it you can then just work on its argument:

A + log(a + b*exp(B-A))

Basically you end up filtering the argument of your exponential functions, which are just linear depth values,
so you enjoy the same range you have with less exotic techniques.
Just don't forget to go back to exp space when you use the final filtered value.


Though hardware texture filtering is not mathematically correct in log space it just causes a some overdarkening, nothing major.

If you have your shadow map filtered in log space occlusion is just computed like this (let assume we use bilinear filtering):

float occluder = tex2D( esm_sampler, esm_uv );
float occlusion = exp( occluder - receiver );

while with filtering in exp space you have:

float exp_occluder = tex2D( esm_sampler, esm_uv );
float occlusion = exp_occluder / exp( receiver );

EDIT: if more complex filters are used (trilinear, aniso, with mip maps) you need to generate mip maps using log filteirng as well.
*/

/*
float log_conv(float x0, float X, float y0, float Y)
{
    return (X + log(x0 + (y0 * exp(Y - X))));
}
*/


/*
source: http://en.wikipedia.org/wiki/Chebyshev%27s_inequality

X = distribution
mu = mean
sigma = standard deviation

=> then for any real number k > 0:

Pr(X -mu >= k * sigma) <= 1 / ( 1 + k^2)
*/




uniform mat4      u_ViewMatrix;
#if LIGHT_DIRECTIONAL==0
	uniform sampler2D u_AttenuationMapXY;
	uniform sampler2D u_AttenuationMapZ;
#endif // LIGHT_DIRECTIONAL

#if LIGHT_DIRECTIONAL==1
	#if !defined(r_ShowParallelShadowSplits)
	uniform sampler2D u_ShadowMap0;
	uniform sampler2D u_ShadowMap1;
	uniform sampler2D u_ShadowMap2;
	uniform sampler2D u_ShadowMap3;
	uniform sampler2D u_ShadowMap4;
	#endif
#elif defined(LIGHT_PROJ)
	uniform sampler2D u_ShadowMap0;
#else
	uniform samplerCube u_ShadowMap;
#endif

#if LIGHT_DIRECTIONAL==1
	uniform vec3 u_LightDir;
#else
	uniform vec3 u_LightOrigin;
#endif
uniform vec3  u_LightColor;
uniform float u_LightRadius;
uniform float u_LightScale; // note that this is NOT the cvar r_LightScale. This is the light->l.scale
uniform float u_LightWrapAround;

uniform mat4  u_ShadowMatrix[MAX_SHADOWMAPS];
uniform vec4  u_ShadowParallelSplitDistances;
uniform float u_ShadowTexelSize;
uniform float u_ShadowBlur;

varying vec4 var_Position;
varying vec4 var_Color;
varying vec3 var_Normal;
#if LIGHT_DIRECTIONAL==0
	varying vec4 var_TexAttenuation;
#endif // LIGHT_DIRECTIONAL
#if defined(USE_PORTAL_CLIPPING)
	varying float var_BackSide; // in front, or behind, the portalplane
#endif // USE_PORTAL_CLIPPING




// Given a normalized forward vector, create two other perpendicular vectors
void MakeNormalVectors(const vec3 forward, inout vec3 right, inout vec3 up)
{
	// this rotate and negate guarantees a vector not colinear with the original
	right.y = -forward.x;
	right.z = forward.y;
	right.x = forward.z;
	float d = dot(right, forward);
	right += forward * -d;
	normalize(right);
	up = cross(right, forward); // GLSL cross product is the same as in Q3A
}



//=======================================================================================================================
//=== unused code                vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//=======================================================================================================================
/*
float Rand(vec2 co)
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float Noise(vec2 co)
{
	return Rand(floor(co * 128.0));
}

vec3 RandomVec3(vec2 uv)
{
	vec3 dir;
//#if 1
	float r = Rand(uv);
//	float angle = M_TAU * r; // / 360.0;
float angle = r;// * M_TAU / 360.0;

	dir = normalize(vec3(cos(angle), sin(angle), r));
//#else
//	// dir = texture2D(u_NoiseMap, gl_FragCoord.st * r_FBufScale).rgb;
//	//dir = normalize(2.0 * (texture2D(u_RandomMap, uv).xyz - 0.5));
//#endif
	return dir;
}
*/


#if defined(PCSS)

#if LIGHT_DIRECTIONAL==1
// TODO SumBlocker for sun shadowing

#elif defined(LIGHT_PROJ)
float SumBlocker(vec4 shadowVert, float vertexDistance, float filterWidth, float samples)
{
	float stepSize = 2.0 * filterWidth / samples;

	float blockerCount = 0.0;
	float blockerSum   = 0.0;

	for (float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for (float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			float shadowDistance = texture2DProj(u_ShadowMap0, vec3(shadowVert.xy + vec2(i, j), shadowVert.w)).x;
			// float shadowDistance = texture2D(u_ShadowMap, shadowVert.xy / shadowVert.w + vec2(i, j)).x;

			// FIXME VSM_CLAMP

			if (vertexDistance > shadowDistance)
			{
				blockerCount += 1.0;
				blockerSum   += shadowDistance;
			}
		}
	}

	float result;
	if (blockerCount > 0.0)
	{
		result = blockerSum / blockerCount;
	}
	else
	{
		result = 0.0;
	}

	return result;
}

#else // case LIGHT_OMNI
float SumBlocker(vec3 I, float vertexDistance, float filterWidth, float samples)
{
	vec3 forward, right, up;

	forward = normalize(-I);
	MakeNormalVectors(forward, right, up);

	float stepSize = 2.0 * filterWidth / samples;

	float blockerCount = 0.0;
	float blockerSum   = 0.0;

	for (float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for (float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			float shadowDistance = textureCube(u_ShadowMap, I + right * i + up * j).x;

			if (vertexDistance > shadowDistance)
			{
				blockerCount += 1.0;
				blockerSum   += shadowDistance;
			}
		}
	}

	float result;
	if (blockerCount > 0.0)
	{
		result = blockerSum / blockerCount;
	}
	else
	{
		result = -1.0;
	}

	return result;
}
#endif

float EstimatePenumbra(float vertexDistance, float blocker)
{
	float penumbra;

	if (blocker == 0.0)
	{
		penumbra = 0.0;
	}
	else
	{
		penumbra = ((vertexDistance - blocker) * u_LightRadius) / blocker;
	}

	return penumbra;
}

#endif // PCSS
//=======================================================================================================================
//=== end unused code            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//=======================================================================================================================





#if defined(EVSM)
float ChebyshevUpperBound(vec2 shadowMoments, float vertexDistance, float minVariance)
{
	if (vertexDistance <= shadowMoments.x) return 1.0;

	float shadowDistance = shadowMoments.x;
	float shadowDistanceSquared = shadowMoments.y;

//!!!DEBUG!!! test:  https://fabiensanglard.net/shadowmappingVSM/
//Adjusting moments (this is sort of bias per pixel) using partial derivative
//float dx = dFdx(shadowDistance);
//float dy = dFdy(shadowDistance);
//shadowDistanceSquared += 0.25*(dx*dx+dy*dy);

	// compute variance
	float E_x2 = shadowDistanceSquared;
	float Ex_2 = shadowDistance * shadowDistance;

//	float variance = max(E_x2 - Ex_2, max(minVariance, VSM_EPSILON));
//	float variance = smoothstep(minVariance, 1.0, max(E_x2 - Ex_2, 0.0));
	float variance = E_x2 - Ex_2;
	variance = max(variance, minVariance);

	// compute probabilistic upper bound
	float d = vertexDistance - shadowDistance;
	float pMax = variance / (variance + (d * d));

//	#if defined(r_LightBleedReduction)
//	pMax = smoothstep(r_LightBleedReduction, 1.0, pMax);
//	#endif

	// one-tailed Chebyshev with k > 0
//	return (vertexDistance <= shadowDistance ? 1.0 : pMax);

	return pMax;
}

vec2 WarpDepth(float depth)
{
	// rescale depth into [-1, 1]
	depth = 2.0 * depth - 1.0;
	float pos = exp(r_EVSMExponents.x * depth);
	float neg = -exp(-r_EVSMExponents.y * depth);
	return vec2(pos, neg);
}

vec4 ShadowDepthToEVSM(float depth)
{
	vec2 warpedDepth = WarpDepth(depth);
	return vec4(warpedDepth.xy, warpedDepth.xy * warpedDepth.xy);
}

#endif // #if defined(EVSM)






#if LIGHT_DIRECTIONAL==1 //===========================================================================================

void FetchShadowMoments(vec3 Pworld, inout vec4 shadowVert, inout vec4 shadowMoments)
{
	// transform to camera space
	vec4 worldP = vec4(Pworld.xyz, 1.0);
	vec4 Pcam = u_ViewMatrix * worldP;
	float vertexDistanceToCamera = -Pcam.z;

#if !defined(r_ShowParallelShadowSplits)
	if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		shadowVert    = u_ShadowMatrix[0] * worldP;
//		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert);
//shadowMoments = texture2D(u_ShadowMap0, shadowVert.xy);
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
	{
		shadowVert    = u_ShadowMatrix[1] * worldP;
//		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert);
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
	{
		shadowVert    = u_ShadowMatrix[2] * worldP;
//		shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
		shadowMoments = texture2DProj(u_ShadowMap2, shadowVert);
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.w)
	{
		shadowVert    = u_ShadowMatrix[3] * worldP;
//		shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
		shadowMoments = texture2DProj(u_ShadowMap3, shadowVert);
	}
	else
	{
		shadowVert    = u_ShadowMatrix[4] * worldP;
//		shadowMoments = texture2DProj(u_ShadowMap4, shadowVert.xyw);
		shadowMoments = texture2DProj(u_ShadowMap4, shadowVert);
	}
#endif
	shadowMoments = ShadowDepthToEVSM(shadowMoments.x);
}

#if defined(SOFTSHADOWSAMPLES)

// shadowVert & shadowMoments are global variables.
// (variables are only output by FetchShadowMoments, but not used in PCF)
// If you define them locally in this function, with the same names, then the code does not run.
// To make the code run, i simply pass the variables on to both functions.
//
void PCF(vec3 Pworld, vec3 lightDir, inout vec4 shadowVert, inout vec4 shadowMoments, float filterWidth, float samples)
{
	vec3 forward, right, up;
	forward = normalize(Pworld);
	MakeNormalVectors(forward, right, up);
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);


#if 1
	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;
	for (float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for (float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			FetchShadowMoments(Pworld + right * i + up * j, shadowVert, shadowMoments);
			moments += shadowMoments;
		}
	}
#else
	for (int i = 0; i < samples; i++)
	{
		for (int j = 0; j < samples; j++)
		{
			vec3 rand = RandomVec3(gl_FragCoord.st * r_FBufScale + vec2(i, j)) * filterWidth;
			// rand.z = 0;
			// rand = normalize(rand) * filterWidth;

			FetchShadowMoments(Pworld + right * rand.x + up * rand.y, shadowVert, shadowMoments);
			moments += shadowMoments;
		}
	}
#endif

	// return average of the samples
	moments *= (1.0 / (samples * samples));
	shadowMoments = moments;
}
#endif // #if defined(SOFTSHADOWSAMPLES)



#elif defined(LIGHT_PROJ) //================================================================================================

vec4 FetchShadowMoments(vec2 st)
{
	return ShadowDepthToEVSM(texture2D(u_ShadowMap0, st).a);
}

#if defined(SOFTSHADOWSAMPLES)
vec4 PCF(vec4 shadowVert, float filterWidth, float samples)
{
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);

#if 1
	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;

	for (float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for (float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			moments += FetchShadowMoments(shadowVert.xy / shadowVert.w + vec2(i, j));
		}
	}
#else
	for (int i = 0; i < samples; i++)
	{
		for (int j = 0; j < samples; j++)
		{
			vec3 rand = RandomVec3(gl_FragCoord.st * r_FBufScale + vec2(i, j)) * filterWidth;
			// rand = vec3(0.0, 0.0, 1.0);
			// rand.z = 0;
			// rand = normalize(rand);// * filterWidth;

			moments += FetchShadowMoments(shadowVert.xy / shadowVert.w + rand.xy);
		}
	}
#endif

	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#endif // #if defined(SOFTSHADOWSAMPLES)

#else //== must be an omni =================================================================================================

vec4 FetchShadowMoments(vec3 I)
{
	return ShadowDepthToEVSM(textureCube(u_ShadowMap, I).a);
}

#if defined(SOFTSHADOWSAMPLES)
vec4 PCF(vec3 I, float filterWidth, float samples)
{
	vec3 forward, right, up;

	forward = normalize(I);
	MakeNormalVectors(forward, right, up);

	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);

#if 1
	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;

	for (float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for (float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			moments += FetchShadowMoments(I + right * i + up * j);
		}
	}
#else
	for (int i = 0; i < samples; i++)
	{
		for (int j = 0; j < samples; j++)
		{
			vec3 rand = RandomVec3(gl_FragCoord.st * r_FBufScale + vec2(i, j)) * filterWidth;
			// rand.z = 0;
			// rand = normalize(rand) * filterWidth;

			moments += FetchShadowMoments(I + right * rand.x + up * rand.y);
		}
	}
#endif

	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#endif // #if defined(SOFTSHADOWSAMPLES)

#endif //===================================================================================================================









void    main()
{
#if defined(USE_PORTAL_CLIPPING)
	if (var_BackSide < 0.0)
	{
		discard;
		return;
	}
#endif



	float shadow = 1.0;
#if defined(USE_SHADOWING)

	vec4 shadowVert;
#if defined(SOFTSHADOWSAMPLES)
	vec4 shadowMoments;
#endif


#if LIGHT_DIRECTIONAL==1 //===========================================================================================

#if defined(SOFTSHADOWSAMPLES)
//	PCF(var_Position.xyz, u_LightDir, shadowVert, shadowMoments, u_ShadowTexelSize * u_ShadowBlur * 750.0, SOFTSHADOWSAMPLES); // * big value for the sun
	PCF(var_Position.xyz, u_LightDir, shadowVert, shadowMoments, u_ShadowBlur * 0.3, SOFTSHADOWSAMPLES);
#else
	FetchShadowMoments(var_Position.xyz, shadowVert, shadowMoments);
#endif

// debug rendering
#if defined(r_ShowParallelShadowSplits)
	// transform to camera space
	vec4  Pcam                   = u_ViewMatrix * vec4(var_Position.xyz, 1.0);
	float vertexDistanceToCamera = -Pcam.z;

	if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
	{
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
	{
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
	}
	else if (vertexDistanceToCamera < u_ShadowParallelSplitDistances.w)
	{
		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
		return;
	}
	else
	{
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
	}
#endif // #if defined(r_ShowParallelShadowSplits)


#elif defined(LIGHT_PROJ) //================================================================================================

	shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);

	// compute incident ray
	vec3 I = var_Position.xyz - u_LightOrigin;

	const float SHADOW_BIAS    = 0.01;
	float       vertexDistance = length(I) / u_LightRadius - SHADOW_BIAS;

#if defined(SOFTSHADOWSAMPLES)
	shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, SOFTSHADOWSAMPLES);

	/*
	#elif defined(PCSS)

	// step 1: find blocker estimate

	float blockerSearchWidth = u_ShadowTexelSize * u_LightRadius / vertexDistance;
	float blockerSamples = 6.0; // how many samples to use for blocker search
	float blocker = SumBlocker(shadowVert, vertexDistance, blockerSearchWidth, blockerSamples);

	#if 0
	// uncomment to visualize blockers
	gl_FragColor = vec4(blocker * 0.3, 0.0, 0.0, 1.0);
	return;
	#endif

	// step 2: estimate penumbra using parallel planes approximation
	float penumbra = EstimatePenumbra(vertexDistance, blocker);

	#if 0
	// uncomment to visualize penumbrae
	gl_FragColor = vec4(0.0, 0.0, penumbra, 1.0);
	return;
	#endif

	// step 3: compute percentage-closer filter
	vec4 shadowMoments;
	if(penumbra > 0.0)
	{
	    const float PCFsamples = 4.0;


	    //float maxpen = PCFsamples * (1.0 / u_ShadowTexelSize);
	    //if(penumbra > maxpen)
	    //	penumbra = maxpen;
	    //

	    shadowMoments = PCF(shadowVert, penumbra, PCFsamples);
	}
	else
	{
	    shadowMoments = texture2DProj(u_ShadowMap, shadowVert.xyw);
	}
	*/
#else
	// no filter
	shadowMoments = FetchShadowMoments(shadowVert.xy / shadowVert.w);
#endif


#else //== must be omni ====================================================================================================

	// compute incident ray
	vec3 I = var_Position.xyz - u_LightOrigin;

	// if the surface is back-facing the light, discard
//	float dotNI = dot(var_Normal.xyz, -normalize(I));
	float dotNI = dot(var_Normal.xyz, -I); // no need to normalize I, just because of a dot-product test <= 0
	if (dotNI <= 0.0) {
//!!!DEBUG!!! DO NOT DISCARD. the usual color calculations must still be done..
		discard;
		return;
	}


#if defined(SOFTSHADOWSAMPLES)
	shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), SOFTSHADOWSAMPLES);
#else
	// no filtering
	shadowMoments = FetchShadowMoments(I);
#endif



#endif //===================================================================================================================



#if defined(EVSM)
	{
		const float SHADOW_BIAS = 0.01;

#if LIGHT_DIRECTIONAL==1
//		float vertexDistance = shadowVert.z - 0.0001;
		float vertexDistance = shadowVert.z;
#else
		float vertexDistance = (length(I) / u_LightRadius) - SHADOW_BIAS;
#endif

		vec2 warpedVertexDistances = WarpDepth(vertexDistance);

		// derivative of warping at depth
		vec2 depthScale  = VSM_EPSILON * r_EVSMExponents * warpedVertexDistances;
		vec2 minVariance = depthScale * depthScale;

		float posContrib = ChebyshevUpperBound(shadowMoments.xz, warpedVertexDistances.x, minVariance.x); // xz = depth, depth^2
		float negContrib = ChebyshevUpperBound(shadowMoments.yw, warpedVertexDistances.y, minVariance.y);

		shadow = min(posContrib, negContrib);

		#if defined(r_DebugShadowMaps)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (r_DebugShadowMaps & 1) != 0 ? posContrib : 0.0;
		gl_FragColor.g = (r_DebugShadowMaps & 2) != 0 ? negContrib : 0.0;
		gl_FragColor.b = (r_DebugShadowMaps & 4) != 0 ? shadow : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#endif
	}
#endif // EVSM






	if (shadow <= 0.0)
	{
//!!!DEBUG!!! DO NOT DISCARD. the usual color calculations must still be done..
		discard;
		return;
	}

#endif // USE_SHADOWING



	// compute light attenuation
#if defined(LIGHT_PROJ)
	vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, var_TexAttenuation.xyw).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(var_TexAttenuation.z + 0.5, 0.0)).rgb; // FIXME
#elif LIGHT_DIRECTIONAL==1
//	vec3 attenuationXY = vec3(1.0);
//	vec3 attenuationZ  = vec3(1.0);
#else // omni
	vec3 attenuationXY = texture2D(u_AttenuationMapXY, var_TexAttenuation.xy).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(var_TexAttenuation.z, 0)).rgb;
#endif


	// compute final color
	vec4 color = vec4(u_LightColor, 1.0);

#if LIGHT_DIRECTIONAL==0
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
#endif

	color.rgb *= u_LightScale;
	color.rgb *= shadow;

	gl_FragColor = color;
}
