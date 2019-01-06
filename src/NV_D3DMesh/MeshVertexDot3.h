/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVertexDot3.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A vertex with tangent space basis vectors

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_MESHVERTEXDOT3_H
#define H_MESHVERTEXDOT3_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

#define DOT3_VERTEX_DECL_SIZE	8

class MeshVertexDot3
{
public:
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DCOLOR	Diffuse;
	D3DXVECTOR2 Texture;
	D3DXVECTOR3 S;
	D3DXVECTOR3 T;
    D3DXVECTOR3 SxT;

	MeshVertexDot3()
	{};
	MeshVertexDot3( const D3DXVECTOR3& _Position, const D3DXVECTOR3& _Normal, const D3DXVECTOR2& _Texture )
		: Position(_Position), Normal(_Normal), Texture(_Texture)
	{};
};

class MeshVertexDot3Decl
{
public:
	D3DVERTEXELEMENT9 * GetVShaderDeclaration() { return( m_pDecl ); };
private:
	D3DVERTEXELEMENT9 * m_pDecl;
public:

	MeshVertexDot3Decl()
	{
		m_pDecl = NULL;
		m_pDecl = new D3DVERTEXELEMENT9[ DOT3_VERTEX_DECL_SIZE ];

		if( m_pDecl != NULL )
		{
			// assumes D3DDECL_END takes 1 D3DVERTEXELEMENT9
			D3DVERTEXELEMENT9 decl[] =
			{
				{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
				{ 0, 12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
				{ 0, 24,	D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },	// used to be 3 floats
				{ 0, 28,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },	// texture coordinate
				{ 0, 36,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
				{ 0, 48,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,	0 },
				{ 0, 60,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	1 },	// SxT
				D3DDECL_END()
			};

			int i;
			for( i=0; i < DOT3_VERTEX_DECL_SIZE; i++ )
			{
				m_pDecl[i] = decl[i];
			}
		}
	}			// MeshVertexDecl()

	virtual ~MeshVertexDot3Decl()
	{
		if( m_pDecl != NULL )
			delete [] m_pDecl;
		m_pDecl = NULL;		
	}
};

#endif 

