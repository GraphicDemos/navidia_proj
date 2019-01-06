/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/main.cpp#2 $

Copyright NVIDIA Corporation 2005
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Comments:
	Photoshop filter framework sample

	THIS SAMPLE REQUIRES THE PRESENC OF THE PHOTOSHOP CS API.
	THE PHOTOSHOP CS API IS AVAILABLE SEPARATELY FROM ADOBE AT ADOBE.COM.
	YOU WILL NEED TO ALTER YOUR LOCAL PROJECT TO NCLUDE FILES FROM
	WITHIN YOUR OWN LOCAL "PhotoshopCSAPI" FOLDER.

******************************************************************************/

#define REQUIRED_EXTENSIONS "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format " \
							              "GL_ARB_shader_objects " \
							              "GL_ARB_vertex_shader " \
							              "GL_ARB_fragment_shader " \
                            "GL_ARB_multitexture " \
                            "WGL_ARB_render_texture " \
                            "WGL_NV_render_texture_rectangle " \
                            "GL_NV_texture_rectangle " \
                            "GL_EXT_texture3D " \
                            "GL_NV_pixel_data_range "
//  Removed this restriction for running on nv3x hardware
//                             "GL_ARB_texture_non_power_of_two " \

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <shared/pbuffer.h>
#include <windows.h>
#include <PIFilter.h>
#include <shared/ErrorHandling.h>

#include "GLManager.h"
#include "GPUFilter.h"
#include "TestFilterData.h"

FilterRecord *gFilterRecord;
int32 *gData;
int16 *gResult;
SPBasicSuite *sSPBasic = NULL;

GPUFilterData *GPUFilterData::me = 0;

void DoAbout(void)
{
	// it would be good to issue a caution about 32-bit imaging on pre-nv4x parts here, if detected
	AboutRecord * aboutRecord = reinterpret_cast<AboutRecord *>(gFilterRecord);
	DoAbout(static_cast<SPPluginRef>(aboutRecord->plugInRef), AboutID);
}

void DoParameters(void)
{
}

void DoPrepare(void)
{
	gFilterRecord->bufferSpace = 0;
	gFilterRecord->maxSpace = 0;
}

void DoStart(void)
{
  // Here it is!! Everything important starts here.

  try
  {
    // Replace this with some installation directory read from the registry
	  // TO-DO -- let this directory be user-definable?? KB
    SetCurrentDirectory("C:\\GPUFilter"); // if this doesn't exist it will read from the PS dir - KB

    // Set up OpenGL
    GLManager theGL(GetDLLInstance((SPPluginRef)gFilterRecord->plugInRef));
	// determine if the image is 32-bit but the card cannot handle that, warn the user and halt.
	if (gFilterRecord->imageMode == plugInModeRGB96) {
		if(!glh_init_extensions("WGL_ATI_pixel_format_float"))
			throw "Sorry, you need a higher-rated GPU for 32-bit float images";
	}
#define EXT_DEBUG
#ifdef EXT_DEBUG  /* look at each extension in turn for compatability */
	if(!glh_init_extensions("WGL_ARB_pbuffer"))
      throw "Couldn't support WGL_ARB_pbuffer";
	if(!glh_init_extensions("WGL_ARB_pixel_format"))
      throw "Couldn't support WGL_ARB_pixel_format";
	if(!glh_init_extensions("GL_ARB_shader_objects"))
      throw "Couldn't support GL_ARB_shader_objects";
	if(!glh_init_extensions("GL_ARB_vertex_shader"))
      throw "Couldn't support GL_ARB_vertex_shader";
	if(!glh_init_extensions("GL_ARB_fragment_shader"))
      throw "Couldn't support GL_ARB_fragment_shader";
	if(!glh_init_extensions("GL_ARB_multitexture"))
      throw "Couldn't support GL_ARB_multitexture";
	if(!glh_init_extensions("WGL_ARB_render_texture"))
      throw "Couldn't support WGL_ARB_render_texture";
	if(!glh_init_extensions("WGL_NV_render_texture_rectangle"))
      throw "Couldn't support WGL_NV_render_texture_rectangle"; // not available on GeForce 5600 etc
	if(!glh_init_extensions("GL_NV_texture_rectangle"))
      throw "Couldn't support GL_NV_texture_rectangle";
	if(!glh_init_extensions("GL_EXT_texture3D"))
      throw "Couldn't support GL_EXT_texture3D";
	if(!glh_init_extensions("GL_NV_pixel_data_range"))
      throw "Couldn't support GL_NV_pixel_data_range";
//	if(!glh_init_extensions("GL_ARB_texture_non_power_of_two"))
//      throw "Couldn't support GL_ARB_texture_non_power_of_two";
#else
	if(!glh_init_extensions(REQUIRED_EXTENSIONS))
      throw "Couldn't support all the necessary OpenGL extensions";
#endif

    // Create everyone's favorite singletons
    TestFilterData data;
    TileManager tiler;
    PreviewManager preview;
    GPUFilter filter;

    // Show the GUI. If the user presses OK, execute the filter.
    if(preview.ShowPreview() == IDOK)
      filter.ApplyFilter();
  }
  catch(const char *err)
  {
    MessageBox(NULL, err, "error", MB_OK);
    *gResult = 1;
  }
}

void DoContinue(void)
{
	VRect outRect = {0, 0, 0, 0}; // forcing outRect null says "don't call me here any more"
	SetOutRect(outRect);
}

void DoFinish(void)
{
}

/////////// MAIN ENTRY POINT HERE //////////////////

DLLExport SPAPI void PluginMain(
  const int16 selector,
  void *filterRecord,
  int32 *data,
  int16 *result)
{
	gData = data;
	gResult = result;

  gFilterRecord = (FilterRecordPtr)filterRecord;
	if (selector == filterSelectorAbout)
	{
		sSPBasic = ((AboutRecordPtr)filterRecord)->sSPBasic;
	} 
	else 
	{
		sSPBasic = gFilterRecord->sSPBasic;

		if (gFilterRecord->bigDocumentData != NULL)
			gFilterRecord->bigDocumentData->PluginUsing32BitCoordinates = true;
	}

	switch (selector)
	{
		case filterSelectorAbout:
			DoAbout();
			break;
		case filterSelectorParameters:
			DoParameters(); // currently Empty - KB
			break;
		case filterSelectorPrepare:
			DoPrepare();
			break;
		case filterSelectorStart:
			DoStart();
			break;
		case filterSelectorContinue:
			DoContinue();
			break;
		case filterSelectorFinish:
			DoFinish(); // currently Empty - KB
			break;
		default:
			*gResult = filterBadParameters;
			break;
	}
}