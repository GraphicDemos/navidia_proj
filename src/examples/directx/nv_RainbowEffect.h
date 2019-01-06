#ifndef NV_RAINBOW_EFFECT_H
#define NV_RAINBOW_EFFECT_H

class nv_D3D9FullScreenQuad;

/**
	Rainbow Effect helper class
	encapsulates the moisture texture render target and rainbow techniques

*/
class nv_RainbowEffect
{

private:
	nv_D3D9FullScreenQuad*	m_pFullScreenQuad;	//quad for rendering rainbow effect in screen space
	LPD3DXEFFECT			m_pRainbowEffect; //the fx file for the rianbow
	D3DXHANDLE              m_hTechniqueRenderRainbowQuad; // Handle to technique in the effect 
	LPDIRECT3DTEXTURE9		m_pRainbowLookupTextureScattering; // the lookup texture for the rainbow
	LPDIRECT3DTEXTURE9		m_pCoronaLookupTexture;// the lookup texture for the corona

	//render target and texture for render to moisture texture
	LPDIRECT3DSURFACE9		m_pRenderTargetDepthBuffer;
	LPDIRECT3DSURFACE9		m_pRenderTarget;
	LPDIRECT3DTEXTURE9		m_RenderTargetTexture_Moisture;
	LPDIRECT3DSURFACE9		m_pRenderTargetTextureSurface;//pointer to surface of texture for copy

	//place holders for begin/end moisture pass
	LPDIRECT3DSURFACE9		m_ptheRealBackBuffer; 
	LPDIRECT3DSURFACE9		m_ptheRealDepthBuffer;


public:
	nv_RainbowEffect();
	~nv_RainbowEffect();

	//standard d3d object interface functions
	HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT InvalidateDeviceObjects();
	HRESULT Create( LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT Destroy();


	/**
		sets up appropriate render target for rendering moisture to
	*/
	HRESULT BeginMoistureTextureRendering(LPDIRECT3DDEVICE9 pd3dDevice);
	
	/**
		restores render target and updates moisture texture
		for use in RenderRainbow
	*/
	HRESULT EndMoistureTextureRendering(LPDIRECT3DDEVICE9 pd3dDevice);
	
	/**
		renders the rainbow effect 
	*/
	HRESULT RenderRainbow( LPDIRECT3DDEVICE9 pd3dDevice);

	/**
		set the sun's light direction in world space
		does not have to be normalized
	*/
	void SetLightDirection( D3DXVECTOR4* lightDir);

	/**
		set the droplet size of the moisture that is making the rainbow
		smaller radii droplets will cause a fogbow, larger ones will cause
		a rainbow. range: [0,1]
	*/
	void SetDropletRadius( FLOAT radius);

	/**
		set the rainbow intensity
	*/
	void SetRainbowIntensity( FLOAT intensity);
	
	/**
		set the current view matrix
	*/
	void SetViewMatrix(D3DXMATRIX* view);
	
	/**
		set the inverse projection matrix
	*/
	void SetProjInvMatrix(D3DXMATRIX* projInv);

	/**
		get access to the moisture texture,
		this is provided for visualization purposes.
	*/
	LPDIRECT3DTEXTURE9		GetMoistureTexture(){ return m_RenderTargetTexture_Moisture;}

};
#endif NV_RAINBOW_EFFECT_H