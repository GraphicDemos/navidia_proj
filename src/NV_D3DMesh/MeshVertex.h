/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVertex.h

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

#ifndef H_D3DMESHVERTEX_H
#define H_D3DMESHVERTEX_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

#define MESHVERTEX_FVF  ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL |	\
						  D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)  )	
							
#define MESHVDECLSIZE		6


class MeshVertex
{
public:
	D3DXVECTOR3		pos;			// position
	D3DXVECTOR3		nrm;			// vertex normal
    D3DCOLOR		diffuse;		// ARGB DWORD
    D3DXVECTOR2		t0;				// texture coord 0    


	void Interpolate( MeshVertex * pOut, const MeshVertex * pInBase, 
						const MeshVertex * pInTo, float factor )
	{
		RET_IF( pOut == NULL );
		RET_IF( pInBase == NULL );
		RET_IF( pInTo == NULL );
		D3DXVec3Lerp( & pOut->pos, & pInBase->pos, & pInTo->pos, factor );
		D3DXVec3Lerp( & pOut->nrm, & pInBase->nrm, & pInTo->nrm, factor );
		D3DXVec2Lerp( & pOut->t0,  & pInBase->t0,  & pInTo->t0,  factor );	
		D3DXCOLOR c1, c2, result;
		c1 = pInBase->diffuse;
		c2 = pInBase->diffuse;
		D3DXColorLerp( & result, & c1, & c2, factor );
		pOut->diffuse = result;
	}
	// compares pIn to this vertex.  Diff in position must be less than fPositionThresh in 
	//  each .x .y and .z axis.  If the DP3 of the vertex normals is greater than fNormalDP3Thresh,
	//  then the vertices are considered to be within the tolerance.
	bool VertexWithinTolerances( MeshVertex * pIn, float fPositionThresh,
									float fNormalDP3Thresh, float fTexcoordThresh )
	{
		D3DXVECTOR3 dpos = pIn->GetPosition() - GetPosition();
		D3DXVECTOR2 dt0  = pIn->GetTexcoord0() - GetTexcoord0();
		float normdp3    = D3DXVec3Dot( pIn->GetNormalP(), GetNormalP() );
		if( fabs(dpos.x) > fPositionThresh )
			return( false );
		if( fabs(dpos.y) > fPositionThresh )
			return( false );
		if( fabs(dpos.z) > fPositionThresh )
			return( false );
		if( fabs(dt0.x) > fTexcoordThresh )
			return( false );
		if( fabs(dt0.y) > fTexcoordThresh )
			return( false );
		if( normdp3 <= fNormalDP3Thresh )
			return( false );
		return( true );
	}
	void SetPosition( const D3DXVECTOR3 & position )
	{
		pos = position;
	}
	D3DXVECTOR3 GetPosition()
	{
		return( pos );		
	}
	D3DXVECTOR3 * GetPositionP()
	{
		return( &pos );
	}
	void SetNormal( const D3DXVECTOR3 & normal )
	{
		nrm = normal;
	}
	D3DXVECTOR3 GetNormal()
	{
		return( nrm );
	}
	D3DXVECTOR3 * GetNormalP()
	{
		return( &nrm );
	}
	void SetColor( const D3DCOLOR & color )
	{
		diffuse = color;
	}
	D3DCOLOR GetColor()
	{
		return( diffuse );
	}
	D3DCOLOR * GetColorP()
	{
		return( &diffuse );
	}
	void SetTexcoord0( const D3DXVECTOR2 & texcoord0 )
	{
		t0 = texcoord0;
	}
	D3DXVECTOR2 GetTexcoord0()
	{
		return( t0 );
	}
	D3DXVECTOR2 * GetTexcoord0P()
	{
		return( &t0 );
	}
	DWORD GetFVF()
	{
		return( MESHVERTEX_FVF );
	}

	MeshVertex() :
		pos( 0.0f, 0.0f, 0.0f ),
		nrm( 0.0f, 0.0f, 0.0f ),
		diffuse( 0x00FFFFFF ),
		t0( 0.0f, 0.0f )
	{
	}
	MeshVertex( const D3DXVECTOR3 & position, const D3DXVECTOR3 & normal,
				const D3DCOLOR & color, const D3DXVECTOR2 & texcoord0 ) :
		pos( position ),
		nrm( normal ),
		diffuse( color ),
		t0( texcoord0 )
	{
	}
	MeshVertex( const D3DXVECTOR3 & position ) :
		pos( position ),
		nrm( 0.0f, 0.0f, 0.0f ),
		diffuse( 0x00FFFFFF ),
		t0( 0.0f, 0.0f )
	{
	}
	MeshVertex( const D3DXVECTOR3 & position, const D3DXVECTOR2 & texcoord0 ) :
		pos( position ),
		nrm( 0.0f, 0.0f, 0.0f ),
		diffuse( 0x00FFFFFF ),
		t0( texcoord0.x, texcoord0.y )
	{
	}
};

class MeshVertexDecl
{
public:
	D3DVERTEXELEMENT9 * GetVShaderDeclaration() { return( m_pDecl ); };
	DWORD				GetFVF()  { return( MESHVERTEX_FVF ); };

private:
	D3DVERTEXELEMENT9 * m_pDecl;

public:
	MeshVertexDecl()
	{
		m_pDecl = NULL;
		m_pDecl = new D3DVERTEXELEMENT9[ MESHVDECLSIZE ];

		if( m_pDecl != NULL )
		{
			// assumes D3DDECL_END takes 1 D3DVERTEXELEMENT9
			D3DVERTEXELEMENT9 decl[] =
			{
				{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
				{ 0, 12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
				{ 0, 24,	D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
				{ 0, 28,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
				D3DDECL_END()
			};
			int i;
			for( i=0; i < MESHVDECLSIZE; i++ )
			{
				m_pDecl[i] = decl[i];
			}
		}
	}			// MeshVertexDecl()

	virtual ~MeshVertexDecl()
	{
		if( m_pDecl != NULL )
			delete [] m_pDecl;
		m_pDecl = NULL;		
	}
};


#endif 

