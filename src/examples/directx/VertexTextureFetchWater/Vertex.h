#ifndef VERTEX_H
#define VERTEX_H

#include <d3dx9.h>

struct Vertex
{
  static const DWORD FVF  = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE4(1));

  D3DXVECTOR3 position;       // vertex position
  D3DXVECTOR3 normal;         // vertex normal
  D3DCOLOR    diffuse;        // Diffuse RGBA color
  float       tex0[2];        // Base texture coordinates  
  float       tex1[4];        // Secondary texture coordinates
};

#endif
