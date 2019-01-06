/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  PA_Water.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
PA_Water is an optimization over the CA_Water class rendering for height-based water
simularion.  PA_Water uses fewer passes and fewer render target changes.  It does
this by holding the water position, force, and velocity all in one texture instead
of using a separate texture for each.

This means 
1)  The 'water' now has only a 1D position.  It is not allowed to ripple differently
    in each of the r,g,b, and a channels.
2)  Water height is now stored in blue AND alpha.  This makes it possible to 
	optimize some of the calculations

Several functions are provided for creating a variety of displacement maps
from the height simulation.  Currently these are:

1)	Normal maps:  x,y,z of normal biased & scaled to [0,1] range (use with _bx2)
	Typicaly used for reflection into a cube map

2)  Dot3x2 EMBM displacement maps:  r,g,b = ( du, dv, 1.0 )
	du,dv are biased & scaled to [0,1] range.
	Used with custom vertex shader which writes a 2x2 EMBM matrix to each vertex's
	texture coords.  This allows the matrix to vary per-vertex for better reflections
	than the standard DX6 style EMBM where the matrix is constant for all vertices.
	Can be used for a sky dome reflection technique where the sky dome is rendered
	into a 2D texture.  Vertex shader does a reflection calculation per-vertex, and
	this is perturbed by the displacement map.

3)  DX6-style EMBM displacement map
	Numbers are in twos-complement format which the texbem/texbeml pixel shader
	instructions require.
	

I am 50/50 on whether to separate out the displacement map creation from the 
water animation.  Putting the map creation in-line makes for fewer renderstate
checks & switches and simpler texture management.  Maybe one day I'll also create
a separate class for turning height maps into displacement maps.

  PA_Water was created thinking that no more than a few of them (1 or 2) would
be used in an app.  If you want to create many of them, it would be worth while
to separate out redundant parts and allocate texture surfaces globally, to be 
shared among all of them.  This can save texture storage for the intermediate
result textures.

The physics of the water simulation is controlled by several parameters.
See comments below for more information about them:
	m_fForceFactor	m_fVelFactor m_fBlurDist m_fNrmlSTScale	m_fPosAtten	m_fEqRestore_factor
	m_fWindY m_fWindX

Functions are provided to set some of them:
	SetDefaultParameters();
	SetDisplacementMapSTScale( float STScale );
	SetBlurDistance( float fac );
	SetEqRestoreFac( float fac );
	SetVelocityApplyFac( float fac );
	SetBlendFac( float fac );
	SetScrollAmount( float u, float v );

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_PA_WATER_ANIM_3MAPS_H
#define H_PA_WATER_ANIM_3MAPS_H

#include "dxstdafx.h"
#include <vector>
#include <string>

#include <NV_D3DCommon/NV_D3DCommonTypes.h>

class ShaderManager;
class ITextureDisplay;
class TextureDisplay2;

using namespace std;

typedef std::basic_string<TCHAR> tstring;
class PA_Water
{
public:
	enum DisplacementMapMode		// which type of displacement map to create
	{
		DM_NONE,
		DM_NORMAL_MAP,
		DM_DOT3X2_MAP,
		DM_EMBM_MAP
	};
	struct FourTextureStage_TexCoordOffsets
	{
		D3DXVECTOR4	t[4];		// offset for texture coordinates t0 to t3
	};
    typedef struct tagQuadVertex
    {
	    D3DXVECTOR3 Position;
        D3DXVECTOR2 Tex;
    } QuadVertex;

protected:
	IDirect3DDevice9 *			m_pD3DDev;

public:
	PA_Water();
	~PA_Water();
	void SetAllNull();

	//  To check device caps for rendering support
	virtual HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT Format );
	virtual HRESULT Free();
	virtual HRESULT Initialize( IDirect3DDevice9 * pDev, int res_x, int res_y,
								tstring droplet_filename,
								DisplacementMapMode eMapMode,
								ShaderManager ** ppShaderManager,
								ITextureDisplay ** ppITextureDisplay = NULL );
	virtual HRESULT Tick();		// call to update simulation by one time step

	// Other util functions you may want to replace
	HRESULT		LoadTexture( IDirect3DDevice9 * pD3DDev, tstring filename, 
							 IDirect3DTexture9 ** ppTex );

	// General utility functions, not central to the water animation
	// receives keyboard events
    virtual bool    Keyboard(DWORD dwKey, UINT nFlags, bool bDown);

	void		LoadTextureMaps();
	void		SetRenderResultToScreen( bool true_to_render );
	void		Diag_RenderResultToScreen();

protected:
	// Do one time step of the water height animation
	HRESULT		DoSingleTimeStep_Optimized();

	//  The following might be called during Tick();
	void		Do_CreateNormalMapSteps();		// Create normal map
	void		Do_CreateDot3x2EMBMMapSteps();	// Create biased-scaled map
												//	with rgb = (du,dv,1.0)
	void		Do_CreateEMBMMapSteps();		// Create twos-complement DX6-style 
												//	displacement map
	//-------------------------------------------------------------------------------
public:
	// To set a number of values controling the properties of the water sim and displacement maps.
	void		SetDefaultParameters();
	void		SetDisplacementMapSTScale( float STScale );
	void		SetBlurDistance( float fac );
	void		SetEqRestoreFac( float fac );
	void		SetVelocityApplyFac( float fac );
	void		SetBlendFac( float fac );
	void		SetScrollAmount( float u, float v );	// set amount to scroll the
														// prev texture when rendering
														// into the new.
	HRESULT		RenderTarget_SetToCurrentStateTexture();
	HRESULT		RenderTarget_Restore();				// set back to normal backbuffer

	// Functions for getting the height maps or displacement map
	//  for use in rendering a scene
	IDirect3DTexture9 *		GetOutputTexture();		// Get displacement map
	IDirect3DTexture9 *		GetStateTexture();		// Get height & velocity texture
	IDirect3DTexture9 *		GetPrevStateTexture();	// Get previous step h & v texture
	
	// Add droplet to be rendered in the next Tick()
	//   Coords range from (0,0) to (1,1) to cover the whole texture
	void		AddDroplet( float x, float y, float scale  );


	// to load shaders and avoid loading same shader twice
	ShaderManager **	m_ppShaderManager;

	// Utility class for displaying texture results and driving the rendering
	TextureDisplay2 *	m_pTextureDisplay;
	ITextureDisplay	**  m_ppITextureDisplay;
	TD_TEXID	m_TD_Fullscreen;
	TD_TEXID	m_TD_UpperRight;
	TD_TEXID	m_TD_UpperLeft;
	TD_TEXID	m_TD_LowerRight;
	TD_TEXID	m_TD_LowerLeft;
	TD_TEXID	m_TD_Droplet;

	// Size in texture coordinates of one texel of the animated
	//  maps.  m_fPerTexelWidth = 1.0f / texture_resolution_x
	// These are used to determine the vertex texture coordinate
	//  offsets which in turn establish neighbor sampling from
	//  the textures.
	float		m_fPerTexelWidth;
	float		m_fPerTexelHeight;
	int			m_nResX;			// texture resolution in X and Y
	int			m_nResY;
	D3DXVECTOR4	m_Const1;			// convenient vertex shader constant value
	D3DXMATRIX	m_mWorldViewProj;
	bool		m_bAnimate;			// animate the texture or not
	bool		m_bSingleStep;		// set to true and one step is taken, then this
									//  will set itself back to false.
    bool		m_bWireframe;		// only affect final rendering for diagnostics
									// not render to texture steps
	bool		m_bReset;			// Reset water simulation - clears textures
	bool		m_bDgbDrawOutputToScreen;
	bool		m_bApplyInteriorBoundaries;		// not supported in current version
	bool		m_bCreateNormalMap;	
	float		m_fDropletFreq;
	void		DisplayParameters();
	bool		m_bWrap;			// wrap or do not wrap at borders - makes textures
									//  that tile seamlessly or do not
	float		m_fDropletMinSize;	// droplets vary in size randomly
	float		m_fDropletMaxSize;

	// These offsets used to establish various patterns of
	//  neighbor sampling for running the simulation and 
	//  creating displacement maps from simulation height map.
	FourTextureStage_TexCoordOffsets	m_Offsets[5];	// 5 sets of offsets.  Each
														//  set is 4 4-float vectors

	void	 SetOffsets( int set );		// Set vshader constant with the set of 
										//  offsets specified.  Chances are you not
										//  call this directly.

	// Below are convenient named functions for 
	//  selecting the various offsets patterns.
	//  Each of these calls SetOffsets(int) for
	//  you with the appropriate index.
	void	 SetOffsets_ZeroOffsets();
	void	 SetOffsets_Step1();
	void	 SetOffsets_Step2();
	void	 SetOffsets_Blur();
	void	 SetOffsets_NearestNeighbors();

protected:
	HRESULT		CreateTextureRenderTargets( int width, int height );
	// create various offsets to sample texel neighbors in rendering each pixel
	void		CreateUVOffsets( int texture_width, int texture_height );

	// A set of offsets for sampling neighbors to blur (average) them all together.
	void		CreateUVBlurOffsets();
	void		SetInitialRenderStates();

	DWORD		m_dwZClearFlags;		// clear Z + stencil or just Z
	float		m_fScrollU;				// scroll amount used by water coupler or in
	float		m_fScrollV;				// general to scroll the surface each step.
	tstring		m_tstrDropletFilename;	// texture file for droplet texture

	// whether to make a normal map, EMBM map, no map, etc.
	DisplacementMapMode		m_eDisplacementMapMode;

private:
	// The number of render targets required 1 extra is created to hold a normal map
	// This is not needed if you just want to animate height.
	// Because each PA_Water creates its own unique textures, but each texture is not
	//  needed all of the time, if you need to create many PA_Water instances you may
	//  want to re-structure the texture allocation so that many PA_Water share the same
	//  pool of intermediate or temporary textures.
    enum 
    {
        kMaxNumTargets = 4										
    };
	enum	DiagnosticDisplayMode
	{
		FULLSCREEN_DISPLACEMENT_MAP,
		FULLSCREEN_STEP_1_CALC,
		ALL_TOGETHER,
		FULLSCREEN_FINALOUT
	};

private:
	void	DrawDroplets();		// Draws the array of m_Droplets which was filled with input before the Tick();
	void	DrawInteriorBoundaryObjects();	// draw fixed objects (boundary conditions interior to the texture

	IDirect3DTexture9	**	m_ppInteriorBoundariesTexture;
	IDirect3DTexture9	*	m_pDropletTexture;

	// Pixel and vertex shader handles
	SM_SHADER_INDEX		m_VSHI_TexCoordOffset;

	SM_SHADER_INDEX		m_PSHI_EqualWeight_PostMult;
	SM_SHADER_INDEX		m_PSHI_WaterAnimStep_1;
	SM_SHADER_INDEX		m_PSHI_WaterAnimStep_2;
	SM_SHADER_INDEX		m_PSHI_NormalMapCreate_Alpha;
	SM_SHADER_INDEX		m_PSHI_Dot3x2EMBMMapCreate_Alpha;
	SM_SHADER_INDEX		m_PSHI_CreateEMBM_A;

	// What textures do display if diagnostic rendering is on.
	DiagnosticDisplayMode	m_eDiagnosticDisplayMode;

	// Controls reset & flip-flop from texture source to texture destination
	int						m_nFlipState;

	// physical parameters & displacement map scale.
	// Feel free to make these public if you want convenient access
	float					m_fForceFactor;		// physical constant used in applying
												//  force to velocity
	float					m_fVelFactor;		// physical constant used in applying
												//  velocity to position
	float					m_fBlurDist;		// For blurring height after each step 
												//  to smooth the water surface.  This
												//  controls the distance at which
												//  neighboring texels are sampled.
												//  0.0 mean no blur, 1.0 means blur 
												//  from neighbors 1 texel size away.
	float					m_fNrmlSTScale;		// 0.0 to 1.0f scale used in creating
												//  displacement maps.  This can be
												//  used to reduce the magnitude of
												//  the displacement map vectors
	float					m_fPosAtten;		// Multiplies the position at each
												//  step.  0.9875 to slightly dampen
												//  positions
	float					m_fEqRestore_factor;// scalar [0-1] for magnitude of force
												//  pulling texels to an equilibrium
												//  value.  Without this, the height 
												//  values will eventualy fall to zero
												//  or hit saturation due to roundoff 
												//  errors and the low numerical
												//  precision of the simulation.

	float	m_fWindY;			// wind factor - make small 0 to 0.05f or so
								// Adds a scrolling amount at each step
	float	m_fWindX;			// You may want to try a few different methods
								//  for how the wind offsets are applied.

protected:
	std::vector < D3DXVECTOR3 > m_Droplets;		// Array of droplet locations for  
												//  accumulating until next rendering
												//  call. x,y = pos, z = scale		
	IDirect3DVertexBuffer9 *  m_pVertexBuffer;
	IDirect3DVertexDeclaration9 * m_pVertexDecl;
    IDirect3DSurface9       * m_pBackbufferColor;	// buffer the user can see
    IDirect3DSurface9       * m_pBackbufferDepth; 

	// These textures hold the height, velocity, force, and displacement bump maps of the simulation
	// surface[0] is texture[0]'s render target surface
    IDirect3DTexture9       * m_pRTTTexture[ kMaxNumTargets ];	// rendered textures       
    IDirect3DSurface9       * m_pRTTSurface[ kMaxNumTargets ]; 

	// Texture/surface pair indices and pointers to mark which
	//  textures are sources and destinations (render targets)
	//  at each step of the simulation.
	// Do not free or allocate to these - they are just copies
	//  of pointers elsewhere.

	IDirect3DTexture9 * m_pTex_HeightSrc;	// need pointer because at step
											//   zero there is an initial texture
											//   not part of the target/src array
	int				m_nTex_HeightTarg;		// indices for which textures to use
	int				m_nTex_HeightSrc;		//   as sources and render targets
	int				m_nTex_DisplTarg;		// displacement map target
	int				m_nForceStepOne;		// index of texture used to accumulate
											//  partial result in a first step of 
											//  the force calculation
};

#endif					// H_PA_WATER_ANIM_3MAPS_H
