

#include "NV_D3DCommonDX9PCH.h"
#include "NV_D3DCommon\CommonFactory.h"
#include "NV_D3DCommon\TextureDisplay2.h"
#include "shared\GetFilePath.h"

#ifndef DELETE_VECTOR_OF_HANDLES
#define DELETE_VECTOR_OF_HANDLES( vec, type )		\
{													\
	size_t i;										\
	type ** handle;									\
	type *  ptr;									\
	for( i=0; i < vec.size(); i++ )					\
	{												\
		handle = vec.at(i);							\
		if( handle != NULL )						\
		{											\
			ptr = *handle;							\
			SAFE_DELETE( ptr );						\
		}											\
		SAFE_DELETE( handle );						\
	}												\
}
#endif

#ifndef COMMONFACTORY_CREATE
#define COMMONFACTORY_CREATE( vec, type )			\
	type *	ptr = NULL;								\
	type ** handle = NULL;							\
	handle = new (type*);							\
	if( handle == NULL )							\
		return( NULL );								\
	*handle = new type;								\
	ptr = *handle;									\
	if( *handle == NULL )							\
		return( NULL );								\
	vec.push_back( handle );
#endif

CommonFactory::CommonFactory()
{
}

CommonFactory::~CommonFactory()
{
	Free();
}

HRESULT CommonFactory::Free()
{
	HRESULT hr = S_OK;
	DELETE_VECTOR_OF_HANDLES( m_vTF, TextureFactory );
	DELETE_VECTOR_OF_HANDLES( m_vRTF, RenderTargetFactory );
	DELETE_VECTOR_OF_HANDLES( m_vSM, ShaderManager );
	DELETE_VECTOR_OF_HANDLES( m_vTD, TextureDisplay2 );
	DELETE_VECTOR_OF_HANDLES( m_vSF, D3DDeviceStateFactory );
	return( hr );
}

TextureFactory **		CommonFactory::CreateTextureFactory( IDirect3DDevice9 * pDev )
{
	COMMONFACTORY_CREATE( m_vTF, TextureFactory );
	ptr->Initialize( GetFilePath::GetFilePath );
	return( handle );
}

RenderTargetFactory **	CommonFactory::CreateRenderTargetFactory( IDirect3DDevice9 * pDev )
{
	COMMONFACTORY_CREATE( m_vRTF, RenderTargetFactory );
	ptr->Initialize( GetFilePath::GetFilePath );
	return( handle );
}

ShaderManager **		CommonFactory::CreateShaderManager( IDirect3DDevice9 * pDev )
{
	COMMONFACTORY_CREATE( m_vSM, ShaderManager );
	ptr->Initialize( pDev, GetFilePath::GetFilePath );
	return( handle );
}

ITextureDisplay **		CommonFactory::CreateTextureDisplay( IDirect3DDevice9 * pDev )
{
	COMMONFACTORY_CREATE( m_vTD, TextureDisplay2 );
	ptr->Initialize( pDev );
	return( (ITextureDisplay**)handle );
}

D3DDeviceStateFactory ** CommonFactory::CreateStateFactory( IDirect3DDevice9 * pDev )
{
	COMMONFACTORY_CREATE( m_vSF, D3DDeviceStateFactory );
	return( handle );
}
