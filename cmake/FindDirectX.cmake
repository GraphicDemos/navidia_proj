
IF (WIN32)

	FIND_PATH(DX9_INCLUDE_PATH d3d9.h
		PATHS
			"$ENV{DXSDK_DIR}/Include"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Include"
		DOC "The directory where D3D9.h resides")

	FIND_PATH(DX10_INCLUDE_PATH D3D10.h
		PATHS
			"$ENV{DXSDK_DIR}/Include"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Include"
		DOC "The directory where D3D10.h resides")

	FIND_LIBRARY(D3D9_LIBRARY d3d9.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/64"
		DOC "The directory where d3d9.lib resides")
	
	FIND_LIBRARY(D3DX9_LIBRARY d3dx9.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
		DOC "The directory where d3dx9.lib resides")

	FIND_LIBRARY(D3D10_LIBRARY d3d10.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
		DOC "The directory where d3d10.lib resides")

	FIND_LIBRARY(D3DX10_LIBRARY d3dx10.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
		DOC "The directory where d3dx10.lib resides")
	
	FIND_LIBRARY(D3DERROR_LIBRARY DxErr.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
		DOC "The directory where DxErr.lib resides")
		
	FIND_LIBRARY(D3DXGUID_LIBRARY dxguid.lib
		PATHS
			"$ENV{DXSDK_DIR}/Lib/x64"
			"$ENV{PROGRAMFILES}/Microsoft DirectX SDK/Lib/x64"
		DOC "The directory where dxguid.lib resides")	
	

	SET(DX10_LIBRARIES ${D3D10_LIBRARY} ${D3DX10_LIBRARY} ${D3DERROR_LIBRARY} ${D3DX9_LIBRARY} ${D3D9_LIBRARY})

ENDIF (WIN32)

IF (DX10_INCLUDE_PATH)
	SET( DX10_FOUND 1 CACHE STRING "Set to 1 if CG is found, 0 otherwise")
ELSE (DX10_INCLUDE_PATH)
	SET( DX10_FOUND 0 CACHE STRING "Set to 1 if CG is found, 0 otherwise")
ENDIF (DX10_INCLUDE_PATH)

MARK_AS_ADVANCED( DX10_FOUND )
