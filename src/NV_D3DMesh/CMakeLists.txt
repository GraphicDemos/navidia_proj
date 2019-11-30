PROJECT(nv_d3dmesh)

file(GLOB nv_d3dmesh_files
    "*.h"
    "*.cpp"
	"../examples/shared/NV_StringFuncs.cpp"
	"../examples/shared/UtilityFunctions.cpp"
	"../examples/shared/NoiseGrid3D.cpp"
)
         
LINK_DIRECTORIES(${DX10_INCLUDE_PATH}/../Lib/x86)		 
# targets
ADD_DEFINITIONS(-DNV_D3D_MESH_EXPORTS)

IF(NV_D3D_MESH_SHARED)
	ADD_DEFINITIONS(-DNV_D3D_MESH_DLL=1)
	ADD_LIBRARY(nv_d3dmesh SHARED ${nv_d3dmesh_files})
	
ELSE(NV_D3D_MESH_SHARED)
	ADD_LIBRARY(nv_d3dmesh ${nv_d3dmesh_files})
ENDIF(NV_D3D_MESH_SHARED)

target_include_directories( nv_d3dmesh PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/ ${DX10_INCLUDE_PATH} ../examples/ ${DX10_INCLUDE_PATH} ../extern/)

TARGET_LINK_LIBRARIES(nv_d3dmesh ${LIBS} ${D3D9_LIBRARY} ${D3DX9_LIBRARY} ${D3D10_LIBRARY} ${D3Dx10_LIBRARY})

INSTALL(TARGETS nv_d3dmesh
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
