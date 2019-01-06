//-----------------------------------------------------------------------------
// Copyright NVIDIA Corporation 2004
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED 
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS 
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL 
// NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR 
// CONSEQUENTIAL DAMAGES WHATSOEVER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR 
// LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS 
// INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR 
// INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGES.
// 
// File: nv_D3D9FullScreenQuad.h
// Desc: a helper class for rendering full-screen efects
//-----------------------------------------------------------------------------
#pragma once
#include "d3d9.h"
#include "d3dx9.h"
#include "assert.h"

class nv_D3D9FullScreenQuad
{
	private:
		struct SCREEN_VERTEX 
		{
			D3DXVECTOR3 pos;
			D3DXVECTOR3 tex1;
		};
		DWORD  D3DFVF_SCREEN_VERTEX;

		SCREEN_VERTEX m_QuadVertices[4];

		LPDIRECT3DVERTEXBUFFER9 m_pVB;

		//	creates the fullscreenquad with texture coordinates set up
		//	for mapping texels to pixels on a given render target.
		//
		//	renderTargetSurface			- the target surface that you will be rendering this
		//									quad to
		void SetUpverts(  LPDIRECT3DSURFACE9 renderTargetSurface)
		{
			D3DSURFACE_DESC desc;
			renderTargetSurface->GetDesc(&desc);
			
			FLOAT right = (FLOAT)desc.Width;//-1.5f;
			FLOAT top = (FLOAT)desc.Height;// +0.5f;
			FLOAT bottom = 0;//0.5f;
			FLOAT left = 0;//0.5f;
			FLOAT depth = 1.0f;
		   
			// Fill in the vertex values
			m_QuadVertices[0].pos = D3DXVECTOR3(right, bottom, depth);
			m_QuadVertices[0].tex1 = D3DXVECTOR3(1.0f, 0.0f, 1.0);
			
			m_QuadVertices[1].pos = D3DXVECTOR3(right, top, depth);
			m_QuadVertices[1].tex1 = D3DXVECTOR3(1.0f, 1.0, 1.0);
			
			m_QuadVertices[2].pos = D3DXVECTOR3(left, bottom, depth);
			m_QuadVertices[2].tex1 = D3DXVECTOR3(0.0f, 0.0f, 1.0);
			
			m_QuadVertices[3].pos = D3DXVECTOR3(left, top, depth);
			m_QuadVertices[3].tex1 = D3DXVECTOR3(0.0f, 1.0, 1.0);

			for( int i = 0 ; i < 4 ; ++i)
			{
				m_QuadVertices[i].pos.x /= right;
				m_QuadVertices[i].pos.y /= top;
				m_QuadVertices[i].pos.y = 1.0f - m_QuadVertices[i].pos.y;
				m_QuadVertices[i].pos.x -= 0.5f;
				m_QuadVertices[i].pos.y -= 0.5f;
				m_QuadVertices[i].pos.x *= 2.0f;
				m_QuadVertices[i].pos.y *= 2.0f;
			}
			
		}

		
	public:
		nv_D3D9FullScreenQuad()
		{
			D3DFVF_SCREEN_VERTEX =  (D3DFVF_XYZ|D3DFVF_TEX1);
			m_pVB = NULL;
		};
		// Rendering
		HRESULT Render( LPDIRECT3DDEVICE9 pd3dDevice)
		{
			HRESULT hr = pd3dDevice->SetFVF( D3DFVF_SCREEN_VERTEX );
			if(FAILED(hr))
				return hr;

			hr = pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(SCREEN_VERTEX) );
			if(FAILED(hr))
				return hr;

			hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			return hr;
		};
		// Initializing
		//	pd3dDevice					- the d3d device
		//	renderTargetSurface			- the target surface that you will be rendering this
		//									quad to
		HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice)
		{
			if(m_pVB)
				m_pVB->Release();
			m_pVB = NULL;

			HRESULT hr = pd3dDevice->CreateVertexBuffer(	4*sizeof(SCREEN_VERTEX), 
															D3DUSAGE_WRITEONLY,
															D3DFVF_SCREEN_VERTEX,
															D3DPOOL_MANAGED,
															&m_pVB,
															NULL);
				
			return hr;
		};

		HRESULT SetUpForRenderTargetSurface( LPDIRECT3DSURFACE9 renderTargetSurface)
		{

			assert(m_pVB && "you must have called RestoreDeviceObjects before calling this");

			//resetup the verts to handle changes in dimensions
			SetUpverts( renderTargetSurface);


			SCREEN_VERTEX* pVerts;
			HRESULT hr = m_pVB->Lock( 0 , 0 , (void**)(&pVerts) , 0 );
			if(FAILED(hr))
				return hr;
			
			for(unsigned int i = 0 ; i < 4 ; ++i)
			{
				pVerts[i] = m_QuadVertices[i];
			}

			hr = m_pVB->Unlock();
				
			return hr;
		};

		HRESULT InvalidateDeviceObjects()
		{
			if(m_pVB)
				m_pVB->Release();
			m_pVB = NULL;
			return S_OK;
			
		};
		
		virtual ~nv_D3D9FullScreenQuad(){};
};

