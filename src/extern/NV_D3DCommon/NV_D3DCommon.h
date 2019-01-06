/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DCommon\
File:  NV_D3DCommon.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
This is a library header for either NV_D3DCommonDX9 or NV_D3DCommonDX8.
Use it if you are selecting the D3D version by a project preprocessor #define setting, by providing
your own NVD3DVers.h file local to your project.
If you #include this after #including the D3D header files, it will select the appropriate version
based on the D3D header file you've included.

-------------------------------------------------------------------------------|--------------------*/


#ifndef H_NVD3DCOMMON_FORD3D9_H
#define H_NVD3DCOMMON_FORD3D9_H

#include <d3d9.h>
#include <d3dx9.h>

#include "NV_D3DCommon\NV_D3DCommonTypes.h"

// Inlcude headers for the library files
// These are under libs\src\NV_D3DCommon
#include "NV_D3DCommon\CommonFactory.h"
#include "NV_D3DCommon\ConvolutionKernelFactory.h"
#include "NV_D3DCommon\Counter.h"
#include "NV_D3DCommon\D3DDeviceAndHWInfo.h"
#include "NV_D3DCommon\DxDiagNVUtil.h"
#include "NV_D3DCommon\LagLocker.h"
#include "NV_D3DCommon\LoadXFile.h"
#include "NV_D3DCommon\LowPassFilter2D.h"
#include "NV_D3DCommon\MatrixNode.h"
#include "NV_D3DCommon\MinMaxAvg.h"
#include "NV_D3DCommon\MouseUI.h"
#include "NV_D3DCommon\PIDController.h"
#include "NV_D3DCommon\Plot.h"
#include "NV_D3DCommon\RenderTargetFactory.h"
#include "NV_D3DCommon\RenderToTextureBase.h"
#include "NV_D3DCommon\ShaderManager.h"
#include "NV_D3DCommon\TextureDisplay2.h"
#include "NV_D3DCommon\TextureFactory.h"
#include "NV_D3DCommon\TimeInfo.h"
#include "NV_D3DCommon\TGroup.h"
#include "NV_D3DCommon\TrackRenderTargetTextures.h"

#include "NV_D3DCommon\D3DStateBundle.h"
#include "NV_D3DCommon\D3DDeviceStates.h"
#include "NV_D3DCommon\D3DGeometryStateBundle.h"
#include "NV_D3DCommon\D3DDeviceStateFactory.h"

#include <shared/NV_StringFuncs.h>
#include <shared/NoiseGrid3D.h>
#include "NV_D3DCommon\NV_D3DCommonLibSelector.h"

#endif		// H_NVD3DCOMMON_FORD3D9_H

