/* depthFill_fp.glsl */

uniform sampler2D u_ColorMap;
#if defined(USE_ALPHA_TESTING)
uniform int u_AlphaTest;
#endif // USE_ALPHA_TESTING

varying vec2 var_Tex;
varying vec4 var_Color;

void main()
{
	vec4 color = texture2D(u_ColorMap, var_Tex);

#if defined(USE_ALPHA_TESTING)
	if (u_AlphaTest == ATEST_GT_0 && color.a <= 0.0)
	{
		discard;
		return;
	}
	else if (u_AlphaTest == ATEST_LT_128 && color.a >= 0.5)
	{
		discard;
		return;
	}
	else if (u_AlphaTest == ATEST_GE_128 && color.a < 0.5)
	{
		discard;
		return;
	}
#endif // USE_ALPHA_TESTING

	gl_FragColor = color * var_Color;
}
