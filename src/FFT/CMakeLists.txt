PROJECT(nv_fft)

SET(NV_fft_SRCS
    FFT.h
	FFT.cpp
	FFTCg.cpp
	PingPong.cpp
	PingPong.h
	FFTEngine.cpp
	FFTfragmentX_CG1.h
	FFTfragmentX_CG2.h
	FFTfragmentY_CG1.h
	FFTfragmentY_CG2.h
	)


# targets
ADD_DEFINITIONS(-DNV_FFT_EXPORTS)

IF(NV_FFT_SHARED)
    ADD_DEFINITIONS(-DNV_FFT_DLL=1)
    ADD_LIBRARY(nv_fft SHARED ${NV_fft_SRCS})
ELSE(NV_FFT_SHARED)
    ADD_LIBRARY(nv_fft ${NV_fft_SRCS})
ENDIF(NV_FFT_SHARED)

target_include_directories( nv_fft PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../PBuffer 
				${CMAKE_CURRENT_SOURCE_DIR}/../../extern 
				${GLEW_INCLUDE_PATH} 
				${CG_INCLUDE_DIR}
				${GLUT_INCLUDE_DIR}
				)
				
TARGET_LINK_LIBRARIES(nv_fft ${LIBS} opengl32 nv_PBuffer ${CG_LIBRARY} ${GLEW_LIBRARY} ${CG_GL_LIBRARY}  ${GLUT_glut_LIBRARY})


INSTALL(TARGETS nv_fft
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
INSTALL(FILES FFT.h PingPong.h DESTINATION include/fft)