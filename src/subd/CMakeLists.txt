PROJECT(nv_subd)

SET(NV_SUB_SRCS
    cctantable.cpp
	subd.cpp
	tess.cpp
	subd.h
	subdinternal.h
	shader.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}) 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../3rth/)

# targets
ADD_DEFINITIONS(-DNV_SUB_EXPORTS)

IF(NV_SUB_SHARED)
    ADD_DEFINITIONS(-DNV_SUB_DLL=1)
    ADD_LIBRARY(nv_subd SHARED ${NV_SUB_SRCS})
ELSE(NV_SUB_SHARED)
    ADD_LIBRARY(nv_subd ${NV_SUB_SRCS})
ENDIF(NV_SUB_SHARED)

TARGET_LINK_LIBRARIES(nv_subd ${LIBS} opengl32)


INSTALL(TARGETS nv_subd
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
INSTALL(FILES subd.h DESTINATION include/subd)