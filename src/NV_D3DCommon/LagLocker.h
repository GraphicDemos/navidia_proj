/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  LagLocker.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A utility class to prevent mouse input lag when rendering the mouse cursor with 3D rendering calls.
The LagLocker class uses various methods to eliminate lag between when mouse input is processed
and when the mouse cursor graphic is rendered.

For now, LagLocker uses only a method of copying and locking a hardware texture in order to
prevent the GPU from falling behind the user input.

This is how the class works:
For LagLockerMode = TEXTURE_LOCK, the class creeates 1 or more GPU render-target textures (RTTs), each 16x16 pixels in size.
It also creates textures in D3DPOOL_SYSTEMMEM which mirror the render-target textures.
Each frame, it updates one RTT using a DX8 Clear() or DX9 Colorfill() call.  N frames later, where N is from 0 to 2, it
copies the GPU RTT to the system memory texture, locks the system memory texture, and reads a DWORD from it.
This forces the GPU rendering to never fall behind the application by more than N frames.

For D3D9-class hardware, D3D's Event Querry mechanism should be used instead of the texture lock
method.  //@@ The Event Querry mechanism is not yet supported in this version of the code.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_LAGLOCKER_H
#define H_LAGLOCKER_H

#include "NV_D3DCommon\NV_D3DCommonDX9PCH.h"

struct TexSurfPair
{
	IDirect3DTexture9 *	m_pRTTTexture;
	IDirect3DSurface9 *	m_pRTTSurface;
	IDirect3DTexture9 *	m_pDynTexture;
	IDirect3DSurface9 *	m_pDynSurface;

	void SetToNull() 
	{
		m_pRTTTexture = NULL;
		m_pRTTSurface = NULL;
		m_pDynTexture = NULL;
		m_pDynSurface = NULL;
	};
};


class LagLocker
{
public:
	// LagLocker can operate using a few different methods
	// TEXTURE_LOCK	creates a series of render-target textures and locks them in order to force synchronization
	// EVENT_QUERY	uses DX9 event queries to force synchronization
	// NVCPL		uses the NV control pannel interface to limit the number of buffered frames
	// DISABLED		does nothing
	enum LagLockerMode
	{
		AUTO_SELECT,		// automaticaly select the best mode
		TEXTURE_LOCK,
		EVENT_QUERY,		// not yet supported
		NVCPL,				// not yet supported
		UNINITIALIZED
	};

public:

	HRESULT	Initialize( IDirect3DDevice9 * pDev, LagLockerMode mode, int max_number_of_frames_to_sync_after );
	HRESULT Free();
	HRESULT OnDeviceLost();

	HRESULT BeginFrame();		// You must call these between your BeginScene() EndScene() pairs 
	HRESULT EndFrame();			// This drives the LagLocker processing.

	void	Enable();			// turn LagLocker on
	void	Disable();			// turn LagLocker off
	bool	IsEnabled()			{ return( m_bEnabled ); };

	// sync_after_n_frames must be <= the max_number_of_frames_to_sync_after supplied to Initialize
	// 0 causes a lock after each frame rendered, allowing no frames to be buffered by the driver
	// 1 causes a lock one frame after each frame rendered, allowing up to 1 frame to be buffered by the driver
	HRESULT	SetNumFramesToSyncAfter( int sync_after_n_frames );
	int		GetSyncAfterNumber()	{ return(m_nLockInterval); };

	HRESULT			SetMode( LagLockerMode mode );
	LagLockerMode	GetMode();

	LagLocker()		{ SetAllNull();			};
	~LagLocker()	{ Free(); SetAllNull();	};
	void SetAllNull()
	{
		m_pD3DDev		= NULL;
		m_pTexSurfPairs = NULL;
		m_nNumSurfaces	= 0;
		m_Mode = UNINITIALIZED;
		m_bTextureModeSupported			= true;
		m_bEventQuerryModeSupported		= false;
		m_bNVCPLModeSupported			= false;
	}

protected:
	IDirect3DDevice9 *		m_pD3DDev;

	TexSurfPair	*	m_pTexSurfPairs;
	int				m_nNumSurfaces;		// number of m_pTexSurfPairs
	int				m_nLockInterval;	// force sync after m_nLockInterval frames have passed

	LagLockerMode	m_Mode;
	bool			m_bEnabled;				// LagLocker is on or off
	int				m_nInitLockInterval;	// lock interval supplied to Initialize(..)
	int				m_nSurfToUpdate;		// counter for which surface to modify
	int				m_nSurfToLock;			// counter for which surface to lock
	bool			m_bTextureModeSupported;
	bool			m_bEventQuerryModeSupported;
	bool			m_bNVCPLModeSupported;

	HRESULT CreateSurfaces( IDirect3DDevice9 * pDev, int num_surf );
	HRESULT FreeSurfaces();
	void ResetLockCount();
	void IncrementLockCount();		// increment the counters based on m_nLockInterval

	HRESULT FindSupportedModes( IDirect3DDevice9 * pDev );
	HRESULT BeginFrame_Texture();
	HRESULT EndFrame_Texture();

};

#endif


