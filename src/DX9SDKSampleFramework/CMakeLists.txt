PROJECT(DX9SDKSampleFramework)

SET(DX9SDKSampleFramework_SRCS
	d3dapp.cpp d3dapp.h
	d3denumeration.cpp d3denumeration.h
	d3dfile.cpp D3DFile.h
	D3DFont.h d3dfont.cpp
	D3DRes.h 
	d3dsaver.cpp D3DSaver.h
	d3dsettings.cpp d3dsettings.h
	d3dutil.cpp d3dutil.h
	DX9SDKSampleFramework.h
	dxstdafx.h nv_dx9_sdk_sample_decl.h
	dxutil.cpp dxutil.h
	)
                  
# targets
LINK_DIRECTORIES(${DX10_INCLUDE_PATH}/../Lib/x86)	
ADD_DEFINITIONS(-DNV_NVB_DX9SDKSampleFramework_LOADER_EXPORTS)

IF(NV_DX9SDKSampleFramework_SHARED)
	ADD_DEFINITIONS(-DNV_NVB_DX9SDKSampleFramework_DLL=1)
	ADD_LIBRARY(DX9SDKSampleFramework SHARED ${DX9SDKSampleFramework_SRCS})
ELSE(NV_DX9SDKSampleFramework_SHARED)
	ADD_LIBRARY(DX9SDKSampleFramework ${DX9SDKSampleFramework_SRCS})
ENDIF(NV_DX9SDKSampleFramework_SHARED)


target_include_directories( DX9SDKSampleFramework PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/ ${DX10_INCLUDE_PATH} )


TARGET_LINK_LIBRARIES(DX9SDKSampleFramework  ${LIBS} ${D3D9_LIBRARY} ${D3DX9_LIBRARY} ${D3DXGUID_LIBRARY})

INSTALL(TARGETS DX9SDKSampleFramework
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
