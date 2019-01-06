#include <GL/glew.h>
#include <GL/gl.h>

#include <ShaderManager.h>
#include "ProgramCg.h"
#include "ShaderGLSL.h"

#include <assert.h>

#define CHECKCGERROR() \
    Ret = cgGetError();\
    if(Ret != CG_NO_ERROR)\
    {\
        fprintf(stderr,"CgError : %s\n", cgGetErrorString(Ret));\
        if (Ret == CG_COMPILER_ERROR) { \
            fprintf(stderr, "%s\n", cgGetLastListing(ShaderManager::gCgContext)); \
        } \
    }


ProgramCg::ProgramCg(const char* fragmentShaderFilename) :
	bUseVertex(false),
	bUseFragment(true)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;

  load(fragmentShaderFilename, NULL);

//  InitUniforms();
}

ProgramCg::ProgramCg(const char* vertexShaderFilename, const char* fragmentShaderFilename) :
	bUseVertex(true),
	bUseFragment(true)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;

  load(vertexShaderFilename, fragmentShaderFilename);

//  InitUniforms();
}

ProgramCg::ProgramCg(const char* vertexShaderFilename, const char* vertexMain, const char **vertexArgs,
					 const char* fragmentShaderFilename, const char* fragMain, const char **fragArgs) :
	bUseVertex(true),
	bUseFragment(true)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;

  load(	vertexShaderFilename, vertexMain, vertexArgs,
		fragmentShaderFilename, fragMain, fragArgs);

//  InitUniforms();
}

ProgramCg::~ProgramCg(void)
{
    if (mVertexShader)   cgDestroyProgram(mVertexShader);
    if (mFragmentShader) cgDestroyProgram(mFragmentShader);

    mVertexShader   = NULL;
    mFragmentShader = NULL;
}

bool ProgramCg::load(const char* fragmentShaderFilename, const char *fragMain)
{
  assert(fragmentShaderFilename != NULL);

  mFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(mFragmentProfile );

  //
  // Load the vertex program. Optionaly have a look in the resources
  //
  printf("  Cg::load( %-30s)", fragmentShaderFilename);
  mFragmentShader = cgCreateProgramFromFile(	ShaderManager::gCgContext, 
												CG_SOURCE, fragmentShaderFilename,
												mFragmentProfile, fragMain, NULL);
  CHECKCGERROR();
  
  if(mFragmentShader == NULL) {
	printf(" - FAILED!\n");
	return false;
  } else {
	printf("\n");
  }
  cgGLLoadProgram(mFragmentShader);
  CHECKCGERROR();

  // now we can get al the named parameers

  // TODO check for errors

  return(true);
}

bool ProgramCg::load(const char* vertexShaderFilename, const char *vertexMain, const char **vertexArgs,
					 const char* fragmentShaderFilename, const char *fragMain, const char **fragArgs)
{
  assert(vertexShaderFilename != NULL);
  assert(fragmentShaderFilename != NULL);

  // Load the Cg vertex program.
  mVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  cgGLSetOptimalOptions(mVertexProfile);

  printf("  Cg::load( %-30s, ", vertexShaderFilename);
  mVertexShader = cgCreateProgramFromFile(  ShaderManager::gCgContext, 
                                            CG_SOURCE, vertexShaderFilename,
											mVertexProfile, vertexMain, vertexArgs);
  CHECKCGERROR();

  if(mVertexShader == NULL) {
	printf(" Failed! \n");
	return false;
  }

  cgGLLoadProgram(mVertexShader);
  CHECKCGERROR();

  // Load the Cg fragment program.
  mFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(mFragmentProfile);

  printf("%-30s)", fragmentShaderFilename);
  mFragmentShader = cgCreateProgramFromFile(ShaderManager::gCgContext, 
											CG_SOURCE, fragmentShaderFilename,
											mFragmentProfile, fragMain, fragArgs);
  CHECKCGERROR();

  if(mFragmentShader == NULL) {
        printf("- FAILED\n");
        return false;
  } else {
      printf("\n");
  }
  cgGLLoadProgram(mFragmentShader);
  CHECKCGERROR();


  // now we can get al the named parameers

  // TODO check for errors

  return(true);
}


void ProgramCg::setWindowSize(int width, int height)
{
    mWinSize.nX = width;
    mWinSize.nY = height;
}

void ProgramCg::setTextureSize(int width, int height, int tex_unit)
{
    mTexSize[tex_unit].nX = width;
    mTexSize[tex_unit].nY = height;
}

void ProgramCg::bind()
{
	if (bUseVertex) {
		cgGLEnableProfile(mVertexProfile);
		cgGLBindProgram(mVertexShader);
	}

	if (bUseFragment) {
		// enable the fragment program
		cgGLEnableProfile(mFragmentProfile);
		cgGLBindProgram(mFragmentShader);
	}

    for (int tex_unit=0; tex_unit < 4; tex_unit++) {
		if (cgIsParameter(gUniforms.mTexture[tex_unit])) {
			cgGLEnableTextureParameter(gUniforms.mTexture[tex_unit]);
//            cgGLSetTextureParameter( gUniforms.mTexture[tex_unit], 0 );
		}
		if (cgIsParameter(gUniforms.mSceneMap[tex_unit])) {
			cgGLEnableTextureParameter(gUniforms.mSceneMap[tex_unit]);
//            cgGLSetTextureParameter( gUniforms.mSceneMap[tex_unit], 0 );
		}
    }

	if (cgIsParameter(gUniforms.mTexSize))
        cgGLSetParameter2f( gUniforms.mTexSize, mTexSize[0].nX, mTexSize[0].nY );
}

void ProgramCg::unbind()
{

    for (int tex_unit=0; tex_unit < 4; tex_unit++) {
		if (cgIsParameter(gUniforms.mTexture[tex_unit])) {
			cgGLDisableTextureParameter(gUniforms.mTexture[tex_unit]);
		}
		if (cgIsParameter(gUniforms.mSceneMap[tex_unit])) {
			cgGLDisableTextureParameter(gUniforms.mSceneMap[tex_unit]);
		}
    }

    if (bUseFragment)	cgGLDisableProfile(mFragmentProfile); // same as     glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, 0); ???
    if (bUseVertex)		cgGLDisableProfile(mVertexProfile);


}

void ProgramCg::InitUniforms()
{
    gUniforms.initUniforms(mVertexShader, mFragmentShader);
}

#if 0
void ProgramCg::InitSpecialTextures(glh::tex_object_2D & tex_dust,
                                  glh::tex_object_2D & tex_lines,
                                  glh::tex_object_2D & tex_tv,
                                  glh::tex_object_3D & tex_noise)
{
    gUniforms.initSpecialTextures(  tex_dust, 
                                    tex_lines, 
                                    tex_tv, 
                                    tex_noise);
}
#endif
