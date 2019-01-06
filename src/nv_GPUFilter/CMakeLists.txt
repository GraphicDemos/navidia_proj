PROJECT(nv_GPUFilter)

file(GLOB GPUFilter_INCLUDE	${CMAKE_CURRENT_SOURCE_DIR}/Applicator.h
						${CMAKE_CURRENT_SOURCE_DIR}/GLManager.h
						${CMAKE_CURRENT_SOURCE_DIR}/GPUFilter.h
						${CMAKE_CURRENT_SOURCE_DIR}/GPUFilterData.h
						${CMAKE_CURRENT_SOURCE_DIR}/MyGLShader.h
						${CMAKE_CURRENT_SOURCE_DIR}/PreviewManager.h
						${CMAKE_CURRENT_SOURCE_DIR}/ShaderSrc.h
						)

file(GLOB GPUFilter_SHADERS	${CMAKE_CURRENT_SOURCE_DIR}/shaders/GrayBlit_fs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/LABBlit_fs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/RGBBlit_fs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/Separable-Symmetric_fs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/Separable-Symmetric_vs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/standard_fs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/standard_vs.glsl
						${CMAKE_CURRENT_SOURCE_DIR}/shaders/build_shadersrc_header.pl
						)						
						
file(GLOB GPUFilter_SOURCES	"*.cpp")
file(GLOB GPUFilter_HEADERS	"*.h")


source_group("Example Filter" FILES ${CMAKE_CURRENT_SOURCE_DIR}/TestFilterData.h)
source_group("Includes" FILES ${GPUFilter_INCLUDE})
source_group("Shaders" FILES ${GPUFilter_SHADERS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../extern)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../extern/adobe_photoshop_cs5_sdk_win/pluginsdk/photoshopapi/photoshop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../extern/adobe_photoshop_cs5_sdk_win/pluginsdk/photoshopapi/pica_sp)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../extern/adobe_photoshop_cs5_sdk_win/pluginsdk/samplecode/common/includes)


# targets
ADD_DEFINITIONS(-DNV_GPUFILTER_EXPORTS)

IF(NV_GPUFILTER_SHARED)
    ADD_DEFINITIONS(-DNV_GPUFILTER_DLL=1)
    ADD_LIBRARY(nv_GPUFilter SHARED ${GPUFilter_SOURCES} ${GPUFilter_HEADERS} ${GPUFilter_SHADERS})
ELSE(NV_GPUFILTER_SHARED)
    ADD_LIBRARY(nv_GPUFilter ${GPUFilter_SOURCES} ${GPUFilter_HEADERS} ${GPUFilter_SHADERS})
ENDIF(NV_GPUFILTER_SHARED)

TARGET_LINK_LIBRARIES(nv_GPUFilter ${LIBS})


INSTALL(TARGETS nv_GPUFilter
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
#INSTALL(FILES nv_dds.h DESTINATION include/nv_dds)