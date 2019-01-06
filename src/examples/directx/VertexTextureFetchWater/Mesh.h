#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

class Mesh
{
public:
  Mesh(void);
  ~Mesh(void);

  D3DXMATRIX& getWorldMatrix(void);
  void        initialize(const char* filename, LPDIRECT3DDEVICE9 d3dDevice);
  void        release(void);
  void        render(bool disableStateSetup = false);
  void        setWorldMatrix(D3DXMATRIX& worldMatrix);
  
protected:
  LPDIRECT3DDEVICE9         _d3dDevice;
  LPD3DXMESH                _mesh;
  std::vector<D3DMATERIAL9> _materials;
  D3DXMATRIX                _worldMatrix;
};
