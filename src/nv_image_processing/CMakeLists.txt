PROJECT(nv_image_processing)

SET(NV_IMAGE_PROCESSING_SRCS
	nv_image_processing_decl.h
    AssertCG.h
	AssertGL.h
	Buffer.h
	GaussFilter.h
	GaussFilter1D.h
	glh_extensions.h
	glh_genext.h
	Image.h
	ImageFilter.h
	ImageView.h
	LoadOperator.h
	SaveOperator.h
	SearchPath.h
	ShaderManager.h
	SinkOperator.h
	SourceOperator.h
	TwoPassGaussFilter.h
	wglExtensionsStringARB.h
	wglPBufferARB.h
	wglPixelFormatARB.h
	wglRenderTextureARB.h
	wglRenderTextureRectangleNV.h
	AssertCG.cpp
	AssertGL.cpp
	Buffer.cpp
	GaussFilter.cpp
	GaussFilter1D.cpp
	Image.cpp
	ImageFilter.cpp
	ImageView.cpp
	LoadOperator.cpp
	SaveOperator.cpp
	SearchPath.cpp
	ShaderManager.cpp
	SinkOperator.cpp
	TwoPassGaussFilter.cpp
	wglExtensionsStringARB.cpp
	wglPBufferARB.cpp
	wglPixelFormatARB.cpp
	wglRenderTextureARB.cpp
	Gauss.cg
	Gauss1D.cg
	Texture.cg
	VertexIdentity.cg
	)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}) 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/nv_image_processing)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
INCLUDE_DIRECTORIES( ${GLUT_INCLUDE_DIR})


#CG
IF(CG_FOUND)
	INCLUDE_DIRECTORIES(${CG_INCLUDE_DIR})
	SET(COMMON_LIBS ${CG_LIBRARY} ${CG_GL_LIBRARY})
ENDIF(CG_FOUND)

# targets
ADD_DEFINITIONS(-DNV_IMAGE_PROCESSING_EXPORTS)

IF(NV_IMAGE_PROCESSING_SHARED)
    ADD_DEFINITIONS(-DNV_IMAGE_PROCESSING_DLL=1)
    ADD_LIBRARY(nv_image_processing SHARED ${NV_IMAGE_PROCESSING_SRCS})
ELSE(NV_IMAGE_PROCESSING_SHARED)
    ADD_LIBRARY(nv_image_processing ${NV_IMAGE_PROCESSING_SRCS})
ENDIF(NV_IMAGE_PROCESSING_SHARED)


TARGET_LINK_LIBRARIES(nv_image_processing ${LIBS} opengl32.lib nv_dds ${COMMON_LIBS}  ${GLUT_glut_LIBRARY})

INSTALL(TARGETS nv_image_processing
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
INSTALL(FILES nv_image_processing_decl.h AssertCG.h AssertGL.h Buffer.h GaussFilter.h GaussFilter1D.h glh_extensions.h glh_genext.h Image.h ImageFilter.h ImageView.h LoadOperator.h SaveOperator.h SearchPath.h ShaderManager.h SinkOperator.h SourceOperator.h TwoPassGaussFilter.h wglExtensionsStringARB.h wglPBufferARB.h wglPixelFormatARB.h wglRenderTextureARB.h wglRenderTextureRectangleNV.h DESTINATION include/nv_image_processing)