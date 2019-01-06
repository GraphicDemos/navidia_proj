/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombScene.cpp

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

#include "dxstdafx.h"
#include <NV_D3DCommon/NV_D3DCommonDX9.h>		// include library sub-headers and add .lib to linker inputs
#include <NV_D3DMesh/NV_D3DMesh.h>

#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

#include "FogTombScene.h"
#include "ThicknessRenderProperties.h"
#include "ThicknessRenderProperties8BPC.h"

// Defines are used to make it very clear what assets this module attempts to load
#define FILENAME_TO_LOAD		TEXT("MEDIA/models/Tomb/tomb5c.x")
#define FNAME_FOG_COLOR_RAMP	TEXT("MEDIA\\textures\\2D\\Gradients\\VolumeFogRamp_03tomb.tga")

// Tesselation amount for the fog volume object.  Higher number generates more triangles
#define FOG_BOX_TESSLEATION		45

// Value to designate an unset state in part of the procedural fog volume object creation
#define CORNER_TEST_UNSET		0xFFFFFFFF


//-----------------------------------------------------------------------------
// Name: FogTombScene()
// Desc: Application constructor.   Paired with ~FogTombScene()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
FogTombScene::FogTombScene()
{
	SetAllNull();
}

//-----------------------------------------------------------------------------
// Name: ~FogTombScene()
// Desc: Application destructor.  Paired with FogTombScene()
//-----------------------------------------------------------------------------
FogTombScene::~FogTombScene()
{
	FreeFogMesh();
	SAFE_RELEASE( m_pd3dDevice );
	SAFE_DELETE( m_pLoadXFile );
	FREE_GUARANTEED_ALLOC( m_ppTextureFactory, m_pTextureFactory );
	SetAllNull();
}

//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT FogTombScene::OneTimeSceneInit()
{
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Paired with DeleteDeviceObjects()
//       The device has been created.  Resources that are not lost on
//       Reset() can be created here -- resources in D3DPOOL_MANAGED,
//       D3DPOOL_SCRATCH, or D3DPOOL_SYSTEMMEM.  Image surfaces created via
//       CreateOffScreenPlainSurface are never lost and can be created here.  Vertex
//       shaders and pixel shaders can also be created here as they are not
//       lost on Reset().
//-----------------------------------------------------------------------------
HRESULT FogTombScene::InitDeviceObjects( IDirect3DDevice9 * pDev, TextureFactory ** ppTextureFactory )
{
    HRESULT hr = S_OK;

	FAIL_IF_NULL( pDev );
	m_pd3dDevice = pDev;
	m_pd3dDevice->AddRef();

	FREE_GUARANTEED_ALLOC( m_ppTextureFactory, m_pTextureFactory );
	GUARANTEE_ALLOCATED( ppTextureFactory, m_ppTextureFactory, m_pTextureFactory, TextureFactory, Initialize( GetFilePath::GetFilePath ) );
	MSG_BREAK_AND_RET_VAL_IF( m_ppTextureFactory == NULL, "no texture factory!\n", E_FAIL );

	IDirect3DSurface9 * 	pSurf;
	m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurf );
	FAIL_IF_NULL( pSurf );
	D3DSURFACE_DESC desc;
	pSurf->GetDesc( &desc );
	SAFE_RELEASE( pSurf );

    // Setup the camera
    float fAspectRatio = 	desc.Width / (float) desc.Height;
	float fNearClip = DEFAULT_NEARCLIP;
	float fFarClip =  DEFAULT_FARCLIP;
    m_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, fNearClip, fFarClip );

	if( m_pProperties != NULL )
	{
		m_pProperties->SetClipPlanes( fNearClip, fFarClip );
	}

	// Set values good for the tomb.x scene
	D3DXVECTOR3 vecEye, vecAt;
	vecEye = D3DXVECTOR3( -0.728632f, 0.972811f, -1.678328f );
	vecAt = vecEye + D3DXVECTOR3( 0.360102f, -0.164252f, 0.918340f );
	
	m_Camera.SetViewParams( &vecEye, &vecAt );
    m_Camera.SetScalers( 0.005f, 3.0f );

	return(hr);
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT FogTombScene::RestoreDeviceObjects()
{
	HRESULT hr = S_OK;

	m_Camera.SetResetCursorAfterMove( false );		// windowed

	SAFE_DELETE( m_pLoadXFile );
	m_pLoadXFile = new LoadXFile;
	FAIL_IF_NULL( m_pLoadXFile );

	m_pLoadXFile->Initialize( m_pd3dDevice, GetFilePath::GetFilePath );

	hr = m_pLoadXFile->LoadFile( FILENAME_TO_LOAD, false );	// 2nd param true for verbose reporting
	MSG_AND_BREAK_IF( FAILED(hr), TEXT("Couldn't load ") FILENAME_TO_LOAD );

	m_pLoadXFile->ListMeshInfo();

	// Generate a matrix to scale and translate the mesh to 0,0,0 and a size in .x of 2.0f
	hr = m_pLoadXFile->SetMatrixToXFormMesh( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
											 D3DXVECTOR3( 2.0f, 2.0f, 2.0f ),
											 LoadXFile::KEEP_ASPECT_RATIO );

	// Load a texture that maps fog thickness to onscreen color
	if( m_ppTextureFactory != NULL )
	{
		TextureFactory * pTF = *m_ppTextureFactory;
		if( pTF != NULL )
		{
			m_ppFogColorRamp = pTF->CreateTextureFromFile( m_pd3dDevice, FNAME_FOG_COLOR_RAMP );
			MSG_BREAK_AND_RET_VAL_IF( m_ppFogColorRamp == NULL, "Couldn't load fog color ramp texture!\n", E_FAIL );
		}
	}

	// Create a closed-hull box object tesselated on top that will form the fog on the floor of the scene
	CreateFogMesh2();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT FogTombScene::FrameMove( float fElapsedTime, bool bAnimateFogVolume )
{
    // Update the camera's postion based on user input 
    m_Camera.FrameMove( fElapsedTime );

	if( bAnimateFogVolume )
	{
		AnimateFogMesh( fElapsedTime );
	}

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT FogTombScene::Render()
{

	// Get the projection & view matrix from the camera class
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION,	m_Camera.GetProjMatrix() );
	m_pd3dDevice->SetTransform( D3DTS_VIEW,			m_Camera.GetViewMatrix() );

	if( m_pLoadXFile != NULL )
		m_pd3dDevice->SetTransform( D3DTS_WORLD, & m_pLoadXFile->m_matWorld );

	if( m_pLoadXFile != NULL )
	{
		m_pLoadXFile->Render();
	}

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT FogTombScene::MsgProc(HWND hWnd, UINT msg, WPARAM wParam,
                                   LPARAM lParam)
{
    m_Camera.HandleMessages( hWnd, msg, wParam, lParam );

    switch(msg)
    {
		case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case 'F':
				if( m_pLoadXFile != NULL )
				{
					bool wireframe = m_pLoadXFile->ToggleWireframe();
					FMsg("wireframe: %s\n", wireframe ? "true" : "false" );
				}
			case 'C':
				// Output camera eye point and look direction.  The 'look-at' point is the
				//  eye location plus the look direction.
                D3DXVECTOR3 eye = (D3DXVECTOR3)*m_Camera.GetEyePt();
				D3DXVECTOR3 lookdir = (D3DXVECTOR3)*m_Camera.GetWorldAhead();
				FMsg("eye     = %f %f %f\n", eye.x, eye.y, eye.z );
				FMsg("lookdir = %f %f %f\n", lookdir.x, lookdir.y, lookdir.z );
				break;
			}
		}
    }

	return( 0L );
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT FogTombScene::InvalidateDeviceObjects()
{
	SAFE_DELETE( m_pLoadXFile );

	FMsg("FogTombScene Inval dev obj %x\n", m_ppTextureFactory );
	if( m_ppTextureFactory != NULL )
	{
		if( *m_ppTextureFactory != NULL )
		{
			(*m_ppTextureFactory)->Free();
		}
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Paired with InitDeviceObjects()
//       Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.  
//-----------------------------------------------------------------------------
HRESULT FogTombScene::DeleteDeviceObjects()
{
	if( m_ppTextureFactory != NULL )
	{
		if( *m_ppTextureFactory != NULL )
		{
			(*m_ppTextureFactory)->Free();
		}
	}
	FREE_GUARANTEED_ALLOC( m_ppTextureFactory, m_pTextureFactory );
    return S_OK;
}


HRESULT FogTombScene::AnimateFogMesh( float fTimeStepInSeconds )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pFogMeshVB );
	FAIL_IF_NULL( m_pFogMesh );
	FAIL_IF_NULL( m_pFogMeshAnimated );

	// Displace the fog mesh vertices by a 3D noise function.  The GridNoiseComponent determines
	//  the spatial frequency, amplitude, random number seed, and filtering type for the 3D noise.
	UINT uNumNoiseComponents = 1;
	GridNoiseComponent pNoiseComponents[1];
	pNoiseComponents[0] = GridNoiseComponent( 6.6f, 0.2f, 1, GridNoiseComponent::GNFT_TRILINEAR );

	static D3DXVECTOR3 trans( 0.0f, 0.0f, 0.0f );
	float fTrans;
	fTrans = fTimeStepInSeconds * 0.04f;
	// translate mostly in the up/down direction, which is .y for this scene
	trans = trans + D3DXVECTOR3( fTrans * 0.3f, fTrans, fTrans * 0.2f );

	MeshProcessor mp;
	mp.AddPositionNoise1D( m_pFogMeshAnimated, m_pFogMesh, D3DXVECTOR3( 0.0f, 1.0f, 0.0f ),
							trans,
							pNoiseComponents, uNumNoiseComponents );

	m_pFogMeshVB->UpdateVerticesFromMesh( m_pFogMeshAnimated );

	return( hr );
}

HRESULT FogTombScene::FreeFogMesh()
{
	SAFE_DELETE( m_pFogMesh );
	SAFE_DELETE( m_pFogMeshAnimated );
	SAFE_DELETE( m_pFogMeshVB );
	return( S_OK );
}


HRESULT FogTombScene::FindBoxCorners( Mesh * pMesh, UINT * tc00, UINT * tc10, UINT * tc01, UINT * tc11,
										UINT * bc00, UINT * bc10, UINT * bc01, UINT * bc11 )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( tc00 );	// top corner verts (.y max)
	FAIL_IF_NULL( tc10 );
	FAIL_IF_NULL( tc01 );
	FAIL_IF_NULL( tc11 );
	FAIL_IF_NULL( bc00 );	// bottom corner verts (.y min)
	FAIL_IF_NULL( bc10 );
	FAIL_IF_NULL( bc01 );
	FAIL_IF_NULL( bc11 );

	MeshProcessor mp;
	float minx, miny, minz, maxx, maxy, maxz;
	mp.FindPositionMinMax( pMesh, &minx, &miny, &minz, &maxx, &maxy, &maxz );
	D3DXVECTOR3 pos;
	*tc00 = *tc10 = *tc01 = *tc11 = *bc00 = *bc10 = *bc01 = *bc11 = CORNER_TEST_UNSET;

	// Find the box corner vertices
	UINT i;
	float thresh = 0.0001f;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pos = pMesh->GetVertexPosition( i );
		if( fabs( pos.x - minx ) <= thresh )
		{
			if( fabs(pos.z - minz) <= thresh )
			{
				if( fabs(pos.y - miny) <= thresh )
				{
					*bc00 = i;
				}
				else if( fabs(pos.y - maxy) <= thresh  )
				{
					*tc00 = i;
				}
				else
				{
//					FMsg("diff = %f\n", fabs( pos.y - maxy ) );
				}
			}
			else if( fabs( pos.z - maxz ) <= thresh )
			{
				if( fabs( pos.y - miny ) <= thresh )
				{
					*bc01 = i;
				}
				else if( fabs( pos.y - maxy ) <= thresh )
				{
					*tc01 = i;
				}
			}
		}
		else if( fabs( pos.x - maxx ) <= thresh )
		{
			if( fabs( pos.z - minz ) <= thresh )
			{
				if( fabs( pos.y - miny ) <= thresh )
				{
					*bc10 = i;
				}
				else if( fabs( pos.y - maxy ) <= thresh )
				{
					*tc10 = i;
				}
			}
			else if( fabs( pos.z - maxz ) <= thresh )
			{
				if( fabs( pos.y - miny ) <= thresh )
				{
					*bc11 = i;
				}
				else if( fabs( pos.y - maxy ) <= thresh )
				{
					*tc11 = i;
				}
			}
		}
	}

//	FMsg("%u %u %u %u   %u %u %u %u \n", bc00, bc10, bc01, bc11, tc00, tc10, tc01, tc11 );

	if( *tc00 == CORNER_TEST_UNSET )
		assert( false );
	if( *tc01 == CORNER_TEST_UNSET )
		assert( false );
	if( *tc11 == CORNER_TEST_UNSET )
		assert( false );
	if( *tc10 == CORNER_TEST_UNSET )
		assert( false );

	if( *bc00 == CORNER_TEST_UNSET )
		assert( false );
	if( *bc01 == CORNER_TEST_UNSET )
		assert( false );
	if( *bc11 == CORNER_TEST_UNSET )
		assert( false );
	if( *bc10 == CORNER_TEST_UNSET )
		assert( false );

	return( hr );
}


// Box with full tesselation on top .y side, no tesselation on bottom .y side
HRESULT FogTombScene::CreateFogMesh2()
{
	HRESULT hr = S_OK;
	FreeFogMesh();

	m_pFogMesh = new Mesh;
	FAIL_IF_NULL( m_pFogMesh );
	MeshGeoCreator gc;

	gc.InitTesselatedBlockCS( m_pFogMesh, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 2.5f, 0.65f, 3.2f ),
								FOG_BOX_TESSLEATION, 0, FOG_BOX_TESSLEATION );

	UINT tc00, tc10, tc01, tc11, bc00, bc10, bc01, bc11;
	FindBoxCorners( m_pFogMesh, &tc00, &tc10, &tc01, &tc11, &bc00, &bc10, &bc01, &bc11 );

	Mesh * pMesh = m_pFogMesh;
	MeshProcessor mp;

	float minx, miny, minz, maxx, maxy, maxz;
	mp.FindPositionMinMax( pMesh, &minx, &miny, &minz, &maxx, &maxy, &maxz );

	MeshBeingProcessed mbp;
	mbp.InitFromMesh( pMesh );

	float thresh = 0.0001f;

	// Reduce the tesselation by collapsing to the box corner vertices
	UINT i;
	D3DXVECTOR3 pos;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pos = pMesh->GetVertexPosition( i );

		if( fabs( pos.x - minx ) <= thresh )	// if lowest x coord
		{
			if( fabs( pos.y - miny ) <= thresh )	// if lowest y coord (on the bottom)
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( bc00, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( bc01, i );	// becomes maxz point bc01				
			}
			else if( fabs( pos.y - maxy ) <= thresh )  // highest .y coord (on top)
			{
				// do nothing
			}
		}
		else if( fabs( pos.x - maxx ) <= thresh )	// highest x coord
		{
			if( fabs( pos.y - miny ) <= thresh )	// lowest y coord
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( bc10, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( bc11, i );	// becomes maxz point bc01				
			}
			else if( fabs( pos.y - maxy ) <= thresh )	// highest y coord
			{
				// do nothing
			}
		}
		//--------
		if( fabs( pos.z - minz ) <= thresh )		// lowest z coord
		{
			if( fabs( pos.y - miny ) <= thresh )	// if on the bottom
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( bc00, i );
				else
					mbp.MergeVertices( bc10, i );
			}
			else if( fabs( pos.y - maxy ) <= thresh ) // highest y coord
			{
				// do nothing
			}
		}
		else if( fabs( pos.z - maxz ) <= thresh )	// highest .z coord
		{
			if( fabs( pos.y - miny ) <= thresh )	// lowest y coord
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( bc01, i );
				else
					mbp.MergeVertices( bc11, i );
			}
			else if( fabs( pos.y - maxy ) <= thresh ) // highest y coord
			{
				// do nothing
			}
		}
		//-----------
		// if vertex is on the bottom, collapse it to a corner
		if( fabs( pos.y - miny ) <= thresh )		// lowest y coord
		{
			if( pos.x < (maxx + minx)/2.0f )		// closest to one x point or other
			{
				if( pos.z < (maxz + minz)/2.0f )
					mbp.MergeVertices( bc00, i );
				else
					mbp.MergeVertices( bc01, i );
			}
			else
			{
				if( pos.z < (maxz + minz)/2.0f )
					mbp.MergeVertices( bc10, i );
				else
					mbp.MergeVertices( bc11, i );
			}
		}
	}

	mbp.GetProcessedMesh( pMesh );
	mp.RemoveUnusedVertices( pMesh );
	mp.RemoveDegenerateVerts( pMesh, 0.00001f, -1.1f, 1.1f );
	mp.RemoveDegenerateTriangles( pMesh );
	mp.RemoveUnusedVertices( pMesh );

	m_pFogMeshAnimated = new Mesh;
	FAIL_IF_NULL( m_pFogMeshAnimated );
	gc.InitClone( m_pFogMeshAnimated, m_pFogMesh );

	m_pFogMeshVB = new MeshVB;
	FAIL_IF_NULL( m_pFogMeshVB );
	hr = m_pFogMeshVB->CreateFromMesh( m_pFogMesh, m_pd3dDevice, MeshVB::DYNAMIC );

	return( hr );
}


// Box with full tesselation on top .y face except around the edges which have no tesselation
// The stitching tris get very skinny around the top .y face.
// The rest of the box is not tesselated
HRESULT FogTombScene::CreateFogMesh()
{
	HRESULT hr = S_OK;
	FreeFogMesh();

	m_pFogMesh = new Mesh;
	FAIL_IF_NULL( m_pFogMesh );
	MeshGeoCreator gc;

	gc.InitTesselatedBlockCS( m_pFogMesh, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 2.5f, 0.65f, 3.2f ),
								FOG_BOX_TESSLEATION, 0, FOG_BOX_TESSLEATION );

	UINT tc00, tc10, tc01, tc11, bc00, bc10, bc01, bc11;
	FindBoxCorners( m_pFogMesh, &tc00, &tc10, &tc01, &tc11, &bc00, &bc10, &bc01, &bc11 );

	Mesh * pMesh = m_pFogMesh;
	MeshProcessor mp;

	float minx, miny, minz, maxx, maxy, maxz;
	mp.FindPositionMinMax( pMesh, &minx, &miny, &minz, &maxx, &maxy, &maxz );

	float thresh = 0.0001f;

	MeshBeingProcessed mbp;
	mbp.InitFromMesh( pMesh );

	UINT i;
	D3DXVECTOR3 pos;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pos = pMesh->GetVertexPosition( i );

		if( fabs( pos.x - minx ) <= thresh )
		{
			if( fabs( pos.y - miny ) <= thresh )	// if on the bottom
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( bc00, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( bc01, i );	// becomes maxz point bc01				
			}
			else if( fabs( pos.y - maxy ) <= thresh )		// on top
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( tc00, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( tc01, i );	// becomes maxz point bc01				
			}
		}
		else if( fabs( pos.x - maxx ) <= thresh )
		{
			if( fabs( pos.y - miny ) <= thresh )	// if on the bottom
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( bc10, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( bc11, i );	// becomes maxz point bc01				
			}
			else if( fabs( pos.y - maxy ) <= thresh )		// on top
			{
				if( pos.z < (maxz + minz)/2.0f )	// closer to minz
					mbp.MergeVertices( tc10, i );	// becomes minz point bc00
				else
					mbp.MergeVertices( tc11, i );	// becomes maxz point bc01				
			}
		}
		//--------
		if( fabs( pos.z - minz ) <= thresh )
		{
			if( fabs( pos.y - miny ) <= thresh )	// if on the bottom
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( bc00, i );
				else
					mbp.MergeVertices( bc10, i );
			}
			else if( fabs( pos.y - maxy ) <= thresh )		// on top
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( tc00, i );
				else
					mbp.MergeVertices( tc10, i );
			}
		}
		else if( fabs( pos.z - maxz ) <= thresh )
		{
			if( fabs( pos.y - miny ) <= thresh )	// if on the bottom
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( bc01, i );
				else
					mbp.MergeVertices( bc11, i );
			}
			else if( fabs( pos.y - maxy ) <= thresh )		// on top
			{
				if( pos.x < (maxx + minx)/2.0f )
					mbp.MergeVertices( tc01, i );
				else
					mbp.MergeVertices( tc11, i );
			}
		}
		//-----------
		// if vertex is on the bottom, collapse it to a corner
		if( fabs( pos.y - miny ) <= thresh )
		{
			if( pos.x < (maxx + minx)/2.0f )
			{
				if( pos.z < (maxz + minz)/2.0f )
					mbp.MergeVertices( bc00, i );
				else
					mbp.MergeVertices( bc01, i );
			}
			else
			{
				if( pos.z < (maxz + minz)/2.0f )
					mbp.MergeVertices( bc10, i );
				else
					mbp.MergeVertices( bc11, i );
			}
		}
	}

	mbp.GetProcessedMesh( pMesh );
	mp.RemoveUnusedVertices( pMesh );
	mp.RemoveDegenerateVerts( pMesh, 0.00001f, -1.1f, 1.1f );
	mp.RemoveDegenerateTriangles( pMesh );
	mp.RemoveUnusedVertices( pMesh );

	m_pFogMeshAnimated = new Mesh;
	FAIL_IF_NULL( m_pFogMeshAnimated );
	gc.InitClone( m_pFogMeshAnimated, m_pFogMesh );

	m_pFogMeshVB = new MeshVB;
	FAIL_IF_NULL( m_pFogMeshVB );
	hr = m_pFogMeshVB->CreateFromMesh( m_pFogMesh, m_pd3dDevice, MeshVB::DYNAMIC );

	return( hr );
}

D3DXMATRIX * FogTombScene::CalculateViewProjMatrix()
{
	D3DXMATRIX *pView, *pProj;
	pView = (D3DXMATRIX*)m_Camera.GetViewMatrix();
	pProj = (D3DXMATRIX*)m_Camera.GetProjMatrix();
	D3DXMatrixMultiply( &m_matViewProj, pView, pProj );
	return( & m_matViewProj );
}

D3DXMATRIX * FogTombScene::ApplyWorldToViewProjMatrixAndTranspose( const D3DXMATRIX * pWorld )
{
	D3DXMATRIX * pViewProj = GetViewProjMatrix();
	D3DXMatrixMultiply( & m_matWorldViewProjTranspose, pWorld, pViewProj );
	D3DXMatrixTranspose( & m_matWorldViewProjTranspose, & m_matWorldViewProjTranspose );
	return( & m_matWorldViewProjTranspose );
}



