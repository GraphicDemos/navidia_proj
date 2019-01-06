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
// File: SpaceShipManager.h
// Desc: This wraps up the physics simulation of the space ships.
//			it includes collision with the asteroids and motherships
//-----------------------------------------------------------------------------
#pragma once

#include "dxstdafx.h"
#include "SpaceShip.h"
#include "AsteroidManager.h"
#include "MothershipManager.h"

class SpaceShipManager
{
public:
	SpaceShipManager();
	void SetWorldExtents(D3DXVECTOR3 &wmin,D3DXVECTOR3 &wmax);
	void Init(int num);

	// Mega simple netwtonian physics
	void Update(float deltaTime,AsteroidManager *asteroidManager, MothershipManager *mothershipManager);
	D3DXMATRIX MakeMatrixFor(int iShip);
	D3DXCOLOR GetColorFor(int iShip);

	D3DXVECTOR3 AsteroidCollisionCheck(int iShip,AsteroidManager *asteroidManager);
	D3DXVECTOR3 MothershipCollisionCheck(int iShip,MothershipManager *mothershipManager);

	D3DXQUATERNION VectorToQuat(D3DXVECTOR3 v);
	D3DXMATRIX VectorToMatrix(D3DXVECTOR3 v);
	D3DXVECTOR3 QuatToVector(D3DXQUATERNION q);
	D3DXVECTOR3 *SafeNormalize(D3DXVECTOR3 *dest,D3DXVECTOR3 *src);

	float accel;
	D3DXVECTOR3 worldMin;
	D3DXVECTOR3 worldMax;
	SpaceShip *ships;
	int numShips;
	D3DXVECTOR3 lastAveragePosition;
};