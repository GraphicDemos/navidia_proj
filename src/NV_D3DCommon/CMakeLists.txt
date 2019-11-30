PROJECT(nv_d3dcommon)

file(GLOB nv_d3dcommon_files
    "*.h"
    "*.cpp"
	"../examples/shared/NV_StringFuncs.cpp"
	"../examples/shared/UtilityFunctions.cpp"
)
         
LINK_DIRECTORIES(${DX10_INCLUDE_PATH}/../Lib/x86)			 
# targets
ADD_DEFINITIONS(-DNV_D3D_COMMON_EXPORTS)

IF(NV_D3D_COMMON_SHARED)
	ADD_DEFINITIONS(-DNV_D3D_COMMON_DLL=1)
	ADD_LIBRARY(nv_d3dcommon SHARED ${nv_d3dcommon_files})
ELSE(NV_D3D_COMMON_SHARED)
	ADD_LIBRARY(nv_d3dcommon ${nv_d3dcommon_files})
ENDIF(NV_D3D_COMMON_SHARED)

target_include_directories( nv_d3dcommon PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/ ${DX10_INCLUDE_PATH} ../nv_d3dmesh ../examples/ ${DX10_INCLUDE_PATH} ../extern/ )

TARGET_LINK_LIBRARIES(nv_d3dcommon ${LIBS} nv_d3dmesh ${D3D9_LIBRARY}  ${D3DX9_LIBRARY} ${D3D10_LIBRARY} ${D3Dx10_LIBRARY})

INSTALL(TARGETS nv_d3dcommon
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
