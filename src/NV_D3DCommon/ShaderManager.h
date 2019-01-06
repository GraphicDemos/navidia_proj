/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  ShaderManager.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

ShaderManager - a class for loading various pixel & vertex shaders.  Takes
	care of not loading shaders twice if they are loaded multiple times.



******************************************************************************/


#ifndef H_NV_SHADERMANAGER_9_H
#define	H_NV_SHADERMANAGER_9_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "shared/GetFilePath.h"			// to search for shader files (media)

#include "NV_D3DCommon_decl_.h"
#pragma warning(disable: 4786)
#include <vector>
#pragma warning(disable: 4786)
#include <string>

using namespace std;

typedef enum tagSM_SHADERTYPE
{
	SM_SHADERTYPE_VERTEX = 0,
	SM_SHADERTYPE_PIXEL = 1,
	SM_SHADERTYPE_FORCEDWORD = 0xFFFFFFFF
} SM_SHADERTYPE;



class ShaderDescription
{
public:
	tstring		m_ShaderDefinitionFile;	// text file name (.nvv, .nvp file)
	tstring		m_ShaderObjectFile;		// compiled shader object file (.vso, .pso)
										//  as created by nvasm or other assembler

	void *			m_pShader;			// Pointer to shader object
	DWORD			m_dwDeclaration;
	SM_SHADERTYPE	m_Type;

	ShaderDescription()
	{
		m_pShader		= NULL;
		m_dwDeclaration = 0;
		m_Type			= SM_SHADERTYPE_FORCEDWORD;
	}
};

//---------------------------------------

class DECLSPEC_NV_D3D_COMMON_API ShaderManager
{
public:
	enum ShaderIndex
	{
		SM_INDEX_UNSET = SM_IDXUNSET
	};
private:
	//@@@ ShaderIndirection is not yet used.
	struct ShaderIndirection
	{
		DWORD				m_dwShaderIndex;	// index to m_Shaders
		SM_SHADER_INDEX *	m_pUserIndex;		// Pointer to user's SM_SHADER_INDEX
	};


	std::vector < ShaderDescription >	m_Shaders;			// m_Shaders[0] is always = 0
	std::vector < SM_SHADER_INDEX >		m_ShaderIndices;	// indirection to m_Shaders
											// Programs hold an index to this array for
											// their shaders.  Each element holds an index
											// into m_Shaders and the address of the SM_SHADER_INDEX
											// that the manager returned to the program, so the 
											// indices can be reordered by the manager.
	std::vector< ShaderIndirection >	m_ShaderIndirections;
	
	IDirect3DDevice9		* m_pD3DDev;

	// Function that searches for a file name and returns the full path to the file.
	GetFilePath::GetFilePathFunction		m_pGetFilePathFunctionSM;

	HRESULT CreateShader( LPD3DXBUFFER pbufShader,
							SM_SHADERTYPE ShaderType,
							ShaderDescription * pDesc );

	HRESULT	FreeShader( ShaderDescription * pDesc );
	HRESULT LoadAndAssembleShader( ShaderDescription * pDesc );		// does not add description to m_Shaders
	HRESULT LoadAndCreateShader( ShaderDescription * pDesc );		// does not add description to m_Shaders

public:

	ShaderManager();
	~ShaderManager();

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev, GetFilePath::GetFilePathFunction file_path_callback = NULL );

	// Set pointer to function which searches for full file path based on short file name
	void								SetFilePathCallback( GetFilePath::GetFilePathFunction pFilePathCallback );
	GetFilePath::GetFilePathFunction	GetGetFilePathFunction()	{ return( m_pGetFilePathFunctionSM ); };
	tstring								GetFilePath( const tstring & file_name, bool bVerbose = false );

	bool	IsShaderLoaded( const tstring & strFilePath, SM_SHADER_INDEX * out_index );

	HRESULT HandleErrorString( const tstring & strError );


	HRESULT LoadAndAssembleShader( const TCHAR * ptcharFilePath,
									SM_SHADERTYPE ShaderType,
									SM_SHADER_INDEX * outIndex,
									bool bVerbose = false );

	// Create shader from text buffer in memory
	// Text is shader assembly code
	HRESULT LoadAndCreateShaderFromMemory( const char * program_asm_code,
											const TCHAR * shader_name,
											SM_SHADERTYPE ShaderType,
											SM_SHADER_INDEX * outIndex );

	// Create shader from pre-compiled object file.
	// strSourceFile is not required and is just a convenient name to track
	HRESULT	LoadAndCreateShader(	const tstring & strFilePath,
									const tstring & strSourceFile,
									SM_SHADERTYPE ShaderType,
									SM_SHADER_INDEX * outIndex	);

	// Same as above but with no strSourceFile arg
	HRESULT	LoadAndCreateShader(	const tstring & strFilePath,
									SM_SHADERTYPE ShaderType,
									SM_SHADER_INDEX * outIndex	);

	HRESULT SetShader( SM_SHADER_INDEX index );		// sets device's vertex or pixel shader
	tstring	GetShaderObjName( SM_SHADER_INDEX index );
	HRESULT ReloadAllShaders( bool bVerbose = false );						// reloads all shaders from disk
	HRESULT ReloadShader( size_t index, bool bVerbose = false );

	void	ListAllShaders();
	vector< DWORD > GetShaderBytes( SM_SHADER_INDEX shader );
	tstring GetShaderText( SM_SHADER_INDEX shader );
	tstring GetShaderBytesText( SM_SHADER_INDEX shader );

};



#endif			// H_NV_SHADERMANAGER_9_H
