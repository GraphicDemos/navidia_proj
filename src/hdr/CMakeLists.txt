PROJECT(nvHdr)

SET(HDR_SRCS
	hdrtexture.h hdrtexture.cpp	
	"../examples/shared/data_path.cpp"
	"../examples/shared/rgbe.c"
	)

# targets
ADD_DEFINITIONS(-DNVHDR_EXPORTS)

IF(NVHDR_SHARED)
    ADD_DEFINITIONS(-DNVHDR_DLL=1)
    ADD_LIBRARY(nvHdr SHARED ${HDR_SRCS})
ELSE(NVHDR_SHARED)
    ADD_LIBRARY(nvHdr ${HDR_SRCS})
ENDIF(NVHDR_SHARED)

target_include_directories(nvHdr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/    ${CMAKE_CURRENT_SOURCE_DIR}/../examples )

TARGET_LINK_LIBRARIES(nvHdr opengl32 ${GLUT_glut_LIBRARY} )

INSTALL(TARGETS nvHdr
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
INSTALL(FILES hdrtexture.h  DESTINATION include/nvHdr)