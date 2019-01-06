
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
##Globe include and link

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../3rth
					${CMAKE_CURRENT_SOURCE_DIR}/../../
					${CMAKE_CURRENT_SOURCE_DIR}/../
					${CMAKE_CURRENT_SOURCE_DIR}
				)

add_compile_options(-D _XKEYCHECK_H)	


####################################################         DirectX            ####################################################
include_directories(${DX10_INCLUDE_PATH})
LINK_DIRECTORIES(${DX10_INCLUDE_PATH}/../Lib/x86)
SET(D3DXLIBS dxerr d3dx9d d3d9 d3dxof dxguid winmm kernel32 user32 gdi32 winspool comctl32 comdlg32 advapi32 shell32 ole32 oleaut32 uuid dmoguids strmiids)
ADD_DEFINITIONS(-DNV_DXUT_DLL)	#dxut									
ADD_DEFINITIONS(-DDIRECT_SHOW_BASE_CLASS_DLL)	#directshow	

ADD_DEFINITIONS(-DNTDDI_VERSION=0x06030000)	#directshow	
ADD_DEFINITIONS(-DWINVER=0x0A00)	#directshow	
ADD_DEFINITIONS(-D_WIN32_WINNT=0x0A00)	#directshow



# ================================================= Cloth =================================================
file(GLOB Cloth_sources_src
    "Cloth/*.h"
    "Cloth/*.cpp"
	"Cloth/resource.h"
	"Cloth/BloodShader.rc"
)

 add_custom_command(OUTPUT  ClothSim.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /Fo ../MEDIA/programs/Cloth/ClothSim.cso ../MEDIA/programs/Cloth/ClothSim.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(Cloth_effect_target DEPENDS ClothSim.cso)							   
add_executable(Cloth WIN32 ${Cloth_sources_src})															
add_dependencies(Cloth  nv_dxut Cloth_effect_target)
set_target_properties(Cloth PROPERTIES PROJECT_LABEL "Cloth")						
set_target_properties(Cloth PROPERTIES OUTPUT_NAME "Cloth")
set_target_properties(Cloth PROPERTIES FOLDER "examples/DirectX")
target_include_directories(Cloth PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(Cloth nv_dxut ${D3DXLIBS})
install(TARGETS Cloth RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


 # ================================================= shader_Aniso =================================================
file(GLOB shader_Aniso_Src
    "../shader_Aniso.h"
    "../shader_Aniso.cpp"
	"aniso_resource.h"
	"HLSL_Aniso.rc"
)
 add_custom_command(OUTPUT  anisoHLSL.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/anisoHLSL.cso ../MEDIA/programs/anisoHLSL.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(shader_Aniso_effect_target DEPENDS anisoHLSL.cso)								   
add_executable(shader_Aniso WIN32 ${shader_Aniso_Src})															
add_dependencies(shader_Aniso  nv_dxut shader_Aniso_effect_target)
set_target_properties(shader_Aniso PROPERTIES PROJECT_LABEL "shader_Aniso")						
set_target_properties(shader_Aniso PROPERTIES OUTPUT_NAME "shader_Aniso")
set_target_properties(shader_Aniso PROPERTIES FOLDER "examples/DirectX")
target_include_directories(shader_Aniso PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(shader_Aniso nv_dxut ${D3DXLIBS})
install(TARGETS shader_Aniso RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

 # ================================================= HLSL_Instancing =================================================
file(GLOB HLSL_Instancing_Src
    "HLSL_Instancing/Asteroid.h"
    "HLSL_Instancing/Asteroid.cpp"
	"HLSL_Instancing/AsteroidManager.h"
	"HLSL_Instancing/AsteroidManager.cpp"
	"HLSL_Instancing/HLSL_Instancing.cpp"
	"nv_skybox.h"
	"HLSL_Instancing/SpaceShip.h"
	"HLSL_Instancing/resource.h"
	"HLSL_Instancing/HLSL_Aniso.rc"
	"HLSL_Instancing/SpaceShipManager.cpp"
	"HLSL_Instancing/SpaceShipManager.h"
	"HLSL_Instancing/MothershipManager.h"
	"HLSL_Instancing/nv_d3d9FullScreenQuad.h"
)
 add_custom_command(OUTPUT  HLSL_Instancing.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/HLSL_Instancing.cso ../MEDIA/programs/HLSL_Instancing.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(HLSL_Instancing_effect_target DEPENDS HLSL_Instancing.cso)								   
add_executable(HLSL_Instancing WIN32 ${HLSL_Instancing_Src})															
add_dependencies(HLSL_Instancing  nv_dxut)
set_target_properties(HLSL_Instancing PROPERTIES PROJECT_LABEL "HLSL_Instancing")						
set_target_properties(HLSL_Instancing PROPERTIES OUTPUT_NAME "HLSL_Instancing")
set_target_properties(HLSL_Instancing PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_Instancing PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(HLSL_Instancing nv_dxut ${D3DXLIBS})
install(TARGETS HLSL_Instancing RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= HLSL_RainbowFogbow =================================================
file(GLOB HLSL_RainbowFogbow_Src
    "HLSL_RainbowFogbow.h"
    "HLSL_RainbowFogbow.cpp"
	"hlsl_rainbowflgbow_resource.h.h"
	"HLSL_RainbowFogbow.rc"
	"nv_RainbowEffect.h"
	"nv_RainbowEffect.cpp"
	"nv_d3d9FullScreenQuad.h"
)
 add_custom_command(OUTPUT  RainbowFogbow.cso RainbowFogbowNoiseScroll.cso RainbowFogbowSDKHelpers.cso
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /LD /Fo ../MEDIA/programs/RainbowFogbow.cso ../MEDIA/programs/RainbowFogbow.fx
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /LD /Fo ../MEDIA/programs/RainbowFogbowNoiseScroll.cso ../MEDIA/programs/RainbowFogbowNoiseScroll.fx
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /LD /Fo ../MEDIA/programs/RainbowFogbowSDKHelpers.cso ../MEDIA/programs/RainbowFogbowSDKHelpers.fx
					    #MAIN_DEPENDENCY ../MEDIA/programs/AntiAliasingWithTransparency/GrassObject.fx
                        COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(HLSL_RainbowFogbow_effect_target DEPENDS RainbowFogbow.cso RainbowFogbowNoiseScroll.cso RainbowFogbowSDKHelpers.cso)								   
add_executable(HLSL_RainbowFogbow WIN32 ${HLSL_RainbowFogbow_Src})															
add_dependencies(HLSL_RainbowFogbow  nv_dxut HLSL_RainbowFogbow_effect_target)
set_target_properties(HLSL_RainbowFogbow PROPERTIES PROJECT_LABEL "HLSL_RainbowFogbow")						
set_target_properties(HLSL_RainbowFogbow PROPERTIES OUTPUT_NAME "HLSL_RainbowFogbow")
set_target_properties(HLSL_RainbowFogbow PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_RainbowFogbow PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(HLSL_RainbowFogbow nv_dxut ${D3DXLIBS})
install(TARGETS HLSL_RainbowFogbow RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= NearClipPlane =================================================
file(GLOB NearClipPlane_Src
    "NearClipPlane.h"
    "NearClipPlane.cpp"
	"NearClipPlaneApp.h"
    "NearClipPlaneApp.cpp"
	"resource.h"
	"resource.rc"
)		   			   														
add_executable(NearClipPlane WIN32 ${NearClipPlane_Src})															
add_dependencies(NearClipPlane  nv_dxut  )
set_target_properties(NearClipPlane PROPERTIES PROJECT_LABEL "NearClipPlane")						
set_target_properties(NearClipPlane PROPERTIES OUTPUT_NAME "NearClipPlane")
set_target_properties(NearClipPlane PROPERTIES FOLDER "examples/DirectX")
target_include_directories(NearClipPlane PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  )
target_link_libraries(NearClipPlane nv_dxut ${D3DXLIBS} )
install(TARGETS NearClipPlane RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= NVTriStripTest =================================================
file(GLOB NVTriStripTest_Src
    "NVTriStripTest.h"
    "NVTriStripTest.cpp"
	"aniso_resource.h"
	"HLSL_Aniso.rc"
)
 add_custom_command(OUTPUT  NVTriStripTest.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/NVTriStripTest.cso ../MEDIA/programs/NVTriStripTest.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
add_custom_target(NVTriStripTest_effect_target DEPENDS NVTriStripTest.cso)								   
add_executable(NVTriStripTest WIN32 ${NVTriStripTest_Src})															
add_dependencies(NVTriStripTest  nv_dxut nvTripStrip NVTriStripTest_effect_target)
set_target_properties(NVTriStripTest PROPERTIES PROJECT_LABEL "NVTriStripTest")						
set_target_properties(NVTriStripTest PROPERTIES OUTPUT_NAME "NVTriStripTest")
set_target_properties(NVTriStripTest PROPERTIES FOLDER "examples/DirectX")
target_include_directories(NVTriStripTest PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nvTriStrip)
target_link_libraries(NVTriStripTest nv_dxut nvTripStrip ${D3DXLIBS})
install(TARGETS NVTriStripTest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= HLSL_VideoFilter =================================================

file(GLOB HLSL_VideoFilter_sources_src
	"HLSL_VideoFilter/Allocator.cpp"
	"HLSL_VideoFilter/Allocator.h"
	"HLSL_VideoFilter/DShowTextures.cpp"
	"HLSL_VideoFilter/DShowTextures.h"
	"HLSL_VideoFilter/d3dfont.cpp"
	"HLSL_VideoFilter/d3dfont.h"
	"HLSL_VideoFilter/dshowutil.h"
	"HLSL_VideoFilter/dshowutil.cpp"
	"HLSL_VideoFilter/dxutil.cpp"
	"HLSL_VideoFilter/dxutil.h"
	"HLSL_VideoFilter/GraphBuilder.h"
	"HLSL_VideoFilter/GraphBuilder.cpp"
	"HLSL_VideoFilter/VideoFilterApp.cpp"
	"HLSL_VideoFilter/VideoFilterApp.h"
	"HLSL_VideoFilter/VideoFilter.cpp"
	"HLSL_VideoFilter/VideoFilter.h"
	"HLSL_VideoFilter/video_flags.h"
	"HLSL_VideoFilter/textures.h"
	"HLSL_VideoFilter/nvafx.h"
	"HLSL_VideoFilter/Mutex.h"
	"HLSL_VideoFilter/dshowasf.h"
	"HLSL_VideoFilter/resource.h"
	"HLSL_VideoFilter/resource.rc"
)

add_executable(HLSL_VideoFilter WIN32 ${HLSL_VideoFilter_sources_src} )															
add_dependencies(HLSL_VideoFilter  nv_dxut directshow_baseclass)
set_target_properties(HLSL_VideoFilter PROPERTIES PROJECT_LABEL "HLSL_VideoFilter")						
set_target_properties(HLSL_VideoFilter PROPERTIES OUTPUT_NAME "HLSL_VideoFilter")
set_target_properties(HLSL_VideoFilter PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_VideoFilter PRIVATE  
${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT ${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/directshow/baseclasses)
target_link_libraries(HLSL_VideoFilter nv_dxut directshow_baseclass ${D3DXLIBS})
install(TARGETS HLSL_VideoFilter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= AnisoDecimation =================================================
 	
add_executable(AnisoDecimation WIN32 "AnisoDecimation/AnisoDecimationApp.cpp" 
										"AnisoDecimation/AnisoDecimationApp.h"
										"AnisoDecimation/nvafx.h" 
										"AnisoDecimation/resource.h"
										"resource.rc"	)							
add_dependencies(AnisoDecimation  nv_dxut )
set_target_properties(AnisoDecimation PROPERTIES PROJECT_LABEL "AnisoDecimation")						
set_target_properties(AnisoDecimation PROPERTIES OUTPUT_NAME "AnisoDecimation")
set_target_properties(AnisoDecimation PROPERTIES FOLDER "examples/DirectX")
target_include_directories(AnisoDecimation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT)
target_link_libraries(AnisoDecimation nv_dxut ${D3DXLIBS})
install(TARGETS AnisoDecimation RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= AntiAliasingWithTransparency =================================================
file(GLOB AntiAliasingWithTransparency_sources_src
    "AntiAliasingWithTransparency/*.h"
    "AntiAliasingWithTransparency/*.cpp"
	"AntiAliasingWithTransparency/resource.h"
	"AntiAliasingWithTransparency/resource.rc"
)

 add_custom_command(OUTPUT  GrassObject.cso TerrainObject.cso
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /Fo ../MEDIA/programs/AntiAliasingWithTransparency/GrassObject.cso ../MEDIA/programs/AntiAliasingWithTransparency/GrassObject.fx
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /Fo ../MEDIA/programs/AntiAliasingWithTransparency/TerrainObject.cso ../MEDIA/programs/AntiAliasingWithTransparency/TerrainObject.fx
					    #MAIN_DEPENDENCY ../MEDIA/programs/AntiAliasingWithTransparency/GrassObject.fx
                        COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(effect_target DEPENDS GrassObject.cso TerrainObject.cso)					   
add_executable(AntiAliasingWithTransparency WIN32 ${AntiAliasingWithTransparency_sources_src})															
add_dependencies(AntiAliasingWithTransparency  nv_dxut  effect_target)
set_target_properties(AntiAliasingWithTransparency PROPERTIES PROJECT_LABEL "AntiAliasingWithTransparency")						
set_target_properties(AntiAliasingWithTransparency PROPERTIES OUTPUT_NAME "AntiAliasingWithTransparency")
set_target_properties(AntiAliasingWithTransparency PROPERTIES FOLDER "examples/DirectX")
target_include_directories(AntiAliasingWithTransparency PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT ${CMAKE_CURRENT_SOURCE_DIR}/../../nvIO)
target_link_libraries(AntiAliasingWithTransparency nv_dxut nvIO  ${D3DXLIBS})
install(TARGETS AntiAliasingWithTransparency RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
 # ================================================= BloodShader =================================================
file(GLOB BloodShader_sources_src
    "HLSL_BloodShader/*.h"
    "HLSL_BloodShader/*.cpp"
	"HLSL_BloodShader/resource.h"
	"HLSL_BloodShader/BloodShader.rc"
)

 add_custom_command(OUTPUT  Blood.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Od /Zi /Fo ../MEDIA/programs/HLSL_BloodShader/Blood.cso ../MEDIA/programs/HLSL_BloodShader/Blood.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(BloodShader_effect_target DEPENDS Blood.cso)					   
add_executable(BloodShader WIN32 ${BloodShader_sources_src} )															
add_dependencies(BloodShader  nv_dxut)
set_target_properties(BloodShader PROPERTIES PROJECT_LABEL "BloodShader")						
set_target_properties(BloodShader PROPERTIES OUTPUT_NAME "BloodShader")
set_target_properties(BloodShader PROPERTIES FOLDER "examples/DirectX")
target_include_directories(BloodShader PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT ${CMAKE_CURRENT_SOURCE_DIR}/../../nvIO)
target_link_libraries(BloodShader nv_dxut nvIO  ${D3DXLIBS})
install(TARGETS BloodShader RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= VertexMorph =================================================
file(GLOB VertexMorph_Src
    "VertexMorph.h"
    "VertexMorphApp.h"
    "VertexMorph.cpp"
    "VertexMorphApp.cpp"
	"special/nvmesh.h"
	"special/nvfile.h"
	"special/nvframe.h"
	"special/nvmesh.cpp"
	"special/nvfile.cpp"
	"special/nvframe.cpp"
	"resource.h"
	"resource.rc"
)
 add_custom_command(OUTPUT  VertexMorph.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/VertexMorph.cso ../MEDIA/programs/VertexMorph.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
add_custom_target(VertexMorph_effect_target DEPENDS VertexMorph.cso)							   
add_executable(VertexMorph WIN32 ${VertexMorph_Src})															
add_dependencies(VertexMorph  nv_dxut VertexMorph_effect_target)
set_target_properties(VertexMorph PROPERTIES PROJECT_LABEL "VertexMorph")						
set_target_properties(VertexMorph PROPERTIES OUTPUT_NAME "VertexMorph")
set_target_properties(VertexMorph PROPERTIES FOLDER "examples/DirectX")
target_include_directories(VertexMorph PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(VertexMorph nv_dxut ${D3DXLIBS})
install(TARGETS VertexMorph RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

include_directories(${DX10_INCLUDE_PATH})
LINK_DIRECTORIES(${DX10_INCLUDE_PATH}/../Lib/x86)
SET(D3DXLIBS dxerr d3dx9d d3d9 d3dxof dxguid winmm kernel32 user32 gdi32 winspool comctl32 comdlg32 advapi32 shell32 ole32 oleaut32 uuid dmoguids strmiids)
ADD_DEFINITIONS(-DNV_DXUT_DLL)	#dxut									
ADD_DEFINITIONS(-DDIRECT_SHOW_BASE_CLASS_DLL)	#directshow	

ADD_DEFINITIONS(-DNTDDI_VERSION=0x06030000)	#directshow	
ADD_DEFINITIONS(-DWINVER=0x0A00)	#directshow	
ADD_DEFINITIONS(-D_WIN32_WINNT=0x0A00)	#directshow
# ================================================= HLSL_FilterBlit =================================================
file(GLOB HLSL_FilterBlit_Src
    "FilterBlit.h"
    "FilterBlit.cpp"
	"FilterBlitApp.h"
	"FilterBlitApp.cpp"
	"resource.h"
	"common_resource.rc"
)

 add_custom_command(OUTPUT  FilterBlit.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/FilterBlit.cso ../MEDIA/programs/FilterBlit.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(FilterBlit_effect_target DEPENDS FilterBlit.cso)							   
add_executable(HLSL_FilterBlit WIN32 ${HLSL_FilterBlit_Src})															
add_dependencies(HLSL_FilterBlit  nv_dxut FilterBlit_effect_target)
set_target_properties(HLSL_FilterBlit PROPERTIES PROJECT_LABEL "HLSL_FilterBlit")						
set_target_properties(HLSL_FilterBlit PROPERTIES OUTPUT_NAME "HLSL_FilterBlit")
set_target_properties(HLSL_FilterBlit PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_FilterBlit PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(HLSL_FilterBlit nv_dxut ${D3DXLIBS})
install(TARGETS HLSL_FilterBlit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= AtlasComparisonViewer =================================================
file(GLOB AtlasComparisonViewer_Src
    "AtlasViewer.h"
    "AtlasViewer.cpp"
	"resource.h"
	"winmain.rc"
)
							   
add_executable(AtlasComparisonViewer WIN32 ${AtlasComparisonViewer_Src})															
add_dependencies(AtlasComparisonViewer  nv_dxut)
set_target_properties(AtlasComparisonViewer PROPERTIES PROJECT_LABEL "AtlasComparisonViewer")						
set_target_properties(AtlasComparisonViewer PROPERTIES OUTPUT_NAME "AtlasComparisonViewer")
set_target_properties(AtlasComparisonViewer PROPERTIES FOLDER "examples/DirectX")
target_include_directories(AtlasComparisonViewer PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(AtlasComparisonViewer nv_dxut ${D3DXLIBS})
install(TARGETS AtlasComparisonViewer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= CSAATutorial =================================================
file(GLOB CSAATutorial_Src
    "CSAATutorialApp.h"
    "CSAATutorialApp.cpp"
	"resource.h"
	"common_resource.rc"
)
 add_custom_command(OUTPUT  csstutorial.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/csstutorial.cso ../MEDIA/programs/csstutorial.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(csstutorial_effect_target DEPENDS csstutorial.cso)								   
add_executable(CSAATutorial WIN32 ${CSAATutorial_Src})															
add_dependencies(CSAATutorial  nv_dxut csstutorial_effect_target)
set_target_properties(CSAATutorial PROPERTIES PROJECT_LABEL "CSAATutorial")						
set_target_properties(CSAATutorial PROPERTIES OUTPUT_NAME "CSAATutorial")
set_target_properties(CSAATutorial PROPERTIES FOLDER "examples/DirectX")
target_include_directories(CSAATutorial PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(CSAATutorial nv_dxut ${D3DXLIBS})
install(TARGETS CSAATutorial RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= WaterInteraction =================================================
file(GLOB WaterInteraction_Src
     "WaterInteraction/*.cpp"
	 "WaterInteraction/*.h"
	 "resource.h"
	 "resource.rc"
)
						   
add_executable(WaterInteraction WIN32 ${WaterInteraction_Src})															
add_dependencies(WaterInteraction  nv_dxut nv_math   nv_d3dcommon nv_d3dmesh  )									
set_target_properties(WaterInteraction PROPERTIES PROJECT_LABEL "WaterInteraction")						
set_target_properties(WaterInteraction PROPERTIES OUTPUT_NAME "WaterInteraction")
set_target_properties(WaterInteraction PROPERTIES FOLDER "examples/DirectX")
target_include_directories(WaterInteraction PRIVATE  
 ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  
 ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon
 ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh 
 ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  
 ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(WaterInteraction nv_dxut  nv_math  nv_d3dcommon nv_d3dmesh   
	${D3DXLIBS})
install(TARGETS WaterInteraction RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= TwoSidedPolys =================================================
file(GLOB TwoSidedPolys_Src
     "TwoSidedPolysApp.cpp"
	 "TwoSidedPolysApp.h"
	 "TwoSidedPolysDemo.cpp"
	 "TwoSidedPolysDemo.h"
	 "resource.h"
	 "resource.rc"
)
						   
add_executable(TwoSidedPolys WIN32 ${TwoSidedPolys_Src})															
add_dependencies(TwoSidedPolys  nv_dxut nv_math   nv_d3dcommon nv_d3dmesh  )									
set_target_properties(TwoSidedPolys PROPERTIES PROJECT_LABEL "TwoSidedPolys")						
set_target_properties(TwoSidedPolys PROPERTIES OUTPUT_NAME "TwoSidedPolys")
set_target_properties(TwoSidedPolys PROPERTIES FOLDER "examples/DirectX")
target_include_directories(TwoSidedPolys PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(TwoSidedPolys nv_dxut  nv_math  nv_d3dcommon nv_d3dmesh   ${D3DXLIBS})
install(TARGETS TwoSidedPolys RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= Membrane =================================================
file(GLOB Membrane_Src
     "MembraneApp.cpp"
	 "MembraneApp.h"
	 "MembraneDemo.h"
	 "MembraneDemo.cpp"
	 "../shared/NV_StringFuncs.cpp"
	 "resource.h"
	 "resource.rc"
)
								
						   
add_executable(Membrane WIN32 ${Membrane_Src})															
add_dependencies(Membrane  nv_dxut nv_math  nv_d3dcommon )									
set_target_properties(Membrane PROPERTIES PROJECT_LABEL "Membrane")						
set_target_properties(Membrane PROPERTIES OUTPUT_NAME "Membrane")
set_target_properties(Membrane PROPERTIES FOLDER "examples/DirectX")
target_include_directories(Membrane PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT 
${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math
  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(Membrane nv_dxut   nv_math  nv_d3dcommon  ${D3DXLIBS})
install(TARGETS Membrane RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= PaletteSkin =================================================
file(GLOB PaletteSkin_Src
     "NVBScene_Skin.cpp"
	 "NVBScene_Skin.h"
	 "PaletteSkin.h"
	 "PaletteSkin.cpp"
	 "PaletteSkinApp.cpp"
	 "PaletteSkinApp.h"
	 "resource.h"
	 "resource.rc"
)
		add_custom_command(OUTPUT  PaletteSkin.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/PaletteSkin.cso ../MEDIA/programs/PaletteSkin.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 							
add_custom_target(PaletteSkin_effect DEPENDS PaletteSkin.cso)								   
add_executable(PaletteSkin WIN32 ${PaletteSkin_Src})															
add_dependencies(PaletteSkin  nv_dxut nv_math   nv_nvbloader nv_nvb PaletteSkin_effect)									
set_target_properties(PaletteSkin PROPERTIES PROJECT_LABEL "PaletteSkin")						
set_target_properties(PaletteSkin PROPERTIES OUTPUT_NAME "PaletteSkin")
set_target_properties(PaletteSkin PROPERTIES FOLDER "examples/DirectX")
target_include_directories(PaletteSkin PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(PaletteSkin nv_dxut  nv_math  nv_nvb nv_nvbloader ${D3DXLIBS})
install(TARGETS PaletteSkin RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= PracticalPSM =================================================
file(GLOB PracticalPSM_Src
     "PracticalPSM/*.cpp"
	 "PracticalPSM/*.h"
	 "../shared/NVBScene9.cpp"
	 "resource.h"
	 "resource.rc"
)
		add_custom_command(OUTPUT  PracticalPSM.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/PracticalPSM.cso ../MEDIA/programs/PracticalPSM.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 							
add_custom_target(PracticalPSM_effect DEPENDS PracticalPSM.cso)								   
add_executable(PracticalPSM WIN32 ${PracticalPSM_Src})															
add_dependencies(PracticalPSM  nv_dxut nv_math DX9SDKSampleFramework  nv_nvbloader nv_nvb PracticalPSM_effect)									
set_target_properties(PracticalPSM PROPERTIES PROJECT_LABEL "PracticalPSM")						
set_target_properties(PracticalPSM PROPERTIES OUTPUT_NAME "PracticalPSM")
set_target_properties(PracticalPSM PROPERTIES FOLDER "examples/DirectX")
target_include_directories(PracticalPSM PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(PracticalPSM nv_dxut  nv_math  nv_nvb nv_nvbloader DX9SDKSampleFramework ${D3DXLIBS})
install(TARGETS PracticalPSM RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= StencilShadow =================================================
file(GLOB StencilShadow_Src
     "ShadowSceneMeshes.cpp"
	 "ShadowSceneMeshes.h"
	 "StencilShadowApp.cpp"
	 "StencilShadowApp.h"
	 "StencilShadowDemo.h"
	 "StencilShadowDemo.cpp"
	 "TestStencil.cpp"
	 "TestStencil.h"
	 "../shared/UtilityFunctions.cpp"
	 "../shared/NV_StringFuncs.cpp"
	 "resource.h"
	 "resource.rc"
)
						   
add_executable(StencilShadow WIN32 ${StencilShadow_Src})															
add_dependencies(StencilShadow  nv_dxut nv_math   nv_d3dcommon nv_d3dmesh  )									
set_target_properties(StencilShadow PROPERTIES PROJECT_LABEL "StencilShadow")						
set_target_properties(StencilShadow PROPERTIES OUTPUT_NAME "StencilShadow")
set_target_properties(StencilShadow PROPERTIES FOLDER "examples/DirectX")
target_include_directories(StencilShadow PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(StencilShadow nv_dxut  nv_math  nv_d3dcommon nv_d3dmesh   ${D3DXLIBS})
install(TARGETS StencilShadow RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= FogPolygonVolumes =================================================
file(GLOB FogPolygonVolumes_Src
     "FogPolygonVolumes3/*.cpp"
	 "FogPolygonVolumes3/*.h"
	 "resource.h"
	 "resource.rc"
)
							   
add_executable(FogPolygonVolumes WIN32 ${FogPolygonVolumes_Src})															
add_dependencies(FogPolygonVolumes  nv_dxut nv_d3dcommon nv_d3dmesh)
set_target_properties(FogPolygonVolumes PROPERTIES PROJECT_LABEL "FogPolygonVolumes")						
set_target_properties(FogPolygonVolumes PROPERTIES OUTPUT_NAME "FogPolygonVolumes")
set_target_properties(FogPolygonVolumes PROPERTIES FOLDER "examples/DirectX")
target_include_directories(FogPolygonVolumes PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(FogPolygonVolumes nv_dxut  nv_math nv_d3dcommon nv_d3dmesh ${D3DXLIBS})
install(TARGETS FogPolygonVolumes RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= FresnelReflection =================================================
file(GLOB FresnelReflection_Src
     "FresnelReflection.cpp"
	 "FresnelReflection.h"
	 "FresnelReflectionApp.h"
	 "FresnelReflectionApp.cpp"
	 "../shared/NVBScene9.cpp"
	 "resource.h"
	 "resource.rc"
)

 add_custom_command(OUTPUT  FresnelReflection.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/FresnelReflection.cso ../MEDIA/programs/FresnelReflection.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
					   
add_custom_target(FresnelReflection_effect_target DEPENDS FresnelReflection.cso)							   
add_executable(FresnelReflection WIN32 ${FresnelReflection_Src})															
add_dependencies(FresnelReflection  nv_dxut FresnelReflection_effect_target)
set_target_properties(FresnelReflection PROPERTIES PROJECT_LABEL "FresnelReflection")						
set_target_properties(FresnelReflection PROPERTIES OUTPUT_NAME "FresnelReflection")
set_target_properties(FresnelReflection PROPERTIES FOLDER "examples/DirectX")
target_include_directories(FresnelReflection PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(FresnelReflection nv_dxut nv_nvb nv_math nv_nvbloader ${D3DXLIBS})
install(TARGETS FresnelReflection RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= FrontBackRegister =================================================
file(GLOB FrontBackRegister_Src
     "FrontBackRegisterApp.cpp"
	 "FrontBackRegisterApp.h"
)
							   
add_executable(FrontBackRegister WIN32 ${FrontBackRegister_Src})															
add_dependencies(FrontBackRegister  nv_dxut )
set_target_properties(FrontBackRegister PROPERTIES PROJECT_LABEL "FrontBackRegister")						
set_target_properties(FrontBackRegister PROPERTIES OUTPUT_NAME "FrontBackRegister")
set_target_properties(FrontBackRegister PROPERTIES FOLDER "examples/DirectX")
target_include_directories(FrontBackRegister PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(FrontBackRegister nv_dxut   nv_math  nv_d3dcommon nv_d3dmesh DX9SDKSampleFramework ${D3DXLIBS})
install(TARGETS FrontBackRegister RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= Glow =================================================
file(GLOB Glow_Src
     "Glow/*.cpp"
	 "Glow/*.h"
	 "resource.h"
	 "resource.rc"
)
							   
add_executable(Glow WIN32 ${Glow_Src})															
add_dependencies(Glow  nv_dxut )
set_target_properties(Glow PROPERTIES PROJECT_LABEL "Glow")						
set_target_properties(Glow PROPERTIES OUTPUT_NAME "Glow")
set_target_properties(Glow PROPERTIES FOLDER "examples/DirectX")
target_include_directories(Glow PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(Glow nv_dxut   nv_math  nv_d3dcommon nv_d3dmesh DX9SDKSampleFramework ${D3DXLIBS})
install(TARGETS Glow RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= HLSL_SoftShadows =================================================
file(GLOB HLSL_SoftShadows_Src
     "HLSL_SoftShadowsApp.cpp"
	 "HLSL_SoftShadowsApp.h"
	 "HLSL_SoftShadows.h"
	 "HLSL_SoftShadows.cpp"
	 "../shared/NVBScene9.cpp"
	 "resource.h"
	 "resource.rc"
)
					
 add_custom_command(OUTPUT  HLSL_SoftShadows.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/HLSL_SoftShadows.cso ../MEDIA/programs/HLSL_SoftShadows.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 					

add_custom_target(HLSL_SoftShadows_effect DEPENDS HLSL_SoftShadows.cso)							   
add_executable(HLSL_SoftShadows WIN32 ${HLSL_SoftShadows_Src})															
add_dependencies(HLSL_SoftShadows  nv_dxut HLSL_SoftShadows_effect)									
set_target_properties(HLSL_SoftShadows PROPERTIES PROJECT_LABEL "HLSL_SoftShadows")						
set_target_properties(HLSL_SoftShadows PROPERTIES OUTPUT_NAME "HLSL_SoftShadows")
set_target_properties(HLSL_SoftShadows PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_SoftShadows PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(HLSL_SoftShadows nv_dxut   nv_math  nv_nvb nv_nvbloader  ${D3DXLIBS})
install(TARGETS HLSL_SoftShadows RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= Mandelbrot =================================================
file(GLOB Mandelbrot_Src
     "Mandelbrot.cpp"
	 "Mandelbrot.h"
	 "MandelbrotApp.h"
	 "MandelbrotApp.cpp"
	 "MandelbrotGUI.cpp"
	 "MandelbrotGUI.h"
	 "resource.h"
	 "resource.rc"
)
								
						   
add_executable(Mandelbrot WIN32 ${Mandelbrot_Src})															
add_dependencies(Mandelbrot  nv_dxut nv_math  nv_d3dcommon nv_d3dmesh)									
set_target_properties(Mandelbrot PROPERTIES PROJECT_LABEL "Mandelbrot")						
set_target_properties(Mandelbrot PROPERTIES OUTPUT_NAME "Mandelbrot")
set_target_properties(Mandelbrot PROPERTIES FOLDER "examples/DirectX")
target_include_directories(Mandelbrot PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DCommon ${CMAKE_CURRENT_SOURCE_DIR}/../../NV_D3DMesh ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math  ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/)
target_link_libraries(Mandelbrot nv_dxut   nv_math  nv_d3dcommon nv_d3dmesh     ${D3DXLIBS})
install(TARGETS Mandelbrot RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= SnowAccumulation =================================================
file(GLOB SnowAccumulation_Src
    "SnowAccumulation.cpp"
	"resource.h"
	"SnowAccumulation.rc"
)
 add_custom_command(OUTPUT  OrthoDepth.cso  Snow.cso
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/SnowAccumulation/OrthoDepth.cso ../MEDIA/programs/SnowAccumulation/OrthoDepth.fx.
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/SnowAccumulation/Snow.cso ../MEDIA/programs/SnowAccumulation/Snow.fx.
					   
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
add_custom_target(SnowAccumulation_effect_target DEPENDS OrthoDepth.cso Snow.cso)								   
add_executable(SnowAccumulation WIN32 ${SnowAccumulation_Src})															
add_dependencies(SnowAccumulation  nv_dxut  SnowAccumulation_effect_target)
set_target_properties(SnowAccumulation PROPERTIES PROJECT_LABEL "SnowAccumulation")						
set_target_properties(SnowAccumulation PROPERTIES OUTPUT_NAME "SnowAccumulation")
set_target_properties(SnowAccumulation PROPERTIES FOLDER "examples/DirectX")
target_include_directories(SnowAccumulation PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(SnowAccumulation nv_dxut  ${D3DXLIBS})
install(TARGETS SnowAccumulation RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= HLSL_VertexLight =================================================
file(GLOB HLSL_VertexLight_Src
    "VertexLight.h"
    "VertexLightApp.h"
    "VertexLight.cpp"
    "VertexLightApp.cpp"
	"special/nvmesh.h"
	"special/nvmesh.cpp"
	"resource.h"
	"resource.rc"
)
 add_custom_command(OUTPUT  VertexLight.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/VertexLight.cso ../MEDIA/programs/VertexLight.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
add_custom_target(HLSL_VertexLight_effect_target DEPENDS VertexLight.cso)							   
add_executable(HLSL_VertexLight WIN32 ${HLSL_VertexLight_Src})															
add_dependencies(HLSL_VertexLight  nv_dxut )
set_target_properties(HLSL_VertexLight PROPERTIES PROJECT_LABEL "HLSL_VertexLight")						
set_target_properties(HLSL_VertexLight PROPERTIES OUTPUT_NAME "HLSL_VertexLight")
set_target_properties(HLSL_VertexLight PROPERTIES FOLDER "examples/DirectX")
target_include_directories(HLSL_VertexLight PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(HLSL_VertexLight nv_dxut ${D3DXLIBS})
install(TARGETS HLSL_VertexLight RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# ================================================= VertexTextureFetchWater =================================================
file(GLOB VertexTextureFetchWater_Src
    "VertexTextureFetchWater/fade.h"
    "VertexTextureFetchWater/Grid.h"
    "VertexTextureFetchWater/Grid.cpp"
    "VertexTextureFetchWater/HeightMap.cpp"
    "VertexTextureFetchWater/HeightMap.h"
    "VertexTextureFetchWater/Mesh.h"
    "VertexTextureFetchWater/Mesh.cpp"
	"VertexTextureFetchWater/registers.h"
	"VertexTextureFetchWater/Vertex.h"
	"VertexTextureFetchWater/Viewer.h"
	"VertexTextureFetchWater/Viewer.cpp"
	"VertexTextureFetchWater/Water.cpp"
	"VertexTextureFetchWater/Water.h"
	"resource.h"
	"VertexTextureFetchWater/VertexTextureFetchWater.rc"
)

 add_custom_command(OUTPUT  simulation.vertexShader  simulation.pixelShader 
							perturbSimulation.vertexShader perturbSimulation.pixelShader
							caustic.vertexShader caustic.pixelShader 
							diffuse.vertexShader diffuse.pixelShader 
							singleTexture.vertexShader singleTexture.pixelShader
							render.pixelShader  render.pixelRefractionShader render.pixelReflectionlShader render.pixelFresnelShader render.pixelNormalsShader
							underwater.pixelShader
					   COMMAND ${FXC} /T "vs_1_1" /Gec /Od /Zi /E vertexShader /Fo ../MEDIA/programs/VertexTextureFetchWater/simulation.vertexShader ../MEDIA/programs/VertexTextureFetchWater/simulation.hlsl
					   COMMAND ${FXC} /T "ps_2_0" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/simulation.pixelShader ../MEDIA/programs/VertexTextureFetchWater/simulation.hlsl
					   
					   COMMAND ${FXC} /T "vs_2_0" /Gec /Od /Zi /E vertexShader /Fo ../MEDIA/programs/VertexTextureFetchWater/perturbSimulation.vertexShader ../MEDIA/programs/VertexTextureFetchWater/perturbSimulation.hlsl
					   COMMAND ${FXC} /T "ps_2_0" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/perturbSimulation.pixelShader ../MEDIA/programs/VertexTextureFetchWater/perturbSimulation.hlsl
					  
					   COMMAND ${FXC} /T "vs_1_1" /Gec /Od /Zi /E vertexShader /Fo ../MEDIA/programs/VertexTextureFetchWater/caustic.vertexShader ../MEDIA/programs/VertexTextureFetchWater/caustic.hlsl
					   COMMAND ${FXC} /T "ps_2_0" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/caustic.pixelShader ../MEDIA/programs/VertexTextureFetchWater/caustic.hlsl
					   
					   COMMAND ${FXC} /T "vs_1_1" /Gec /Od /Zi /E vertexShader /Fo ../MEDIA/programs/VertexTextureFetchWater/diffuse.vertexShader ../MEDIA/programs/VertexTextureFetchWater/diffuse.hlsl
					   COMMAND ${FXC} /T "ps_1_1" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/diffuse.pixelShader ../MEDIA/programs/VertexTextureFetchWater/diffuse.hlsl
					   
					   COMMAND ${FXC} /T "vs_1_1" /Gec /Od /Zi /E vertexShader /Fo ../MEDIA/programs/VertexTextureFetchWater/singleTexture.vertexShader ../MEDIA/programs/VertexTextureFetchWater/singleTexture.hlsl
					   COMMAND ${FXC} /T "ps_1_1" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/singleTexture.pixelShader ../MEDIA/programs/VertexTextureFetchWater/singleTexture.hlsl
					   
					   COMMAND ${FXC} /T "ps_3_0" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/render.pixelShader ../MEDIA/programs/VertexTextureFetchWater/render.hlsl
					   COMMAND ${FXC} /T "ps_3_0" /Gec /Od /Zi /E pixelShader /D RETURN_REFRACTION /Fo ../MEDIA/programs/VertexTextureFetchWater/render.pixelRefractionShader ../MEDIA/programs/VertexTextureFetchWater/render.hlsl
					   COMMAND ${FXC} /T "ps_3_0" /Gec /Od /Zi /E pixelShader /D RETURN_REFLECTION /Fo ../MEDIA/programs/VertexTextureFetchWater/render.pixelReflectionlShader ../MEDIA/programs/VertexTextureFetchWater/render.hlsl
					   COMMAND ${FXC} /T "ps_3_0" /Gec /Od /Zi /E pixelShader /D RETURN_FRESNEL /Fo ../MEDIA/programs/VertexTextureFetchWater/render.pixelFresnelShader ../MEDIA/programs/VertexTextureFetchWater/render.hlsl
					   COMMAND ${FXC} /T "ps_3_0" /Gec /Od /Zi /E pixelShader /D RETURN_NORMALS /Fo ../MEDIA/programs/VertexTextureFetchWater/render.pixelNormalsShader ../MEDIA/programs/VertexTextureFetchWater/render.hlsl
					   
					   COMMAND ${FXC} /T "ps_2_0" /Gec /Od /Zi /E pixelShader /Fo ../MEDIA/programs/VertexTextureFetchWater/underwater.pixelShader ../MEDIA/programs/VertexTextureFetchWater/underwater.hlsl
					  
					   COMMENT "Effect-compile"
                        VERBATIM
					   )
add_custom_target(VertexTextureFetchWater_effect_target DEPENDS 
	simulation.vertexShader  
	simulation.pixelShader 
	perturbSimulation.vertexShader 
	perturbSimulation.pixelShader 
	render.pixelShader  
	render.pixelRefractionShader 
	render.pixelReflectionlShader 
	render.pixelFresnelShader 
	render.pixelNormalsShader
	underwater.pixelShader 
	caustic.vertexShader 
	caustic.pixelShader 
	diffuse.vertexShader 
	diffuse.pixelShader 
	singleTexture.vertexShader 
	singleTexture.pixelShader
					  )
add_executable(VertexTextureFetchWater WIN32 ${VertexTextureFetchWater_Src})															
add_dependencies(VertexTextureFetchWater  nv_dxut VertexTextureFetchWater_effect_target)

set_target_properties(VertexTextureFetchWater PROPERTIES PROJECT_LABEL "VertexTextureFetchWater")						
set_target_properties(VertexTextureFetchWater PROPERTIES OUTPUT_NAME "VertexTextureFetchWater")
set_target_properties(VertexTextureFetchWater PROPERTIES FOLDER "examples/DirectX")
target_include_directories(VertexTextureFetchWater PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT )
target_link_libraries(VertexTextureFetchWater nv_dxut ${D3DXLIBS})
install(TARGETS VertexTextureFetchWater RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
 
# ================================================= BumpyShinyVariations =================================================
file(GLOB BumpyShinyVariations_Src
     "BumpyShinyVariations.cpp"
	 "BumpyShinyVariations.h"
	 "BumpyShinyVariationsApp.h"
	 "BumpyShinyVariationsApp.cpp"
	 "../shared/NVBScene9.cpp"
	 "resource.h"
	 "resource.rc"
)

 add_custom_command(OUTPUT  BumpyShinyVariations.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/VertexMorph.cso ../MEDIA/programs/BumpyShinyVariations.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
					   
add_custom_target(BumpyShinyVariations_effect_target DEPENDS BumpyShinyVariations.cso)							   
add_executable(BumpyShinyVariations WIN32 ${BumpyShinyVariations_Src})															
add_dependencies(BumpyShinyVariations  nv_dxut BumpyShinyVariations_effect_target)
set_target_properties(BumpyShinyVariations PROPERTIES PROJECT_LABEL "BumpyShinyVariations")						
set_target_properties(BumpyShinyVariations PROPERTIES OUTPUT_NAME "BumpyShinyVariations")
set_target_properties(BumpyShinyVariations PROPERTIES FOLDER "examples/DirectX")
target_include_directories(BumpyShinyVariations PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(BumpyShinyVariations nv_dxut nv_nvb nv_math nv_nvbloader ${D3DXLIBS})
install(TARGETS BumpyShinyVariations RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= DeferredShading =================================================
file(GLOB DeferredShading_Src
     "DeferredShading.cpp"
	 "DeferredShading.h"
	 "DeferredShadingApp.h"
	 "DeferredShadingApp.cpp"
	 "NVBScene9_DS.cpp"
	 "resource.h"
	 "resource.rc"
)

 add_custom_command(OUTPUT  DeferredShading.cso  
					   COMMAND ${FXC} /T "fx_2_0" /Gec /Od /Zi /Fo ../MEDIA/programs/DeferredShading.cso ../MEDIA/programs/DeferredShading.fx
					   COMMENT "Effect-compile"
                        VERBATIM
					   ) 
					   
add_custom_target(DeferredShading_effect_target DEPENDS DeferredShading.cso)							   
add_executable(DeferredShading WIN32 ${DeferredShading_Src})															
add_dependencies(DeferredShading  nv_dxut DeferredShading_effect_target)
set_target_properties(DeferredShading PROPERTIES PROJECT_LABEL "DeferredShading")						
set_target_properties(DeferredShading PROPERTIES OUTPUT_NAME "DeferredShading")
set_target_properties(DeferredShading PROPERTIES FOLDER "examples/DirectX")
target_include_directories(DeferredShading PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../../DXUT  ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvb ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_math ${CMAKE_CURRENT_SOURCE_DIR}/../../nv_nvbloader)
target_link_libraries(DeferredShading nv_dxut nv_nvb nv_math nv_nvbloader ${D3DXLIBS})
install(TARGETS DeferredShading RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


