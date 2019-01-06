/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  SDK\LIBS\src\NV_D3DCommon\
File:  ShaderManager.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/

#include <shared/UtilityFunctions.h>

#include <string.h>

#ifdef NV_D3DCOMMON_NOASSERT
	#undef BREAK_AND_RET_VAL_IF_FAILED
	#define BREAK_AND_RET_VAL_IF_FAILED(h)		return( h );
#endif

#if 0
	#define TRACE0	FMsg
	#define SM_TRACE0_ON
#else
	#define TRACE0	NullFunc
#endif


////////////////////////////////////////////////////////

#include "ShaderManager.h"
ShaderManager::ShaderManager()
{
	m_Shaders.clear();
	m_ShaderIndices.clear();
	m_ShaderIndirections.clear();

	m_pD3DDev = NULL;
}

ShaderManager::~ShaderManager()
{
	Free();
}


////////////////////////////////////////////////////////


HRESULT ShaderManager::Free()
{
	size_t i;
	HRESULT hr;

	if( m_pD3DDev != NULL )
	{
		m_pD3DDev->SetPixelShader( 0 );
		m_pD3DDev->SetVertexShader( NULL );
		m_pD3DDev->SetFVF( D3DFVF_XYZ );

		// Free vertex & pixel shaders
		// Start from 1.  
		// Shader 0 is always null shader
		for( i=1; i < m_Shaders.size() ; i++ )
		{
			hr = FreeShader( &(m_Shaders[i]) );
		}
		SAFE_RELEASE( m_pD3DDev );		// we AddRef()'d in Initialize
	}

	m_Shaders.clear();
	m_ShaderIndices.clear();
	m_ShaderIndirections.clear();
	return( S_OK );
}


HRESULT ShaderManager::FreeShader( ShaderDescription * pDesc )
{
	FAIL_IF_NULL( pDesc );
	IDirect3DVertexShader9	* pvsh;
	IDirect3DPixelShader9	* ppsh;

	switch( pDesc->m_Type )
	{
	case	SM_SHADERTYPE_VERTEX :
		pvsh = (IDirect3DVertexShader9*) pDesc->m_pShader;
		SAFE_RELEASE( pvsh );
		pDesc->m_pShader = NULL;
		break;

	case	SM_SHADERTYPE_PIXEL :
		ppsh = (IDirect3DPixelShader9*) pDesc->m_pShader;
		SAFE_RELEASE( ppsh );
		pDesc->m_pShader = NULL;
		break;
	}
	return( S_OK );
}


HRESULT ShaderManager::Initialize( IDirect3DDevice9 * pDev, GetFilePath::GetFilePathFunction file_path_callback )
{
#ifdef SM_TRACE0_ON
	TRACE0("ShaderManager::Initialize... Freeing these shaders:\n");
	ListAllShaders();
#endif

	Free();
	FAIL_IF_NULL( pDev );

	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();
	m_pGetFilePathFunctionSM = file_path_callback;

	// Add one "NULL" shader with handle value of 0
	//  so that SetShader(0) sets a null shader
	//  to disable the vertex shader;
	ShaderDescription Desc;
	Desc.m_ShaderDefinitionFile = TEXT("NULL Shader");
	Desc.m_ShaderObjectFile		= TEXT("NULL Shader");
	Desc.m_Type					= SM_SHADERTYPE_VERTEX;
	Desc.m_pShader				= NULL;
	m_Shaders.push_back( Desc );

	return( S_OK );
}


void ShaderManager::SetFilePathCallback( GetFilePath::GetFilePathFunction pFilePathCallback )
{
	m_pGetFilePathFunctionSM = pFilePathCallback;
}


tstring ShaderManager::GetShaderObjName( SM_SHADER_INDEX index )
{
	tstring name;
	if( index < m_Shaders.size() && index >= 0 )
	{
		return( m_Shaders.at(index).m_ShaderObjectFile );
	}
	else
	{
		FMsg("Index out of bounds!\n");
		return( TEXT("Index out of bounds!") );
	}
	return( TEXT("") );
}


void	ShaderManager::ListAllShaders()
{
	unsigned int i;

	FMsg("\nList of all ShaderManager shaders: %d\n", m_Shaders.size() );
	for( i = 0; i < m_Shaders.size(); i++ )
	{
		FMsg("Shader %3.3d   %s\n", i, GetShaderObjName(i).c_str() );
	}
}



tstring	ShaderManager::GetFilePath( const tstring & file_name, bool bVerbose )
{
	tstring filepath;
	if( m_pGetFilePathFunctionSM != NULL )
	{
		filepath = (*m_pGetFilePathFunctionSM)( file_name, bVerbose );
	}
	else
	{
		filepath = file_name;
	}
	return( filepath );
}



HRESULT ShaderManager::CreateShader( LPD3DXBUFFER pbufShader,
										SM_SHADERTYPE ShaderType,
										ShaderDescription * pDesc )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pbufShader );
	FAIL_IF_NULL( pDesc );
	FAIL_IF_NULL( m_pD3DDev );

	pDesc->m_Type			= ShaderType;

	if( ShaderType == SM_SHADERTYPE_VERTEX )
	{
		hr = m_pD3DDev->CreateVertexShader( (DWORD*) pbufShader->GetBufferPointer(),
											 (IDirect3DVertexShader9**) & pDesc->m_pShader );
	    SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED(hr);		
	}
	else
	{
		hr = m_pD3DDev->CreatePixelShader( (DWORD*) pbufShader->GetBufferPointer(),
											(IDirect3DPixelShader9**) & pDesc->m_pShader );
	    SAFE_RELEASE( pbufShader );		
		BREAK_AND_RET_VAL_IF_FAILED(hr);				
	}
	return( hr );
}



// Creates a vertex shader from a vertex shader text buffer in memory
// The text buffer is an uncompiled vertex shader program
HRESULT ShaderManager::LoadAndCreateShaderFromMemory( const char * program_asm_code,	// ASCII assembly code
														const TCHAR * shader_name,
														SM_SHADERTYPE ShaderType,
														SM_SHADER_INDEX * outIndex )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( program_asm_code );
	FAIL_IF_NULL( outIndex );

	//@@@@@ check if shader exists!

	ShaderDescription	Desc;
	Desc.m_ShaderDefinitionFile = TEXT("");
	Desc.m_Type					= ShaderType;
	Desc.m_ShaderObjectFile		= shader_name;

	//	typedef struct D3DXMACRO {
	//		LPCSTR Name;
	//		LPCSTR Definition;	};
    LPD3DXBUFFER pbufShader, pbufErrors;

	hr = D3DXAssembleShader( program_asm_code, 
							(UINT) strlen( program_asm_code ),
							 NULL,		// D3DXMACRO preprocessor definitions
							 NULL,		// include directives
							 NULL,		// compile option flags
							 & pbufShader,
							 & pbufErrors );
	if( FAILED( hr ))
	{
		char * pfunc = "D3DXAssembleShader";
		switch( hr )
		{
		case D3DERR_INVALIDCALL : 
			FMsg("%s failed with HRESULT = D3DERR_INVALIDCALL\n", pfunc );
			break;
		case D3DXERR_INVALIDDATA : 
			FMsg("%s failed with HRESULT = D3DXERR_INVALIDDATA\n", pfunc );
			break;
		case E_OUTOFMEMORY : 
			FMsg("%s failed with HRESULT = E_OUTOFMEMORY\n", pfunc );
			break;
		default : 
			FMsg("Unknown HRESULT : %u\n", hr );
			break;
		}

		FMsg("ShaderManager::D3DXAssembleShader Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
	    SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, ShaderType, & Desc );

	// Add shader description to the array, set its index and return the index
	//@@@@ make function for this!!	
	m_Shaders.push_back( Desc );
	* outIndex = (SM_SHADER_INDEX) m_Shaders.size() - 1;
	m_ShaderIndices.push_back( *outIndex );
	return( hr );
}

//----------------------------------------------------------------------------

HRESULT ShaderManager::LoadAndCreateShader( ShaderDescription * pDesc )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDesc );
	FAIL_IF_NULL( m_pD3DDev );

	FreeShader( pDesc );

	tstring		strFilePath;
	strFilePath = GetFilePath( pDesc->m_ShaderObjectFile );

	try	{
		HANDLE hFile;
		HRESULT hr;

		hFile = CreateFile( strFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE) 
		{
			FDebug("Could not find file %s\n", strFilePath.c_str() );
			return E_FAIL;
		}
		
		DWORD dwFileSize = GetFileSize( hFile, NULL );
		
		const DWORD* pShader = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
		if (!pShader)
		{
			FMsg( "Failed to allocate memory to load shader %s\n", strFilePath.c_str() );
			return E_FAIL;
		}
		
		ReadFile(hFile, (void*)pShader, dwFileSize, &dwFileSize, NULL);
		
		CloseHandle(hFile);
		
		if( pDesc->m_Type == SM_SHADERTYPE_VERTEX )
		{
			hr = m_pD3DDev->CreateVertexShader( pShader, (IDirect3DVertexShader9**) & pDesc->m_pShader );
		} 
		else if( pDesc->m_Type == SM_SHADERTYPE_PIXEL )
		{
			hr = m_pD3DDev->CreatePixelShader( pShader, (IDirect3DPixelShader9**) & pDesc->m_pShader );
		}
		HeapFree(GetProcessHeap(), 0, (void*)pShader);
		
		if (FAILED(hr))	
		{
			FDebug("Failed to create shader %s\n", strFilePath.c_str() );
			return E_FAIL;
		}
	}
	catch(...)
	{
		FDebug("Error opening file %s\n", strFilePath.c_str() );
		return E_FAIL;
	}

	return( hr );
}


//  Loads a .vso or .pso shader object file from strFilePath.
HRESULT ShaderManager::LoadAndCreateShader(	const tstring & strFilePath,
											SM_SHADERTYPE ShaderType,
											SM_SHADER_INDEX * outIndex	)
{
	HRESULT hr;
	tstring filepath = TEXT("");
	hr = LoadAndCreateShader( strFilePath, filepath, ShaderType, outIndex );
	return( hr );
}



//  Loads a compiled shader binary file (.vso or .pso) from strFilePath.
//  Also tracks the original ASCII text file name from strSourceFile, but
//   this is not required.  This could be the .nvv or .nvp file path.
HRESULT	ShaderManager::LoadAndCreateShader(	const tstring & file_name,
											const tstring & strSourceFile,
											SM_SHADERTYPE ShaderType,
											SM_SHADER_INDEX * outIndex )
{
	// strSourceFile is optional ASCII assembly code file and not used
	//  for shader creation.
	FAIL_IF_NULL( m_pD3DDev );
	tstring	strFilePath;
	strFilePath = GetFilePath( file_name );

	// Check to see if shader has already been loaded & created
	SM_SHADER_INDEX index;
	if( IsShaderLoaded( strFilePath, & index ))
	{
		FDebug("Shader [%s] already loaded and has index %u\n", strFilePath.c_str(), index );	
		*outIndex = index;
		return( S_OK );
	}

	try	{
		HANDLE hFile;
		HRESULT hr;
		ShaderDescription	Desc;
		Desc.m_ShaderDefinitionFile = TEXT("");
		Desc.m_ShaderObjectFile		= strFilePath.c_str();
		Desc.m_Type					= ShaderType;

		hFile = CreateFile( strFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE) 
		{
			FDebug("Could not find file %s\n", strFilePath.c_str() );
			return E_FAIL;
		}
		
		DWORD dwFileSize = GetFileSize( hFile, NULL );
		const DWORD* pShader = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
		if (!pShader)
		{
			FMsg( "Failed to allocate memory to load shader %s\n", strFilePath.c_str() );
			return E_FAIL;
		}
		
		ReadFile(hFile, (void*)pShader, dwFileSize, &dwFileSize, NULL);
		CloseHandle(hFile);
		if (ShaderType == SM_SHADERTYPE_VERTEX )
		{
			hr = m_pD3DDev->CreateVertexShader( pShader, (IDirect3DVertexShader9**) & Desc.m_pShader );
		} 
		else if (ShaderType == SM_SHADERTYPE_PIXEL )
		{
			hr = m_pD3DDev->CreatePixelShader( pShader, (IDirect3DPixelShader9**) & Desc.m_pShader );
		}

		HeapFree(GetProcessHeap(), 0, (void*)pShader);		
		if (FAILED(hr))	
		{
			FDebug("Failed to create shader %s\n", strFilePath.c_str() );
			return E_FAIL;
		}

		// Add shader description to the array, set its index and return the index
		m_Shaders.push_back( Desc );
		* outIndex = (SM_SHADER_INDEX) m_Shaders.size() - 1;
		m_ShaderIndices.push_back( *outIndex );
	}
	catch(...)
	{
		FDebug("Error opening file %s\n", strFilePath.c_str() );
		return E_FAIL;
	}

	FMsg("LOADED Shader %d name [%s]\n", *outIndex, m_Shaders.at(*outIndex).m_ShaderObjectFile.c_str() );
	return S_OK;
}

// Assemble shader using DX runtime
HRESULT ShaderManager::LoadAndAssembleShader( ShaderDescription * pDesc )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDesc );
	FreeShader( pDesc );
    LPD3DXBUFFER pbufShader, pbufErrors;

	//flags:
	//	D3DXSHADER_DEBUG
	//	D3DXSHADER_SKIPVALIDATION
	//	D3DXSHADER_SKIPOPTIMIZATION
	hr = D3DXAssembleShaderFromFile( pDesc->m_ShaderDefinitionFile.c_str(),
										NULL,			// D3DXMACRO defines
										NULL,			// LPD3DXINcLUDE include, NULL = do #include
										0,				// flags
										&pbufShader,	
										&pbufErrors );
	if( FAILED( hr ))
	{
		FMsg("ShaderManager::LoadAndAssembleShader( ShaderDescription * pDesc ) Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
	    SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, pDesc->m_Type, pDesc );
	if( FAILED(hr) )
	{
		FMsg("LoadAndAssembleShader( ShaderDescription * pDesc ) couldn't create shader : %s\n", pDesc->m_ShaderDefinitionFile.c_str() );
	}

	return( hr );	
}

HRESULT ShaderManager::LoadAndAssembleShader( const TCHAR * file_name,
												SM_SHADERTYPE ShaderType,
												SM_SHADER_INDEX * outIndex,
												bool bVerbose )


{
	// assemble shader using DX runtime
	HRESULT hr = S_OK;
	MSG_BREAK_AND_RET_VAL_IF( m_pD3DDev==NULL, TEXT("ShaderManager::LoadAndAssembleShader(..) ShaderManager not initialized!\n"), E_FAIL );
	tstring strFilePath;
	strFilePath = GetFilePath( file_name, bVerbose );

/*
//@@@@@  this is mixing up the shaders
	// Check to see if shader has already been loaded & created
	SM_SHADER_INDEX index;
	if( IsShaderLoaded( strFilePath, & index ))
	{
		FDebug("Shader [%s] already assembled and has index %u\n", strFilePath.c_str(), index );	
		*outIndex = index;
		return( S_OK );
	}
//*/

	ShaderDescription	Desc;
    LPD3DXBUFFER pbufShader, pbufErrors;

	//flags:
	//	D3DXSHADER_DEBUG
	//	D3DXSHADER_SKIPVALIDATION
	//	D3DXSHADER_SKIPOPTIMIZATION
	hr = D3DXAssembleShaderFromFile( strFilePath.c_str(),
										NULL,			// D3DXMACRO defines
										NULL,			// LPD3DXINcLUDE include, NULL = do #include
										0,				// flags
										&pbufShader,	
										&pbufErrors );
	if( FAILED( hr ))
	{
		FMsg("ShaderManager::LoadAndAssembleShader Errors:\n");
		if( pbufErrors != NULL )
		{
			HandleErrorString( (TCHAR*) pbufErrors->GetBufferPointer() );
		}
		else
		{
			FMsg("Error buffer is NULL!\n");
		}
	    SAFE_RELEASE( pbufShader );
		BREAK_AND_RET_VAL_IF_FAILED( hr );
	}

	hr = CreateShader( pbufShader, ShaderType, & Desc );

	Desc.m_ShaderDefinitionFile = strFilePath.c_str();
	Desc.m_ShaderObjectFile		= TEXT("");
	Desc.m_Type					= ShaderType;

	m_Shaders.push_back( Desc );
	* outIndex = (SM_SHADER_INDEX) m_Shaders.size() - 1;
	m_ShaderIndices.push_back( *outIndex );
	return( hr );
}


HRESULT ShaderManager::SetShader( SM_SHADER_INDEX index )
{
	// sets device's vertex or pixel shader
	HRESULT hr = S_OK;
	MSG_BREAK_AND_RET_VAL_IF( m_pD3DDev==NULL, TEXT("ShaderManager::SetShader() ShaderManager not initialized!\n"), E_FAIL );

	if( index < m_Shaders.size() )
	{
		SM_SHADERTYPE ShaderType;
		ShaderType	= m_Shaders.at(index).m_Type;

		switch( ShaderType )
		{
		case SM_SHADERTYPE_VERTEX :
			m_pD3DDev->SetVertexShader( (IDirect3DVertexShader9*) (m_Shaders.at(index).m_pShader) );
			break;
		case SM_SHADERTYPE_PIXEL :
			m_pD3DDev->SetPixelShader( (IDirect3DPixelShader9*) (m_Shaders.at(index).m_pShader) );
			break;
		default:
			assert( false );
			hr = E_FAIL;
			break;
		}
	}
	else
	{
		FMsg("ShaderManager Index out of bounds! %u\n", index );
		assert( false );
		hr = E_FAIL;
	}
	return( hr );
}



HRESULT ShaderManager::HandleErrorString( const tstring & strError )
{
	HRESULT hr = S_OK;
	tstring::size_type i;
	i = strError.find( TEXT("error") );
	tstring err;
	int column_width = 90;

	if( i > 0 && i < strError.size() )
	{
		err = strError.substr( i, strError.size() );
		i = err.find( TEXT(" "), column_width );
		while( i > 0 && i < err.size() )
		{
			tstring spacer = TEXT("\n     ");
			err.insert( i, spacer );
			i = err.find( TEXT(" "), i + 2 + spacer.size() + column_width );
		}
	}
	else
	{
		err = strError;
	}
	FMsg("[\n%s]\n", err.c_str() );
	return( hr );
}


bool ShaderManager::IsShaderLoaded( const tstring & strFilePath, SM_SHADER_INDEX * out_index )
{
	// Checks if shader has already been loaded and created from the source file
	//  and returns the index of the created shader in out_index if it has.
	bool is_loaded = false;
	tstring filename;
	unsigned int i;

	for( i=0; i < m_Shaders.size(); i++ )
	{
		filename = m_Shaders.at(i).m_ShaderObjectFile;
		if( filename == strFilePath )
		{
			is_loaded = true;

			// find the index in m_ShaderIndices
			unsigned int ind;
			for( ind=0; ind < m_ShaderIndices.size(); ind++ )
			{
				if( m_ShaderIndices.at(ind) == i )
				{
					*out_index = ind;
					return( is_loaded );
				}
			}
			// if not found, keep searching
			is_loaded = false;
		}
	}
	return( false );
}

// Reload all shaders from disk
HRESULT ShaderManager::ReloadAllShaders( bool bVerbose )
{
	HRESULT hr = S_OK;	
	size_t i;
	for( i=0; i < m_Shaders.size(); i++ )
	{
		ReloadShader( i, bVerbose );
	}
	return( hr );
}

HRESULT ShaderManager::ReloadShader( size_t index, bool bVerbose )
{
	HRESULT hr = S_OK;
	if( index >= m_Shaders.size() )
		return( E_FAIL );
	if( index == 0 )
		return( S_OK );

	size_t	defname_length, objname_length;
	defname_length = m_Shaders.at(index).m_ShaderDefinitionFile.length();
	objname_length = m_Shaders.at(index).m_ShaderObjectFile.length();

	if( bVerbose )
		FMsg("defname_length = %u   objname_length = %u\n", defname_length, objname_length );

	if( defname_length > 0 && objname_length > 0 )
	{
		if( bVerbose )
			FMsg("Shader %u has both a text source name and binary object name.  Can't tell whether to compile or create from binary!\n", index );
		assert( false );
		return( E_FAIL );
	}

	if( defname_length > 0 )
	{
		hr = LoadAndAssembleShader( &(m_Shaders[index]) );
		if( bVerbose )
			FMsg("Loaded shader %s : %s\n", SUCCEEDED(hr) ? "SUCCEEDED" : "FAILED   ", m_Shaders[index].m_ShaderDefinitionFile.c_str() );
	}
	else if( objname_length > 0 )
	{
		hr = LoadAndCreateShader( &(m_Shaders[index]) );
		if( bVerbose )
			FMsg("Loaded shader %s : %s\n", SUCCEEDED(hr) ? "SUCCEEDED" : "FAILED   ", m_Shaders[index].m_ShaderObjectFile.c_str() );
	}

	return( hr );
}

vector< DWORD > ShaderManager::GetShaderBytes( SM_SHADER_INDEX index )
{
	vector< DWORD > vBytes;
	HRESULT hr = S_OK;
	UINT uSize;
	if( index < m_Shaders.size() )
	{
		SM_SHADERTYPE ShaderType;
		ShaderType	= m_Shaders.at(index).m_Type;
		switch( ShaderType )
		{
		case SM_SHADERTYPE_VERTEX :
			hr = ((IDirect3DVertexShader9*)(m_Shaders.at(index).m_pShader))->GetFunction( NULL, &uSize );
			vBytes.resize( uSize / 4, 0 );
			hr = ((IDirect3DVertexShader9*)(m_Shaders.at(index).m_pShader))->GetFunction( &(vBytes.at(0)), &uSize );
			if( FAILED(hr) )
			{
				FMsg(TEXT("ShaderManager::GetShaderBytes vshader couldn't get bytes!\n"));
				vBytes.clear();				
				return( vBytes );
			}
			break;
		case SM_SHADERTYPE_PIXEL :
			hr = ((IDirect3DPixelShader9*)(m_Shaders.at(index).m_pShader))->GetFunction( NULL, &uSize );
			vBytes.resize( uSize / 4, 0 );
			hr = ((IDirect3DPixelShader9*)(m_Shaders.at(index).m_pShader))->GetFunction( &(vBytes.at(0)), &uSize );
			if( FAILED(hr) )
			{
				FMsg(TEXT("ShaderManager::GetShaderBytes pshader couldn't get bytes!\n"));
				vBytes.clear();				
				return( vBytes );
			}
			break;
		default:
			assert( false );
			hr = E_FAIL;
			break;
		}
	}
	else
	{
		FMsg("ShaderManager GetShaderBytes index out of bounds! %u\n", index );
		hr = E_FAIL;
	}
	return( vBytes );
}

tstring ShaderManager::GetShaderText( SM_SHADER_INDEX index )
{
	HRESULT hr = S_OK;
	if( index >= m_Shaders.size() )
		return( TEXT(""));
	SM_SHADERTYPE ShaderType;
	ShaderType	= m_Shaders.at(index).m_Type;
	tstring tstr = TEXT("");
	switch( ShaderType )
	{
	case SM_SHADERTYPE_VERTEX :
		tstr = tstrShader( ((IDirect3DVertexShader9*)(m_Shaders.at(index).m_pShader)) );
		break;
	case SM_SHADERTYPE_PIXEL :
		tstr = tstrShader( ((IDirect3DPixelShader9*)(m_Shaders.at(index).m_pShader)) );
		break;
	default:
		tstr = TEXT("ShaderManager::GetShaderText() unknown shader type");
		assert( false );
		break;
	}
	return( tstr );
}

// Return a text string formatted with the hex values of the shader opcode bytes
tstring ShaderManager::GetShaderBytesText( SM_SHADER_INDEX shader )
{
	HRESULT hr = S_OK;
	vector< DWORD > vDWORD;
	vDWORD = GetShaderBytes( shader );
	MSG_AND_RET_VAL_IF( vDWORD.size() == 0, TEXT("GetShaderBytesText - no bytes!\n"), TEXT("") );
	tstring tstr = tstrBytes( (BYTE*)&(vDWORD.at(0)), vDWORD.size() * 4 );
	return( tstr );
}


