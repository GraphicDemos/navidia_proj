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
//-----------------------------------------------------------------------------
#include "dxstdafx.h"
#include "AsteroidManager.h"

AsteroidManager::AsteroidManager()
{
	rocks = NULL;
	numRocks = 0;
}

void AsteroidManager::SetWorldExtents(D3DXVECTOR3 &wmin,D3DXVECTOR3 &wmax)
{
	worldMin = wmin;
	worldMax = wmax;
}

void AsteroidManager::Init(int num)
{
	Asteroid *newrocks;
	if(num > 0)
	{
		newrocks = new Asteroid[num];
	}

	if(rocks != NULL)
	{
		// preserve existing rocks
		if(num > 0 && numRocks > 0)
		{
			if(num > numRocks)
			{
				int numToCopy = numRocks;
				memcpy(newrocks,rocks,numToCopy*sizeof(Asteroid));
				for(int iRock=numRocks;iRock<num;iRock++)
				{
					newrocks[iRock].MakeRandomPosition(worldMin,worldMax);
				}
			}
			else
			{
				int numToCopy = num;
				memcpy(newrocks,rocks,numToCopy*sizeof(Asteroid));
			}
		}

		delete[]rocks;
	}
	else
	{
		for(int iRock=0;iRock<num;iRock++)
		{
			newrocks[iRock].MakeRandomPosition(worldMin,worldMax);
		}
	}
	rocks = newrocks;
	numRocks = num;
}

void AsteroidManager::Update(float deltaTime)
{
	for(int iRock=0;iRock<numRocks;iRock++)
	{
		D3DXQUATERNION rotAdjust;
		D3DXQuaternionRotationAxis(&rotAdjust,&rocks[iRock].rotationAxis,deltaTime*rocks[iRock].speed);
		D3DXQuaternionMultiply(&rocks[iRock].rotation,&rocks[iRock].rotation,&rotAdjust);
	}
}

D3DXMATRIX AsteroidManager::MakeMatrixFor(int iRock)
{
	D3DXMATRIX scaling; D3DXMatrixScaling(&scaling,rocks[iRock].scale.x,rocks[iRock].scale.y,rocks[iRock].scale.z);
	D3DXMATRIX rotation; D3DXMatrixRotationQuaternion(&rotation,&rocks[iRock].rotation);
	D3DXMATRIX translation; D3DXMatrixTranslation(&translation,rocks[iRock].position.x,rocks[iRock].position.y,rocks[iRock].position.z);
	D3DXMATRIX world = scaling*rotation*translation;
	return world;
}

D3DXCOLOR AsteroidManager::GetColorFor(int iRock)
{
	return rocks[iRock].color;
}
