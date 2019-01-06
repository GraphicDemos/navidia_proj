PROJECT(nv_math)

SET(NV_MATH_SRCS
    nv_mathdecl.h
	nv_math.h 
	nv_algebra.h nv_algebra.cpp
	affine_decomp.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# targets
ADD_DEFINITIONS(-DNV_MATH_EXPORTS)

IF(NV_MATH_SHARED)
    ADD_DEFINITIONS(-DNV_MATH_DLL=1)
    ADD_LIBRARY(nv_math SHARED ${NV_MATH_SRCS})
ELSE(NV_MATH_SHARED)
    ADD_LIBRARY(nv_math ${NV_MATH_SRCS})
ENDIF(NV_MATH_SHARED)

INSTALL(TARGETS nv_math
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
INSTALL(FILES nv_math.h nv_algebra.h nv_mathdecl.h DESTINATION include/nv_math)