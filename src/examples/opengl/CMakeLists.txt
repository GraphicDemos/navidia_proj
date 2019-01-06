

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
##Globe include and link

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../3rth
					${CMAKE_CURRENT_SOURCE_DIR}/../../
					${CMAKE_CURRENT_SOURCE_DIR}/../
					${CMAKE_CURRENT_SOURCE_DIR}
				)

add_compile_options(-D _XKEYCHECK_H)						

# ================================================= AntiAliasingWithPostProcessing 1D =================================================
add_executable(AntiAliasingWithPostProcessing "AntiAliasingWithPostProcessing.cpp")
add_dependencies(AntiAliasingWithPostProcessing nv_png nvparse)
source_group("Source Files" FILES "AntiAliasingWithPostProcessing.cpp")

set_target_properties(AntiAliasingWithPostProcessing PROPERTIES PROJECT_LABEL "AntiAliasingWithPostProcessing")						# in Visual Studio
set_target_properties(AntiAliasingWithPostProcessing PROPERTIES OUTPUT_NAME "AntiAliasingWithPostProcessing")
set_target_properties(AntiAliasingWithPostProcessing PROPERTIES FOLDER "examples/opengl")

target_include_directories(AntiAliasingWithPostProcessing PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse)
# Properties->Linker->Input->Additional Dependencies
target_link_libraries(AntiAliasingWithPostProcessing nv_png nvparse)
#install
install(TARGETS AntiAliasingWithPostProcessing RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= ambient_occlusion =================================================
add_executable(ambient_occlusion "ambient_occlusion.cpp" "../shared/pbuffer.h" 
								"../shared/pbuffer.cpp" 
								"../shared/NvIOModel.cpp"
								"../shared/NvIOModel.h" 
								"../shared/accpersp.h" 
								"../shared/accpersp.cpp" )
						
add_dependencies(ambient_occlusion nv_manip nv_math nvIO)

set_target_properties(ambient_occlusion PROPERTIES PROJECT_LABEL "ambient_occlusion")						# in Visual Studio
set_target_properties(ambient_occlusion PROPERTIES OUTPUT_NAME "ambient_occlusion")
set_target_properties(ambient_occlusion PROPERTIES FOLDER "examples/opengl")

target_include_directories(ambient_occlusion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../nvmath ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math)
# Properties->Linker->Input->Additional Dependencies
target_link_libraries(ambient_occlusion nv_manip nv_math  nvIO ${GLUT_glut_LIBRARY})
#install
install(TARGETS ambient_occlusion RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= bump_mapping =================================================
add_executable(bump_mapping 			"bump_mapping.cpp" 
										"../shared/read_text_file.h" 
										"../shared/read_text_file.cpp"  
										"../shared/bumpmap_to_normalmap.h" 
										"../shared/bumpmap_to_normalmap.cpp" 
										"../shared/data_path.h" 
										"../shared/data_path.cpp"  
										"../shared/array_texture.h" 
										"../shared/array_texture.cpp")
add_dependencies(bump_mapping  nv_png )
set_target_properties(bump_mapping PROPERTIES PROJECT_LABEL "bump_mapping")						# in Visual Studio
set_target_properties(bump_mapping PROPERTIES OUTPUT_NAME "bump_mapping")
set_target_properties(bump_mapping PROPERTIES FOLDER "examples/opengl")
target_include_directories(bump_mapping PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png)
# Properties->Linker->Input->Additional Dependencies
target_link_libraries(bump_mapping nv_png ${GLUT_glut_LIBRARY} )
#install
install(TARGETS bump_mapping RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= bump_reflect_local_light =================================================
add_executable(bump_reflect_local_light 
				"bump_reflect_local_light.cpp" 
				"../shared/read_text_file.h" 
				"../shared/read_text_file.cpp" 
				"../shared/bumpmap_to_normalmap.h" 
				"../shared/bumpmap_to_normalmap.cpp" 
			    "../shared/array_texture.h" 
				"../shared/array_texture.cpp")
add_dependencies(bump_reflect_local_light  nv_png nvparse )
set_target_properties(bump_reflect_local_light PROPERTIES PROJECT_LABEL "bump_reflect_local_light")						# in Visual Studio
set_target_properties(bump_reflect_local_light PROPERTIES OUTPUT_NAME "bump_reflect_local_light")
set_target_properties(bump_reflect_local_light PROPERTIES FOLDER "examples/opengl")
target_include_directories(bump_reflect_local_light PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png)
target_link_libraries(bump_reflect_local_light nv_png nvparse )
#instal
install(TARGETS bump_reflect_local_light RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= bumpy_shiny_patch =================================================
add_executable(bumpy_shiny_patch "bump_teapot.cpp" "bump_teapot.h"  "bumpy_shiny_patch.cpp" "dynamic_normalmap.h" "dynamic_normalmap.cpp"
				"../shared/read_text_file.h" 
				"../shared/read_text_file.cpp"  
				"../shared/bumpmap_to_normalmap.h" 
				"../shared/bumpmap_to_normalmap.cpp" 
				"../shared/array_texture.h" 
				"../shared/array_texture.cpp" 
				"../shared/data_path.h" 
				"../shared/data_path.cpp" 
				"../shared/load_cubemap.h" 
			    "../shared/cubemap_borders.cpp" 
			    "../shared/cubemap_borders.h" 
			    "../shared/load_cubemap.cpp")
add_dependencies(bumpy_shiny_patch  nv_png nvparse )

set_target_properties(bumpy_shiny_patch PROPERTIES PROJECT_LABEL "bumpy_shiny_patch")						
set_target_properties(bumpy_shiny_patch PROPERTIES OUTPUT_NAME "bumpy_shiny_patch")
set_target_properties(bumpy_shiny_patch PROPERTIES FOLDER "examples/opengl")
target_include_directories(bumpy_shiny_patch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png)
target_link_libraries(bumpy_shiny_patch nv_png nvparse )
install(TARGETS bumpy_shiny_patch RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= cg_scotopic =================================================
add_executable(cg_scotopic 
				"cg_scotopic.cpp" 
				"InteractionController.cpp" 
				"InteractionController.h" 
				"NightFilter.h" "NightFilter.cpp"
				"ScotopicFilter.h" 
				"ScotopicFilter.cpp")
add_dependencies(cg_scotopic  nv_image_processing paramgl )
set_target_properties(cg_scotopic PROPERTIES PROJECT_LABEL "cg_scotopic")						
set_target_properties(cg_scotopic PROPERTIES OUTPUT_NAME "cg_scotopic")
set_target_properties(cg_scotopic PROPERTIES FOLDER "examples/opengl")
set_target_properties( cg_scotopic PROPERTIES COMPILE_DEFINITIONS   NV_IMAGE_PROCESSING_DLL=1 )
target_include_directories(cg_scotopic PRIVATE ${GLEW_INCLUDE_PATH} ${CG_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_image_processing ${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl )
target_link_libraries(cg_scotopic nv_image_processing paramgl ${GLEW_LIBRARY})
install(TARGETS cg_scotopic RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# ================================================= cg_shades =================================================
add_executable(cg_shades "cg_shades.cpp" "../shared/data_path.h" "../shared/data_path.cpp")
add_dependencies(cg_shades  nv_dds paramgl)
set_target_properties(cg_shades PROPERTIES PROJECT_LABEL "cg_shades")						
set_target_properties(cg_shades PROPERTIES OUTPUT_NAME "cg_shades")
set_target_properties(cg_shades PROPERTIES FOLDER "examples/opengl") 
target_include_directories(cg_shades PRIVATE  ${CG_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_dds )
target_link_libraries(cg_shades nv_dds ${GLEW_LIBRARY} paramgl ${CG_LIBRARY} ${CG_GL_LIBRARY})
#install
install(TARGETS cg_shades RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= cg_CShinySprites =================================================
#add_executable(cg_CShinySprites "cg_CShinySprites.cpp" "cg_CShinySprites.h" "cg_ShinySprites.cpp" "resource.h" "resource.rc")
#add_dependencies(cg_CShinySprites  nv_dds )
#set_target_properties(cg_CShinySprites PROPERTIES PROJECT_LABEL "cg_CShinySprites")						
#set_target_properties(cg_CShinySprites PROPERTIES OUTPUT_NAME "cg_CShinySprites")
#set_target_properties(cg_CShinySprites PROPERTIES FOLDER "examples/opengl")
#target_include_directories(cg_CShinySprites PRIVATE  ${CG_INCLUDE_DIR} ${GLEW_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../paramgl ${CMAKE_CURRENT_SOURCE_DIR}/../png ${CMAKE_CURRENT_SOURCE_DIR}/../nv_dds ${CMAKE_CURRENT_SOURCE_DIR}/../nvparse/include )
#target_link_libraries(cg_CShinySprites nv_dds ${GLEW_LIBRARY} paramgl ${CG_LIBRARY} ${CG_GL_LIBRARY})
#install(TARGETS cg_CShinySprites RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= cullfragment2 =================================================
add_executable(cullfragment2 "cullfragment2.cpp" 
							"../shared/data_path.h" "../shared/data_path.cpp" 
							"../shared/read_text_file.h" 
							"../shared/read_text_file.cpp")
add_dependencies(cullfragment2  nvparse )
set_target_properties(cullfragment2 PROPERTIES PROJECT_LABEL "cullfragment2")						
set_target_properties(cullfragment2 PROPERTIES OUTPUT_NAME "cullfragment2")
set_target_properties(cullfragment2 PROPERTIES FOLDER "examples/opengl")
target_include_directories(cullfragment2 PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(cullfragment2 nvparse)
install(TARGETS cullfragment2 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= detail_normalmap =================================================
add_executable(detail_normalmap "detail_normalmap.cpp" "../shared/data_path.h" "../shared/data_path.cpp" 
					"../shared/bumpmap_to_normalmap.h" "../shared/bumpmap_to_normalmap.cpp" 
					"../shared/array_texture.h" "../shared/array_texture.cpp")
add_dependencies(detail_normalmap  nv_png nvparse)
set_target_properties(detail_normalmap PROPERTIES PROJECT_LABEL "detail_normalmap")						
set_target_properties(detail_normalmap PROPERTIES OUTPUT_NAME "detail_normalmap")
set_target_properties(detail_normalmap PROPERTIES FOLDER "examples/opengl")
# Properties->Linker->Input->Additional Dependencies
target_link_libraries(detail_normalmap nv_png nvparse)
install(TARGETS detail_normalmap RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= dynamic_amb_occ =================================================
add_executable(dynamic_amb_occ WIN32 "dynamic_amb_occ/DynamicAO.cpp" "dynamic_amb_occ/shader.cpp" "dynamic_amb_occ/SurfaceElement.cpp" "dynamic_amb_occ/SurfaceElement.h" "dynamic_amb_occ/vmath.h" "dynamic_amb_occ/DynamicAO.rc" "dynamic_amb_occ/resource.h")
add_dependencies(dynamic_amb_occ  nv_subd )
set_target_properties(dynamic_amb_occ PROPERTIES PROJECT_LABEL "dynamic_amb_occ")						
set_target_properties(dynamic_amb_occ PROPERTIES OUTPUT_NAME "dynamic_amb_occ")
set_target_properties(dynamic_amb_occ PROPERTIES FOLDER "examples/opengl")
# Properties->Linker->Input->Additional Dependencies
target_link_libraries(dynamic_amb_occ nv_subd opengl32)
install(TARGETS dynamic_amb_occ RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= external_triangles =================================================
add_executable(external_triangles "external_triangles.cpp")
set_target_properties(external_triangles PROPERTIES PROJECT_LABEL "external_triangles")						
set_target_properties(external_triangles PROPERTIES OUTPUT_NAME "external_triangles")
set_target_properties(external_triangles PROPERTIES FOLDER "examples/opengl")
target_link_libraries(external_triangles  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS external_triangles RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= fast_third_order =================================================
add_executable(fast_third_order "fast_third_order/fast_third_order.cpp" 
				"fast_third_order/slicing.h" "fast_third_order/slicing.cpp" 
				"fast_third_order/RenderTextureFix.h" "fast_third_order/RenderTextureFix.cpp"
				"fast_third_order/blobby.cpp" "fast_third_order/blobby.h" 
				"../shared/pbuffer.cpp" "../shared/read_text_file.cpp" 
				"../shared/data_path.cpp" )
set_target_properties(fast_third_order PROPERTIES PROJECT_LABEL "fast_third_order")						
set_target_properties(fast_third_order PROPERTIES OUTPUT_NAME "fast_third_order")
set_target_properties(fast_third_order PROPERTIES FOLDER "examples/opengl")
target_include_directories(fast_third_order PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(fast_third_order  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS fast_third_order RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= gpgpu_fft =================================================
add_executable(pptDemo "gpgpu_fft.cpp")
target_include_directories(pptDemo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../FFT ${CMAKE_CURRENT_SOURCE_DIR}/../../PBuffer ${GLEW_INCLUDE_PATH} ${CG_INCLUDE_DIR} )
set_target_properties(pptDemo PROPERTIES PROJECT_LABEL "pptDemo")						
set_target_properties(pptDemo PROPERTIES OUTPUT_NAME "pptDemo")
set_target_properties(pptDemo PROPERTIES FOLDER "examples/opengl")
add_dependencies(pptDemo  nv_fft nv_PBuffer)
target_link_libraries(pptDemo  opengl32 nv_fft nv_PBuffer ${GLEW_LIBRARY}  ${CG_LIBRARY} ${CG_GL_LIBRARY})
install(TARGETS pptDemo RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= fog_compare =================================================
add_executable(fog_compare "fog_compare.cpp" "TesselatedQuad.cpp" "TesselatedQuad.hpp")
#target_include_directories(fog_compare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../FFT ${CMAKE_CURRENT_SOURCE_DIR}/../PBuffer ${GLEW_INCLUDE_PATH} ${CG_INCLUDE_DIR} )
set_target_properties(fog_compare PROPERTIES PROJECT_LABEL "fog_compare")						
set_target_properties(fog_compare PROPERTIES OUTPUT_NAME "fog_compare")
set_target_properties(fog_compare PROPERTIES FOLDER "examples/opengl")
#add_dependencies(fog_compare  nv_fft nv_PBuffer)
target_link_libraries(fog_compare  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS fog_compare RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= GL_GameOfLife =================================================
add_executable(GL_GameOfLife "CA_GameOfLife.cpp" "CA_GameOfLife.hpp" "GL_GameOfLife.cpp" "ppm.hpp" "ppm.cpp" 
				"../shared/data_path.h" "../shared/data_path.cpp" 
							"../shared/read_text_file.h" "../shared/read_text_file.cpp" "../shared/pbuffer.cpp")
#target_include_directories(fog_compare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../FFT ${CMAKE_CURRENT_SOURCE_DIR}/../PBuffer ${GLEW_INCLUDE_PATH} ${CG_INCLUDE_DIR} )
set_target_properties(GL_GameOfLife PROPERTIES PROJECT_LABEL "GL_GameOfLife")						
set_target_properties(GL_GameOfLife PROPERTIES OUTPUT_NAME "GL_GameOfLife")
set_target_properties(GL_GameOfLife PROPERTIES FOLDER "examples/opengl")
add_dependencies(GL_GameOfLife  nvparse)
target_include_directories(GL_GameOfLife PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(GL_GameOfLife  opengl32 nvparse)
install(TARGETS GL_GameOfLife RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= glsl_pseudo_instancing =================================================
add_executable(glsl_pseudo_instancing "glsl_pseudo_instancing/ARBProgram.cpp" "glsl_pseudo_instancing/ARBProgram.h"
										"glsl_pseudo_instancing/Average.cpp" "glsl_pseudo_instancing/Average.h"
										"glsl_pseudo_instancing/File.cpp" "glsl_pseudo_instancing/File.h"
										"glsl_pseudo_instancing/File.cpp" "glsl_pseudo_instancing/File.h"
										"glsl_pseudo_instancing/InstanceConfiguration.cpp" "glsl_pseudo_instancing/InstanceConfiguration.h"
										"glsl_pseudo_instancing/Mesh.cpp" "glsl_pseudo_instancing/Mesh.h"
										"glsl_pseudo_instancing/Program.cpp" "glsl_pseudo_instancing/Program.h"
										"glsl_pseudo_instancing/SphereMesh.cpp" "glsl_pseudo_instancing/SphereMesh.h"
										"glsl_pseudo_instancing/Timer.cpp" "glsl_pseudo_instancing/Timer.h"
										"glsl_pseudo_instancing/Shader.cpp" "glsl_pseudo_instancing/Shader.h"
										"glsl_pseudo_instancing/glsl_pseudo_instancing.cpp" "glsl_pseudo_instancing/Vertex.h"
										"../shared/data_path.h" "../shared/data_path.cpp" 
										"../shared/array_texture.h" "../shared/array_texture.cpp"
							)
set_target_properties(glsl_pseudo_instancing PROPERTIES PROJECT_LABEL "glsl_pseudo_instancing")						
set_target_properties(glsl_pseudo_instancing PROPERTIES OUTPUT_NAME "glsl_pseudo_instancing")
set_target_properties(glsl_pseudo_instancing PROPERTIES FOLDER "examples/opengl")
add_dependencies(glsl_pseudo_instancing  nv_png)
target_include_directories(glsl_pseudo_instancing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(glsl_pseudo_instancing  opengl32 nv_png ${GLUT_glut_LIBRARY})
install(TARGETS glsl_pseudo_instancing RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= gpgpu_dct =================================================
add_executable(gpgpu_dct "gpgpu_dct/dct_cpu.cpp" "gpgpu_dct/dct_cpu.h"
										"gpgpu_dct/dct_gpu.cpp" "gpgpu_dct/dct_gpu.h"
										"gpgpu_dct/jidctflt.cpp" "gpgpu_dct/jfdctflt.cpp"
										"gpgpu_dct/gpgpu_dct.cpp" "gpgpu_dct/jdct.h"
										"../shared/data_path.h" "../shared/data_path.cpp" 
										"../shared/read_text_file.h" "../shared/read_text_file.cpp" "../shared/pbuffer.cpp"
										)
set_target_properties(gpgpu_dct PROPERTIES PROJECT_LABEL "gpgpu_dct")						
set_target_properties(gpgpu_dct PROPERTIES OUTPUT_NAME "gpgpu_dct")
set_target_properties(gpgpu_dct PROPERTIES FOLDER "examples/opengl")
add_dependencies(gpgpu_dct  nv_png)
target_include_directories(gpgpu_dct PRIVATE ${CG_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(gpgpu_dct  opengl32 nv_png ${CG_LIBRARY} ${CG_GL_LIBRARY} ${GLUT_glut_LIBRARY})
install(TARGETS gpgpu_dct RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= gpgpu_disease =================================================
add_executable(gpgpu_disease 
			"diseasehead.cpp" 
			"diseasehead.h" 
			"gpgpu_disease.cpp"  
			"reactiondiffusion.h" 
			"reactiondiffusion.cpp" "../shared/geep/streamopCgGL.cpp"
						"common/src/stopwatch.cpp" "../shared/objload.cpp" "../shared/pbuffer.cpp" "../shared/load_cubemap.cpp" "../shared/array_texture.cpp"
						"../shared/data_path.cpp")
						
set_target_properties(gpgpu_disease PROPERTIES PROJECT_LABEL "gpgpu_disease")						
set_target_properties(gpgpu_disease PROPERTIES OUTPUT_NAME "gpgpu_disease")
set_target_properties(gpgpu_disease PROPERTIES FOLDER "examples/opengl")
add_dependencies(gpgpu_disease  paramgl nv_png  nvIO)
target_include_directories(gpgpu_disease PRIVATE ${CG_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../shared/  
							${CMAKE_CURRENT_SOURCE_DIR}/common/inc ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl )
target_link_libraries(gpgpu_disease  opengl32 nv_png  nvIO paramgl ${CG_LIBRARY} ${CG_GL_LIBRARY})
install(TARGETS gpgpu_disease RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= gpgpu_fluid =================================================
add_executable(gpgpu_fluid "gpgpu_fluid/flo.cpp" "gpgpu_fluid/flo.h" "gpgpu_fluid/gpgpu_fluid.cpp"  
"common/src/stopwatch.cpp"  "../shared/pbuffer.cpp" 
"../shared/load_cubemap.cpp" "../shared/geep/fluidbase.cpp"
"../shared/data_path.cpp" "../shared/array_texture.cpp"
"../shared/MovieMaker.cpp"  "../shared/geep/streamopCgGL.cpp"
)
set_target_properties(gpgpu_fluid PROPERTIES PROJECT_LABEL "gpgpu_fluid")						
set_target_properties(gpgpu_fluid PROPERTIES OUTPUT_NAME "gpgpu_fluid")
set_target_properties(gpgpu_fluid PROPERTIES FOLDER "examples/opengl")
add_dependencies(gpgpu_fluid  paramgl nv_png  )
target_include_directories(gpgpu_fluid PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl ${CMAKE_CURRENT_SOURCE_DIR}/common/inc ${CG_INCLUDE_DIR})
target_link_libraries(gpgpu_fluid  opengl32 nv_png paramgl ${CG_LIBRARY} ${CG_GL_LIBRARY} Vfw32)
install(TARGETS gpgpu_fluid RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= gpu_particles =================================================
add_executable(gpu_particles "gpu_particles.cpp" "ParticleSystem.h" "ParticleSystem.cpp"  
"../shared/data_path.cpp" "../shared/array_texture.cpp" "../shared/pbuffer.cpp" "../shared/renderVertexArray.cpp"
)
set_target_properties(gpu_particles PROPERTIES PROJECT_LABEL "gpu_particles")						
set_target_properties(gpu_particles PROPERTIES OUTPUT_NAME "gpu_particles")
set_target_properties(gpu_particles PROPERTIES FOLDER "examples/opengl")
add_dependencies(gpu_particles   nv_png  )
target_include_directories(gpu_particles PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CG_INCLUDE_DIR} )
target_link_libraries(gpu_particles  opengl32 nv_png ${CG_LIBRARY} ${CG_GL_LIBRARY} ${GLUT_glut_LIBRARY})
install(TARGETS gpu_particles RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# ================================================= gpu_videoeffects =================================================

file(GLOB gpu_videoeffects_files
    "gpu_videoeffects/*.h"
    "gpu_videoeffects/*.cpp"
	"NightFilter.cpp"
)
add_executable(gpu_videoeffects ${gpu_videoeffects_files} "../shared/data_path.cpp")

set_target_properties(gpu_videoeffects PROPERTIES PROJECT_LABEL "gpu_videoeffects")						
set_target_properties(gpu_videoeffects PROPERTIES OUTPUT_NAME "gpu_videoeffects")
set_target_properties(gpu_videoeffects PROPERTIES FOLDER "examples/opengl")
target_compile_definitions(gpu_videoeffects PRIVATE NV_IMAGE_PROCESSING_DLL OPENEXR_DLL FILTER_DLL DIRECT_SHOW_BASE_CLASS_DLL)
#set_target_properties( gpu_videoeffects PROPERTIES COMPILE_DEFINITIONS   NV_IMAGE_PROCESSING_DLL=1 OPENEXR_DLL=1)
add_dependencies(gpu_videoeffects nv_image_processing paramgl nv_dds directshow_baseclass)
target_include_directories( gpu_videoeffects PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_image_processing 
				${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl 
				${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/directshow/baseclasses
				${CMAKE_CURRENT_SOURCE_DIR}/../../nv_dds 
				${OPENEXR_INCLUDE_PATHS} 
				${GLEW_INCLUDE_PATH} 
				${CG_INCLUDE_DIR}
				${DX10_INCLUDE_PATH}
				${DIRECTSHOW_INCLUDE_PATH}
				${GLUT_INCLUDE_DIR}
				)
				
target_link_libraries(gpu_videoeffects  opengl32  paramgl nv_dds nv_image_processing  directshow_baseclass ${GLEW_LIBRARY} ${OPENEXR_HALF_LIBRARY}
					${OPENEXR_IEX_LIBRARY} ${OPENEXR_ILMIMF_LIBRARY} 
					${OPENEXR_ILMTHREAD_LIBRARY} 
					${OPENEXR_IMATH_LIBRARY} 
					${CG_LIBRARY} ${CG_GL_LIBRARY} 
					${DIRECTSHOW_STRMBASE_LIBRARY} 
					${GLUT_glut_LIBRARY})
install(TARGETS gpu_videoeffects RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= hdr =================================================

file(GLOB hdr_src
    "hdr.cpp"
	
	"../shared/blur.cpp"
	"../shared/blur.h"
	"../shared/pbuffer.cpp"
	"../shared/NvIOModel.cpp"
)
add_executable(hdr ${hdr_src} "../shared/data_path.cpp")

set_target_properties(hdr PROPERTIES PROJECT_LABEL "hdr")						
set_target_properties(hdr PROPERTIES OUTPUT_NAME "hdr")
set_target_properties(hdr PROPERTIES FOLDER "examples/opengl")

#target_compile_definitions(gpu_videoeffects PRIVATE NV_IMAGE_PROCESSING_DLL OPENEXR_DLL FILTER_DLL DIRECT_SHOW_BASE_CLASS_DLL)
#set_target_properties( gpu_videoeffects PROPERTIES COMPILE_DEFINITIONS   NV_IMAGE_PROCESSING_DLL=1 OPENEXR_DLL=1)
target_include_directories( hdr PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../../hdr  ${CG_INCLUDE_DIR} 
				)
				
add_dependencies(hdr nvHdr nvIO)
 
target_link_libraries(hdr  opengl32 nvHdr ${CG_LIBRARY} ${CG_GL_LIBRARY} nvIO)
install(TARGETS hdr RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= imgproc_histogram =================================================

file(GLOB imgproc_histogram_src
    "histogram.cpp"
	"histogram.h"
	"imgproc_histogram.cpp"
)
add_executable(imgproc_histogram  ${imgproc_histogram_src} "../shared/data_path.cpp")

set_target_properties(imgproc_histogram PROPERTIES PROJECT_LABEL "imgproc_histogram")						
set_target_properties(imgproc_histogram PROPERTIES OUTPUT_NAME "imgproc_histogram")
set_target_properties(imgproc_histogram PROPERTIES FOLDER "examples/opengl")

target_include_directories( imgproc_histogram PRIVATE   ${CG_INCLUDE_DIR}  ${GLUT_INCLUDE_DIR})
target_link_libraries(imgproc_histogram  opengl32 ${CG_LIBRARY} ${CG_GL_LIBRARY} ${GLUT_glut_LIBRARY})
install(TARGETS imgproc_histogram RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= layerz_3x =================================================

file(GLOB layerz_3x_src
    "layerz_3x.cpp"
)
add_executable(layerz_3x  ${layerz_3x_src} )

set_target_properties(layerz_3x PROPERTIES PROJECT_LABEL "layerz_3x")						
set_target_properties(layerz_3x PROPERTIES OUTPUT_NAME "layerz_3x")
set_target_properties(layerz_3x PROPERTIES FOLDER "examples/opengl")

target_include_directories( layerz_3x PRIVATE    ${GLUT_INCLUDE_DIR})
target_link_libraries(layerz_3x  opengl32 ${GLUT_glut_LIBRARY} )
install(TARGETS layerz_3x RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= metalpaint =================================================

#file(GLOB metalpaint_src
#    "metalpaint.cpp"
#	"nvbHelper.h"
#	"../shared/data_path.cpp"
#	"../shared/noise.cpp"
#	"../shared/read_text_file.cpp"
#)
#add_executable(metalpaint  ${metalpaint_src} )

#set_target_properties(metalpaint PROPERTIES PROJECT_LABEL "metalpaint")						
#set_target_properties(metalpaint PROPERTIES OUTPUT_NAME "metalpaint")
#set_target_properties(metalpaint PROPERTIES FOLDER "examples/opengl")
#add_dependencies(metalpaint nv_dds)
#target_include_directories( metalpaint PRIVATE    ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../nv_dds  shared/ )
#target_link_libraries(metalpaint  opengl32 ${GLUT_glut_LIBRARY} nv_dds)
#install(TARGETS metalpaint RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= motion_blur =================================================

file(GLOB motion_blur_src
    "motion_blur.cpp"
	"../shared/NvIOModel.cpp"
	"../shared/array_texture.cpp"
)
add_executable(motion_blur  ${motion_blur_src} )

set_target_properties(motion_blur PROPERTIES PROJECT_LABEL "motion_blur")						
set_target_properties(motion_blur PROPERTIES OUTPUT_NAME "motion_blur")
set_target_properties(motion_blur PROPERTIES FOLDER "examples/opengl")
add_dependencies(motion_blur nv_png nvIO)
target_include_directories( motion_blur PRIVATE    ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png  ${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CG_INCLUDE_DIR})
target_link_libraries(motion_blur  opengl32 nv_png ${GLUT_glut_LIBRARY} ${CG_LIBRARY} ${CG_GL_LIBRARY} nvIO)
install(TARGETS motion_blur RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= MRI =================================================
file(GLOB MRI_src "MRI.cpp")
add_executable(MRI ${MRI_src} )
set_target_properties(MRI PROPERTIES PROJECT_LABEL "MRI")						
set_target_properties(MRI PROPERTIES OUTPUT_NAME "MRI")
set_target_properties(MRI PROPERTIES FOLDER "examples/opengl")
add_dependencies(MRI nv_fft nv_PBuffer)
target_include_directories( MRI PRIVATE    ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../FFT  ${CMAKE_CURRENT_SOURCE_DIR}/../../PBuffer ${CG_INCLUDE_DIR} )
target_link_libraries(MRI  opengl32 nv_fft nv_PBuffer ${GLEW_LIBRARY} ${CG_LIBRARY} ${CG_GL_LIBRARY})
install(TARGETS MRI RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= normalization_heuristics =================================================
file(GLOB normalization_heuristics_src 
	"head.cpp" "head.h" "normalization_heuristics.cpp" 
	"../shared/data_path.cpp" "../shared/objload.cpp" 
	"common/src/stopwatch.cpp")
	
add_executable(normalization_heuristics ${normalization_heuristics_src} )
set_target_properties(normalization_heuristics PROPERTIES PROJECT_LABEL "normalization_heuristics")						
set_target_properties(normalization_heuristics PROPERTIES OUTPUT_NAME "normalization_heuristics")
set_target_properties(normalization_heuristics PROPERTIES FOLDER "examples/opengl")
add_dependencies(normalization_heuristics nv_png nvIO)
target_include_directories( normalization_heuristics PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../  ${CG_INCLUDE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/common/inc)
target_link_libraries(normalization_heuristics  opengl32 nv_png ${GLUT_glut_LIBRARY} nvIO ${CG_LIBRARY} ${CG_GL_LIBRARY} )
install(TARGETS normalization_heuristics RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= NVCPl =================================================
file(GLOB NVCPl_src 
	"NvApiError.h"
	"NvCpl.h" "NvCpl.cpp" 
	"NvPanelApi.h")
	
add_executable(NVCPl ${NVCPl_src} )
set_target_properties(NVCPl PROPERTIES PROJECT_LABEL "NVCPl")						
set_target_properties(NVCPl PROPERTIES OUTPUT_NAME "NVCPl")
set_target_properties(NVCPl PROPERTIES FOLDER "examples/opengl")
#target_include_directories( NVCPl PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../png ${CMAKE_CURRENT_SOURCE_DIR}/../  ${CG_INCLUDE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/common/inc)
target_link_libraries(NVCPl  opengl32 )
install(TARGETS NVCPl RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= oblique_frustum =================================================
file(GLOB oblique_frustum_src 
	"oblique_frustum.cpp"
	)
	
add_executable(oblique_frustum ${oblique_frustum_src} )
set_target_properties(oblique_frustum PROPERTIES PROJECT_LABEL "oblique_frustum")						
set_target_properties(oblique_frustum PROPERTIES OUTPUT_NAME "oblique_frustum")
set_target_properties(oblique_frustum PROPERTIES FOLDER "examples/opengl")
target_include_directories( oblique_frustum PRIVATE  ${GLUT_INCLUDE_DIR})
target_link_libraries(oblique_frustum  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS oblique_frustum RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= occlusion_query =================================================
file(GLOB occlusion_query_src 
	"occlusion_query.cpp"
	)
add_executable(occlusion_query ${occlusion_query_src} )
set_target_properties(occlusion_query PROPERTIES PROJECT_LABEL "occlusion_query")						
set_target_properties(occlusion_query PROPERTIES OUTPUT_NAME "occlusion_query")
set_target_properties(occlusion_query PROPERTIES FOLDER "examples/opengl")
target_include_directories( occlusion_query PRIVATE  ${GLUT_INCLUDE_DIR})
target_link_libraries(occlusion_query  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS occlusion_query RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= offset_bump_mapping =================================================
file(GLOB offset_bump_mapping_src 
	"offset_bump_mapping.cpp"
	"../shared/array_texture.cpp"
	"../shared/data_path.cpp"
	"../shared/read_text_file.cpp"
	"../shared/bumpmap_to_normalmap.cpp"
	)
add_executable(offset_bump_mapping ${offset_bump_mapping_src} )
set_target_properties(offset_bump_mapping PROPERTIES PROJECT_LABEL "offset_bump_mapping")						
set_target_properties(offset_bump_mapping PROPERTIES OUTPUT_NAME "offset_bump_mapping")
set_target_properties(offset_bump_mapping PROPERTIES FOLDER "examples/opengl")
add_dependencies( offset_bump_mapping nvparse nv_png)
target_include_directories( offset_bump_mapping PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include ${CMAKE_CURRENT_SOURCE_DIR}/../../png)
target_link_libraries(offset_bump_mapping  opengl32 nvparse nv_png ${GLUT_glut_LIBRARY})
install(TARGETS offset_bump_mapping RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= oit_3x =================================================
file(GLOB oit_3x_src 
	"oit_3x.cpp"
	)
add_executable(oit_3x ${oit_3x_src} )
set_target_properties(oit_3x PROPERTIES PROJECT_LABEL "oit_3x")						
set_target_properties(oit_3x PROPERTIES OUTPUT_NAME "oit_3x")
set_target_properties(oit_3x PROPERTIES FOLDER "examples/opengl")
target_include_directories( oit_3x PRIVATE  ${GLUT_INCLUDE_DIR} )
target_link_libraries(oit_3x  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS oit_3x RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= pbuffer_to_texture_rectangle =================================================
file(GLOB pbuffer_to_texture_rectangle_src 
	"pbuffer_to_texture_rectangle.cpp"
	"../shared/pbuffer.cpp"
	)
add_executable(pbuffer_to_texture_rectangle ${pbuffer_to_texture_rectangle_src} )
set_target_properties(pbuffer_to_texture_rectangle PROPERTIES PROJECT_LABEL "pbuffer_to_texture_rectangle")						
set_target_properties(pbuffer_to_texture_rectangle PROPERTIES OUTPUT_NAME "pbuffer_to_texture_rectangle")
set_target_properties(pbuffer_to_texture_rectangle PROPERTIES FOLDER "examples/opengl")
target_include_directories( pbuffer_to_texture_rectangle PRIVATE  ${GLUT_INCLUDE_DIR} )
target_link_libraries(pbuffer_to_texture_rectangle  opengl32 ${GLUT_glut_LIBRARY})
install(TARGETS pbuffer_to_texture_rectangle RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= perpixel_attenuation =================================================
file(GLOB perpixel_attenuation_src 
	"perpixel_attenuation.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	"../shared/load_cubemap.cpp"
	)
add_executable(perpixel_attenuation ${perpixel_attenuation_src} )
set_target_properties(perpixel_attenuation PROPERTIES PROJECT_LABEL "perpixel_attenuation")						
set_target_properties(perpixel_attenuation PROPERTIES OUTPUT_NAME "perpixel_attenuation")
set_target_properties(perpixel_attenuation PROPERTIES FOLDER "examples/opengl")
add_dependencies( offset_bump_mapping nv_png nvparse)
target_include_directories( perpixel_attenuation PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png  ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include)
target_link_libraries(perpixel_attenuation  opengl32  nv_png  nvparse ${GLUT_glut_LIBRARY})
install(TARGETS perpixel_attenuation RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= physics =================================================
file(GLOB physics_src 
	"glsl_physics/ParticleSystem.cpp"
	"glsl_physics/ParticleSystem.h"
	"glsl_physics/physics.cpp"
	"../shared/data_path.cpp"
	"../shared/read_text_file.cpp"
	"../shared/array_texture.cpp"
	"../shared/load_cubemap.cpp"
	"../shared/renderVertexArray.cpp"
	"../shared/pbuffer.cpp"
	)
add_executable(physics ${physics_src} )
set_target_properties(physics PROPERTIES PROJECT_LABEL "physics")						
set_target_properties(physics PROPERTIES OUTPUT_NAME "physics")
set_target_properties(physics PROPERTIES FOLDER "examples/opengl")
add_dependencies( physics nv_png )
target_include_directories( physics PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
target_link_libraries(physics  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS physics RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= separate_specular =================================================
file(GLOB separate_specular_src 
	"separate_specular.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	)
add_executable(separate_specular ${separate_specular_src} )
set_target_properties(separate_specular PROPERTIES PROJECT_LABEL "separate_specular")						
set_target_properties(separate_specular PROPERTIES OUTPUT_NAME "separate_specular")
set_target_properties(separate_specular PROPERTIES FOLDER "examples/opengl")
add_dependencies( separate_specular nv_png )
target_include_directories( separate_specular PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
target_link_libraries(separate_specular  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS separate_specular RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_draw_buffers =================================================
file(GLOB simple_draw_buffers_src 
	"simple_draw_buffers.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	"../shared/read_text_file.cpp"
	"../shared/pbuffer.cpp"
	)
add_executable(simple_draw_buffers ${simple_draw_buffers_src} )
set_target_properties(simple_draw_buffers PROPERTIES PROJECT_LABEL "simple_draw_buffers")						
set_target_properties(simple_draw_buffers PROPERTIES OUTPUT_NAME "simple_draw_buffers")
set_target_properties(simple_draw_buffers PROPERTIES FOLDER "examples/opengl")
add_dependencies( simple_draw_buffers nv_png )
target_include_directories( simple_draw_buffers PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
target_link_libraries(simple_draw_buffers  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS simple_draw_buffers RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_float_pbuffer =================================================
file(GLOB simple_float_pbuffer_src 
	"float_pbuffer_lnx.h"
	"float_pbuffer_win.h"
	"simple_float_pbuffer.cpp"
	"../shared/data_path.cpp"
	)
add_executable(simple_float_pbuffer ${simple_float_pbuffer_src} )
set_target_properties(simple_float_pbuffer PROPERTIES PROJECT_LABEL "simple_float_pbuffer")						
set_target_properties(simple_float_pbuffer PROPERTIES OUTPUT_NAME "simple_float_pbuffer")
set_target_properties(simple_float_pbuffer PROPERTIES FOLDER "examples/opengl")
add_dependencies( simple_float_pbuffer nv_dds )
target_include_directories( simple_float_pbuffer PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_dds  )
target_link_libraries(simple_float_pbuffer  opengl32  nv_dds   ${GLUT_glut_LIBRARY})
install(TARGETS simple_float_pbuffer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_fragment_program2 =================================================
file(GLOB simple_fragment_program2_src 
	"simple_fragment_program2.cpp"
	"../shared/data_path.cpp"
	"../shared/read_text_file.cpp"
	)
add_executable(simple_fragment_program2 ${simple_fragment_program2_src} )
set_target_properties(simple_fragment_program2 PROPERTIES PROJECT_LABEL "simple_fragment_program2")						
set_target_properties(simple_fragment_program2 PROPERTIES OUTPUT_NAME "simple_fragment_program2")
set_target_properties(simple_fragment_program2 PROPERTIES FOLDER "examples/opengl")
target_include_directories( simple_fragment_program2 PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(simple_fragment_program2  opengl32     ${GLUT_glut_LIBRARY})
install(TARGETS simple_fragment_program2 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_framebuffer_object =================================================
file(GLOB simple_framebuffer_object_src 
	"simple_framebuffer_object.cpp"
	)
add_executable(simple_framebuffer_object ${simple_framebuffer_object_src} )
set_target_properties(simple_framebuffer_object PROPERTIES PROJECT_LABEL "simple_framebuffer_object")						
set_target_properties(simple_framebuffer_object PROPERTIES OUTPUT_NAME "simple_framebuffer_object")
set_target_properties(simple_framebuffer_object PROPERTIES FOLDER "examples/opengl")
target_include_directories( simple_framebuffer_object PRIVATE  ${GLUT_INCLUDE_DIR})
target_link_libraries(simple_framebuffer_object  opengl32     ${GLUT_glut_LIBRARY})
install(TARGETS simple_framebuffer_object RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_lighting =================================================
file(GLOB simple_lighting_src 
	"simple_lighting.cpp"
	"../shared/data_path.cpp"
	"../shared/objload.cpp"
	"../shared/read_text_file.cpp"
	)
add_executable(simple_lighting ${simple_lighting_src} )
set_target_properties(simple_lighting PROPERTIES PROJECT_LABEL "simple_lighting")						
set_target_properties(simple_lighting PROPERTIES OUTPUT_NAME "simple_lighting")
set_target_properties(simple_lighting PROPERTIES FOLDER "examples/opengl")
target_include_directories( simple_lighting PRIVATE  ${GLUT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../nvIO ${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
add_dependencies( simple_lighting nvIO )
target_link_libraries(simple_lighting  opengl32   nvIO  ${GLUT_glut_LIBRARY})
install(TARGETS simple_lighting RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_pbuffer =================================================
file(GLOB simple_pbuffer_src 
	"simple_pbuffer.cpp"
	"../shared/pbuffer.cpp"
	)
add_executable(simple_pbuffer ${simple_pbuffer_src} )
set_target_properties(simple_pbuffer PROPERTIES PROJECT_LABEL "simple_pbuffer")						
set_target_properties(simple_pbuffer PROPERTIES OUTPUT_NAME "simple_pbuffer")
set_target_properties(simple_pbuffer PROPERTIES FOLDER "examples/opengl")
target_include_directories( simple_pbuffer PRIVATE  ${GLUT_INCLUDE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/../shared/)
target_link_libraries(simple_pbuffer  opengl32     ${GLUT_glut_LIBRARY})
install(TARGETS simple_pbuffer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_render_texture 1D =================================================
file(GLOB simple_render_texture_src 
	"simple_render_texture.cpp"
	"../shared/pbuffer.cpp"
	)
add_executable(simple_render_texture ${simple_render_texture_src} )
 
set_target_properties(simple_render_texture PROPERTIES PROJECT_LABEL "simple_render_texture")						# in Visual Studio
set_target_properties(simple_render_texture PROPERTIES OUTPUT_NAME "simple_render_texture")
set_target_properties(simple_render_texture PROPERTIES FOLDER "examples/opengl")
target_include_directories(simple_render_texture PRIVATE ${GLUT_INCLUDE_DIR}  
${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_manip )
add_dependencies( simple_render_texture nv_math nv_manip )
target_link_libraries(simple_render_texture    opengl32 ${GLUT_glut_LIBRARY} nv_math nv_manip )
#install
install(TARGETS simple_render_texture RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_soft_shadows =================================================
file(GLOB simple_soft_shadows_src 
	"simple_soft_shadows_opengl.cpp"
	"../shared/pbuffer.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	"../shared/read_text_file.cpp"
	)
add_executable(simple_soft_shadows ${simple_soft_shadows_src} )
set_target_properties(simple_soft_shadows PROPERTIES PROJECT_LABEL "simple_soft_shadows")						
set_target_properties(simple_soft_shadows PROPERTIES OUTPUT_NAME "simple_soft_shadows")
set_target_properties(simple_soft_shadows PROPERTIES FOLDER "examples/opengl")
target_include_directories(simple_soft_shadows PRIVATE ${GLUT_INCLUDE_DIR} 
 ${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
add_dependencies( simple_soft_shadows nv_png  )
target_link_libraries(simple_soft_shadows  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS simple_soft_shadows RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_texture_rectangle =================================================
file(GLOB simple_texture_rectangle_src 
	"simple_soft_shadows_opengl.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	"../shared/pbuffer.cpp"
	"../shared/read_text_file.cpp"
	)
add_executable(simple_texture_rectangle ${simple_texture_rectangle_src} )
set_target_properties(simple_texture_rectangle PROPERTIES PROJECT_LABEL "simple_texture_rectangle")						
set_target_properties(simple_texture_rectangle PROPERTIES OUTPUT_NAME "simple_texture_rectangle")
set_target_properties(simple_texture_rectangle PROPERTIES FOLDER "examples/opengl")
target_include_directories(simple_texture_rectangle PRIVATE ${GLUT_INCLUDE_DIR}  
${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
add_dependencies( simple_texture_rectangle nv_png  )
target_link_libraries(simple_texture_rectangle  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS simple_texture_rectangle RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= simple_vertex_texture =================================================
file(GLOB simple_vertex_texture_src 
	"simple_vertex_texture.cpp"
	"../shared/data_path.cpp"
	"../shared/array_texture.cpp"
	"../shared/pbuffer.cpp"
	"../shared/read_text_file.cpp"
	)
add_executable(simple_vertex_texture ${simple_vertex_texture_src} )
set_target_properties(simple_vertex_texture PROPERTIES PROJECT_LABEL "simple_vertex_texture")						
set_target_properties(simple_vertex_texture PROPERTIES OUTPUT_NAME "simple_vertex_texture")
set_target_properties(simple_vertex_texture PROPERTIES FOLDER "examples/opengl")
target_include_directories(simple_vertex_texture PRIVATE 
${GLUT_INCLUDE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
add_dependencies( simple_vertex_texture nv_png  )
target_link_libraries(simple_vertex_texture  opengl32  nv_png   ${GLUT_glut_LIBRARY})
install(TARGETS simple_vertex_texture RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= skinning =================================================
file(GLOB skinning_src 
	"skinning.cpp"
	"skinning.h"
	"../shared/data_path.cpp"
	"../shared/read_text_file.cpp"
	"../nv_util/nv_zip.cpp"
	)
add_executable(skinning ${skinning_src} )
set_target_properties(skinning PROPERTIES PROJECT_LABEL "skinning")						
set_target_properties(skinning PROPERTIES OUTPUT_NAME "skinning")
set_target_properties(skinning PROPERTIES FOLDER "examples/opengl")
target_include_directories(skinning PRIVATE ${GLUT_INCLUDE_DIR} 
 ${CMAKE_CURRENT_SOURCE_DIR}/../../png ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_util  )
add_dependencies( skinning nv_png nv_util )
set_target_properties( skinning PROPERTIES COMPILE_DEFINITIONS  NV_UTIL_DLL )
target_link_libraries(skinning  opengl32  nv_png  nv_util ${GLUT_glut_LIBRARY})
install(TARGETS skinning RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= superflow =================================================
file(GLOB superflow_src 
	"superflow.cpp"
	"stablefluids.h"
	"stablefluids.cpp"
	"../shared/data_path.cpp"
	"texture.cpp"
	"texture.h"
	)
add_executable(superflow ${superflow_src} )
set_target_properties(superflow PROPERTIES PROJECT_LABEL "superflow")						
set_target_properties(superflow PROPERTIES OUTPUT_NAME "superflow")
set_target_properties(superflow PROPERTIES FOLDER "examples/opengl")
target_include_directories(superflow PRIVATE ${GLUT_INCLUDE_DIR}   
	${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include 
	${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/fftw-2.1.5-Win-x64-master/fftw  
	${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/fftw-2.1.5-Win-x64-master)
add_dependencies( superflow nvparse )
target_link_libraries(superflow  opengl32  nvparse 
		${GLUT_glut_LIBRARY} 
		${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/fftw-2.1.5-Win-x64-master/build/Debug/fftw.lib  
		${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/fftw-2.1.5-Win-x64-master/build/Debug/rfftw.lib)
		
install(TARGETS superflow RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= texshd_offset_2d =================================================
file(GLOB texshd_offset_2d_src 
	"texshd_offset_2d.cpp"
	"Perlin.cpp"
	"../shared/data_path.cpp"
	)
add_executable(texshd_offset_2d ${texshd_offset_2d_src} )
set_target_properties(texshd_offset_2d PROPERTIES PROJECT_LABEL "texshd_offset_2d")						
set_target_properties(texshd_offset_2d PROPERTIES OUTPUT_NAME "texshd_offset_2d")
set_target_properties(texshd_offset_2d PROPERTIES FOLDER "examples/opengl")
target_include_directories(texshd_offset_2d PRIVATE ${GLUT_INCLUDE_DIR}    ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_util ${CMAKE_CURRENT_SOURCE_DIR}/../../nvparse/include   )
add_dependencies( texshd_offset_2d nv_util nvparse )
target_link_libraries(texshd_offset_2d  opengl32  nvparse ${GLUT_glut_LIBRARY} nv_util)
install(TARGETS texshd_offset_2d RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= TexturePerformancePBO =================================================
file(GLOB TexturePerformancePBO_SRC 
	"TexturePerformancePBO/*"
	)
add_executable(TexturePerformancePBO ${TexturePerformancePBO_SRC} )
set_target_properties(TexturePerformancePBO PROPERTIES PROJECT_LABEL "TexturePerformancePBO")						
set_target_properties(TexturePerformancePBO PROPERTIES OUTPUT_NAME "TexturePerformancePBO")
set_target_properties(TexturePerformancePBO PROPERTIES FOLDER "examples/opengl")
target_compile_definitions(TexturePerformancePBO PRIVATE NV_IMAGE_PROCESSING_DLL OPENEXR_DLL )
target_include_directories(TexturePerformancePBO PRIVATE ${GLUT_INCLUDE_DIR}   
	${CMAKE_CURRENT_SOURCE_DIR}/../../paramgl  ${OPENEXR_INCLUDE_PATHS})
add_dependencies( TexturePerformancePBO paramgl  )
target_link_libraries(TexturePerformancePBO  opengl32  ${GLUT_glut_LIBRARY} paramgl
						${OPENEXR_IEX_LIBRARY} ${OPENEXR_ILMIMF_LIBRARY} 
						${OPENEXR_ILMTHREAD_LIBRARY} 
						${OPENEXR_IMATH_LIBRARY}
						${OPENEXR_HALF_LIBRARY}
					)
install(TARGETS TexturePerformancePBO RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= vertex_water =================================================
file(GLOB vertex_water_SRC 
	vertex_water.cpp
	../shared/read_text_file.cpp
	../shared/array_texture.cpp
	../shared/data_path.cpp
	../shared/load_cubemap.cpp
	)
add_executable(vertex_water ${vertex_water_SRC} )
set_target_properties(vertex_water PROPERTIES PROJECT_LABEL "vertex_water")						
set_target_properties(vertex_water PROPERTIES OUTPUT_NAME "vertex_water")
set_target_properties(vertex_water PROPERTIES FOLDER "examples/opengl")
target_include_directories(vertex_water PRIVATE ${GLUT_INCLUDE_DIR}   
	${CMAKE_CURRENT_SOURCE_DIR}/../../png  )
add_dependencies( vertex_water nv_png  )
target_link_libraries(vertex_water  opengl32  ${GLUT_glut_LIBRARY} nv_png
					)
install(TARGETS vertex_water RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= volume_intersect =================================================
file(GLOB volume_intersect_SRC 
	volume_intersect.cpp
	)
add_executable(volume_intersect ${volume_intersect_SRC} )
set_target_properties(volume_intersect PROPERTIES PROJECT_LABEL "volume_intersect")						
set_target_properties(volume_intersect PROPERTIES OUTPUT_NAME "volume_intersect")
set_target_properties(volume_intersect PROPERTIES FOLDER "examples/opengl")
target_include_directories(volume_intersect PRIVATE ${GLUT_INCLUDE_DIR})
#add_dependencies( volume_intersect   )
target_link_libraries(volume_intersect  opengl32  ${GLUT_glut_LIBRARY})
install(TARGETS volume_intersect RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)



