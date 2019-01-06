/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DCommon\
File:  NV_D3DCommonTypes.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/


#ifndef H_NVD3DCOMMON_TYPES_H
#define H_NVD3DCOMMON_TYPES_H

#include <windows.h>
class FRECT
{
public:
	float left;
	float right;
	float top;
	float bottom;
	FRECT()	{};
	FRECT( float _left, float _top, float _right, float _bottom )
	{
		left = _left; right = _right; top = _top, bottom = _bottom;		
	};
};

typedef DWORD		SM_SHADER_INDEX;
#define SM_IDXUNSET	0xFFFFFFFF
typedef DWORD		TD_TEXID;
typedef DWORD		PLOT_ID;

class CommonFactory;

class Gaussian;
class ConvolutionKernelElement1D;
class ConvolutionKernelElement2D;
class ConvolutionKernelElement3D;
class ConvolutionKernel;
class ConvolutionKernel1D;
class ConvolutionKernel2D;
class ConvolutionKernel3D;
class ConvolutionKernelFactory;

class DXDiagNVUtil;
class D3DDeviceAndHWInfo;
class LagLocker;
class LoadXFile;
class LowPassFilter2D;
class MatrixV;
class MatrixNode;
class MatrixNodeNamed;
class MouseUI;
struct GridNoiseComponent;
class NoiseGrid3D;
class PID_Controller;
class Plot;
class IPlot;
class RenderTargetFactory;
class RenderTargetSet;
class RenderTargetDesc;
class ShaderManager;
class ITextureDisplay;
class TextureDisplay2;
class TextureFactory;
class TextureFilenamePair;
class TrackRenderTargetTextures;

class D3DGeometryStateBundle;
class D3DStateBundle;
class D3DDeviceState;
class D3DDeviceStateFactory;

#endif
