#-----------------------------------------------------------------
# Install GeoIP
#-----------------------------------------------------------------

message(STATUS "Installing GeoIP")

set(ETLEGACY_GEOIP_ARCHIVE "GeoIP.dat.tar.gz")
set(ETLEGACY_GEOIP_ARCHIVE_URL "https://mirror.etlegacy.com/geoip/GeoIP.dat.tar.gz")

LEG_DOWNLOAD(
	"GeoIP archive"
	"${ETLEGACY_GEOIP_ARCHIVE_URL}"
	"${CMAKE_CURRENT_BINARY_DIR}/legacy/${ETLEGACY_GEOIP_ARCHIVE}"
	FALSE
	"${CMAKE_CURRENT_BINARY_DIR}/legacy"
	"${CMAKE_CURRENT_BINARY_DIR}/legacy/GeoIP.dat"
)

message(STATUS "Adding GeoIP to installer scripts")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/legacy/GeoIP.dat"
	DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy"
)
