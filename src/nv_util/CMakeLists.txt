PROJECT(nv_util)

SET(NV_UTIL_SRCS
	nv_ase.h nv_ase.cpp
	nv_unzip.h nv_unzip.cpp
	unzip.h unzip.c
	nv_util.h
	nv_util.cpp
	nv_tga.h nv_tga.cpp
	nv_jpeg.h nv_jpeg.cpp
	)

# targets
ADD_DEFINITIONS(-DNV_UTIL_EXPORTS)

IF(NV_UTIL_SHARED)
	ADD_DEFINITIONS(-DNV_UTIL_DLL=1)
	ADD_LIBRARY(nv_util SHARED ${NV_UTIL_SRCS})
ELSE(NV_UTIL_SHARED)
	ADD_LIBRARY(nv_util ${NV_UTIL_SRCS})
ENDIF(NV_UTIL_SHARED)



target_include_directories( nv_util PRIVATE   ${ZLIB_INCLUDE_DIR} ../nv_math/ ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/
						${NV_SOURCE_DIR}/extern/libtga/include ${NV_SOURCE_DIR}/extern/ ${JPEG_INCLUDE_PATH})
add_dependencies( nv_util nv_math libtga)

TARGET_LINK_LIBRARIES(nv_util ${LIBS} nv_math ${ZLIB_LIBRARY_RELEASE} libtga ${JPEG_LIBRARY})

INSTALL(TARGETS nv_util
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
