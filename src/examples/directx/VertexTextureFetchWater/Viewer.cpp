
#include "Viewer.h"

#include <assert.h>
#include <d3dx9.h>
#include <math.h>

extern int dprintf(const char* formatString, ...);


Viewer::Viewer(void)
{
  //TODO remove 1.8f and replace with height
  set(D3DXVECTOR3(0.0f, 1.8f, 0.0f), 0.0f, 0.0f);
}

D3DXMATRIX& Viewer::computeViewMatrix(D3DXMATRIX& viewMatrix)
{
  D3DXMATRIX orientationMatrix;
  D3DXMATRIX pitchMatrix;
  D3DXMATRIX translationMatrix;
  D3DXMATRIX backupMatrix;

  D3DXMatrixRotationY(&orientationMatrix, -_yaw);
  D3DXMatrixRotationAxis(&pitchMatrix, &D3DXVECTOR3(orientationMatrix._11, orientationMatrix._21, orientationMatrix._31), _pitch);
  orientationMatrix = pitchMatrix * orientationMatrix;

  D3DXMatrixTranslation(&translationMatrix, -_position.x, -_position.y, -_position.z);
  viewMatrix = translationMatrix * orientationMatrix;

  return(viewMatrix);
}

void Viewer::moveForward(float distance)
{
  D3DXVECTOR3 forward;

  forward = D3DXVECTOR3(-sin(_yaw), 0.0f, -cos(_yaw));
  forward *= distance;
  
  _position += forward;
}

void Viewer::moveRight(float distance)
{
  D3DXVECTOR3 right;

  right = D3DXVECTOR3(cos(_yaw), 0.0f, -sin(_yaw));
  right *= distance;
  
  _position += right;
}

void Viewer::rotateLeftRight(float angle)
{
  _yaw = (float)fmod(_yaw + angle, 2.0f * D3DX_PI);
}

void Viewer::rotateUpDown(float angle)
{
  _pitch = (float)fmod(_pitch + angle, 2.0f * D3DX_PI);
  if(_pitch < -1.2f)
    {
      _pitch = -1.2f;
    }
  else if(_pitch > 1.2f)
    {
      _pitch = 1.2f;
    }
}

D3DXVECTOR3 Viewer::getPosition(void) const
{
  return(_position);
}

void Viewer::set(const D3DXVECTOR3& position, float yaw, float pitch)
{
  _position = position;
  _yaw = yaw;
  _pitch = pitch;
}

void Viewer::setHeight(float y)
{
  _position.y = y;
}

void Viewer::setPosition(const D3DXVECTOR3& position)
{
  _position = position;
}
