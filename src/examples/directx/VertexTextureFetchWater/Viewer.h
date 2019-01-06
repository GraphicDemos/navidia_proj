#ifndef VIEWER_H
#define VIEWER_H

#include <d3dx9.h>

class Viewer
{
public:
  Viewer(void);

  D3DXMATRIX& computeViewMatrix(D3DXMATRIX& viewMatrix);
  void        moveForward(float distance);
  void        moveRight(float distance);
  void        rotateLeftRight(float angle);
  void        rotateUpDown(float angle);
  

  // Accessor methods
  D3DXVECTOR3 getPosition(void) const;
  void        set(const D3DXVECTOR3& position, float yaw, float pitch);
  void        setHeight(float y);
  void        setPosition(const D3DXVECTOR3& position);

protected:
  D3DXVECTOR3 _position;
  float       _yaw;
  float       _pitch;
};

#endif
