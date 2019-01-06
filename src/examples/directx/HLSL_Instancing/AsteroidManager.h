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
// File: AsteroidManager.h
// Desc: The asteroid manager manages a slew of little rotating asteroids.  It is also used by
//			the space ship manager to collide the space ships with
//-----------------------------------------------------------------------------
#pragma once

#include "dxstdafx.h"
#include "Asteroid.h"

class AsteroidManager
{
public:

	AsteroidManager();
	void SetWorldExtents(D3DXVECTOR3 &wmin,D3DXVECTOR3 &wmax);
	void Init(int num);
	void Update(float deltaTime);
	D3DXMATRIX MakeMatrixFor(int iRock);
	D3DXCOLOR GetColorFor(int iRock);

	D3DXVECTOR3 worldMin;
	D3DXVECTOR3 worldMax;
	Asteroid *rocks;
	int numRocks;
};
