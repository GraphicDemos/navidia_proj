
#include "Mesh.h"
#include "Registers.h"
#include "Vertex.h"

#include <assert.h>

extern WCHAR* toUnicode(const char* string);

Mesh::Mesh(void)
{
  _d3dDevice = NULL;
  _mesh = NULL;
  D3DXMatrixIdentity(&_worldMatrix);
 
  D3DXMatrixScaling(&_worldMatrix, 0.1f, 0.1f, 0.1f);

  _worldMatrix = D3DXMATRIX(0.1f, 0.0f, 0.0f, 0.0f,
                            0.0f, 0.1f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.1f, 0.0f,
                            20.0f, 0.5f, 20.0f, 1.0f);
}

Mesh::~Mesh(void)
{
  release();
}

D3DXMATRIX& Mesh::getWorldMatrix(void)
{
  return(_worldMatrix);
}

void Mesh::initialize(const char* filename, LPDIRECT3DDEVICE9 d3dDevice)
{
  assert(filename != NULL);
  assert(d3dDevice != NULL);

  HRESULT       hr;
  DWORD         numMaterials;
  int           i;
  LPD3DXBUFFER  materialBuffer;
  LPD3DXMESH    mesh;
  D3DXMATERIAL* materials;

  // If we already have mesh stuff, release it
  release();

  // Store the D3D device
  _d3dDevice = d3dDevice;

  // Load the mesh into system memory
  hr = D3DXLoadMeshFromX(toUnicode(filename), D3DXMESH_SYSTEMMEM, d3dDevice, NULL, &materialBuffer, NULL, &numMaterials, &mesh);
  assert(hr == D3D_OK);

  // Create a clone of the mesh using our FVF
  hr = mesh->CloneMeshFVF(D3DXMESH_MANAGED, Vertex::FVF, d3dDevice, &_mesh);
  assert(hr == D3D_OK);

  // Throw away the system memory mesh
  if(mesh != NULL)
    {
      mesh->Release();
      mesh = NULL;
    }
  
  // Extract the materials
  materials = (D3DXMATERIAL*)materialBuffer->GetBufferPointer();

  for(i=0; i<(int)numMaterials; i++)
    {
      D3DMATERIAL9 material;
      
      material = materials[i].MatD3D;
      material.Ambient = material.Diffuse;

      _materials.push_back(material);
    }

  // Release the material buffer
  materialBuffer->Release();
}

void Mesh::release(void)
{
  if(_mesh != NULL)
    {
      _mesh->Release();
    }
  _mesh = NULL;
}

void Mesh::render(bool disableStateSetup)
{
  assert(_d3dDevice != NULL);
  assert(_mesh != NULL);
  
  int   i;
  
  // Render each subset of the mesh
  for(i=0; i<(int)_materials.size(); i++)
    {
      if(disableStateSetup)
        {
          _d3dDevice->SetMaterial(&_materials[i]);
        }

      _d3dDevice->SetVertexShaderConstantF(VS_MATERIAL_DIFFUSE, (float*)&_materials[i].Diffuse, 1);
      _d3dDevice->SetVertexShaderConstantF(VS_MATERIAL_AMBIENT, (float*)&_materials[i].Ambient, 1);

      _mesh->DrawSubset(i);
    }
}

void Mesh::setWorldMatrix(D3DXMATRIX& worldMatrix)
{
  _worldMatrix = worldMatrix;
}
