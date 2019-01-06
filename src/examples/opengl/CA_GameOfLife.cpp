#if defined(WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#if defined(MACOS)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <glh/glh_extensions.h>
#include <nvparse.h>
#include <shared/read_text_file.h>
#include <shared/pbuffer.h>
#include <shared/quitapp.h>
#include <iostream>
#include "CA_GameOfLife.hpp"
#include "ppm.hpp"

using namespace std;

//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::CA_GameOfLife                                |
//|   Description: Constructor.                                                |
//.----------------------------------------------------------------------------.
CA_GameOfLife::CA_GameOfLife()
: _iCurrentSourceID(0),
  _iVertexProgramID(0),
  _iFlipState(0),
  _bWrap(true),
  _bReset(true),
  _bSingleStep(false),
  _bAnimate(true),
  _bSlow(false),
  _bWireframe(false),
  _bOccasionalExcitation(false),
  _bAlphaTest(false),
  _rAlphaRef(0),
  _iSlowDelay(25),
  _iSkipInterval(0),
  _iExcitationInterval(400),
  _pPixelBuffer(NULL),
  _eRenderMode(CA_FULLSCREEN_NEIGHBOR_CALC)
{
    memset(_pRulesDimensions, 0, sizeof(_pRulesDimensions));
    memset(_pInitialMapDimensions, 0, sizeof(_pInitialMapDimensions));
    memset(_pStaticTextureIDs, 0, sizeof(_pStaticTextureIDs));
    memset(_pDynamicTextureIDs, 0, sizeof(_pDynamicTextureIDs));
    memset(_pDisplayListIDs, 0, sizeof(_pDisplayListIDs));
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::~CA_GameOfLife                               |
//|   Description: Destructor                                                  |
//.----------------------------------------------------------------------------.
CA_GameOfLife::~CA_GameOfLife()
{
    int i;

    for (i = 0; i < CA_NUM_STATIC_TEXTURES; i++)
    {
        if (_pStaticTextureIDs[i])
            glDeleteTextures(1, &_pStaticTextureIDs[i]);
    }

    for (i = 0; i < CA_NUM_DYNAMIC_TEXTURES; i++)
    {
        if (_pDynamicTextureIDs[i])
            glDeleteTextures(1, &_pDynamicTextureIDs[i]);
    }
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::Initialize                                   |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::Initialize(const char *pRulesFilename, const char *pInitialMapFilename)
{
    _LoadRulesAndInitialMap(pRulesFilename, pInitialMapFilename);

    if (_pInitialMapDimensions[0] <= 0 && _pInitialMapDimensions[1] <= 0)
    {
        fprintf(stderr, "Unable to load initial map");
        quitapp(-1);
    }
    
    // create the pbuffer.  Will use this as an offscreen rendering buffer.
    // it allows rendering a texture larger than our window.
    _pPixelBuffer = new PBuffer("rgb");
    _pPixelBuffer->Initialize(_pInitialMapDimensions[0], _pInitialMapDimensions[1], false, true);
    _pPixelBuffer->Activate();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);	
    
    glClearColor(0, 0, 0, 0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // make the offsets used by the vertex program to generated texcoords and 
    // load them into contstant memory
    _CreateAndWriteUVOffsets(_pInitialMapDimensions[0], _pInitialMapDimensions[1]);

    char * programBuffer;

    ///////////////////////////////////////////////////////////////////////////
    // UV Offset Vertex Program
    ///////////////////////////////////////////////////////////////////////////

    glGenProgramsARB(1, &_iVertexProgramID);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _iVertexProgramID);

    programBuffer = read_text_file("GL_GameOfLife/Texcoord_4_Offset.vp");
    
    nvparse(programBuffer);
  
    for (char * const * errors = nvparse_get_errors(); *errors; errors++)
        fprintf(stderr, *errors);
    delete [] programBuffer;

    ///////////////////////////////////////////////////////////////////////////
    // register combiner setup for equal weight combination of texels
    ///////////////////////////////////////////////////////////////////////////
    programBuffer = read_text_file("GL_GameOfLife/EqWeightCombine.rcp");
    
    _pDisplayListIDs[CA_REGCOMBINER_EQ_WEIGHT_COMBINE] = glGenLists(1);
    glNewList(_pDisplayListIDs[CA_REGCOMBINER_EQ_WEIGHT_COMBINE], GL_COMPILE);
    {
        nvparse(programBuffer);
         
        for (char * const * errors = nvparse_get_errors(); *errors; errors++)
           fprintf(stderr, "Register Combiner error: %s\n", *errors);
        glEnable(GL_REGISTER_COMBINERS_NV);
    }
    glEndList();
    delete [] programBuffer;


    ///////////////////////////////////////////////////////////////////////////
    // register combiner setup for discarding all but blue and then biasing blue and green.
    ///////////////////////////////////////////////////////////////////////////
    programBuffer = read_text_file("GL_GameOfLife/TexelsToBlueWithBias.rcp");

    _pDisplayListIDs[CA_REGCOMBINER_TEXELS_TO_BLUE_WITH_BIAS] = glGenLists(1);
    glNewList(_pDisplayListIDs[CA_REGCOMBINER_TEXELS_TO_BLUE_WITH_BIAS], GL_COMPILE);
    {    
        nvparse(programBuffer);

        for (char * const * errors = nvparse_get_errors(); *errors; errors++)
           fprintf(stderr, "Register Combiner error: %s\n", *errors);
        glEnable(GL_REGISTER_COMBINERS_NV);
    }
    glEndList();

    ///////////////////////////////////////////////////////////////////////////
    // texture shader / reg combiner for dependent green-blue texturing.
    ///////////////////////////////////////////////////////////////////////////
    _pDisplayListIDs[CA_TEXSHADER_DEPENDENT_GB] = glGenLists(1);
    glNewList(_pDisplayListIDs[CA_TEXSHADER_DEPENDENT_GB], GL_COMPILE);
    {

        programBuffer = read_text_file("GL_GameOfLife/DependentGB.ts");

        // Configure texture shader
        nvparse(programBuffer);
            
        char * const * errors;
        
        for (errors = nvparse_get_errors(); *errors; errors++)
            fprintf(stderr, "Texture shader error: %s\n", *errors);
        delete [] programBuffer;

        programBuffer = read_text_file("GL_GameOfLife/Tex1ToOutput.rcp");

        nvparse(programBuffer);

        for (errors = nvparse_get_errors(); *errors; errors++)
            fprintf(stderr, "Register Combiner error: %s\n", *errors);
        delete [] programBuffer;
        
        glEnable(GL_TEXTURE_SHADER_NV);
        glEnable(GL_REGISTER_COMBINERS_NV);
    }
    glEndList();


    ///////////////////////////////////////////////////////////////////////////
    // display list to render a single screen space quad.
    ///////////////////////////////////////////////////////////////////////////
    _pDisplayListIDs[CA_DRAW_SCREEN_QUAD] = glGenLists(1);
    glNewList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD], GL_COMPILE);
    {
        glColor4f(1, 1, 1, 1);
        glBegin(GL_TRIANGLE_STRIP);
        {
            glTexCoord2f(0, 1); glVertex2f(-1,  1);
            glTexCoord2f(0, 0); glVertex2f(-1, -1);
            glTexCoord2f(1, 1); glVertex2f( 1,  1);
            glTexCoord2f(1, 0); glVertex2f( 1, -1);
            
            
        }
        glEnd();
    }
    glEndList();

    _pPixelBuffer->Deactivate();
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::Display                                      |
//|   Description: Display the current status of the game of life.             |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::Display()
{
    static unsigned int nSkip = 0;

	if( nSkip >= _iSkipInterval && (_eRenderMode != CA_DO_NOT_RENDER) )
	{
		nSkip = 0;

		// Display the results of the rendering to texture
		if( _bWireframe )
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
           
			// chances are the texture will be all dark, so lets not use a texture
            glDisable(GL_TEXTURE_2D);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            			
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
        }

        switch( _eRenderMode ) // more later
        {
        case CA_FULLSCREEN_FINALOUT:
            {
                // Draw quad over full display
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP]);
                
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
                
            }
            break;
        case CA_FULLSCREEN_NEIGHBOR_CALC:
            {
                // Draw quad over full display
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE]);
			                 
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
			    
            }
            break;
        case CA_TILED_THREE_WINDOWS:
            {
                // Draw quad over full display
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE]);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
			                 
                glTranslatef(-0.5f, -0.5f, 0);
                glScalef(0.5f, 0.5f, 1);
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
                glPopMatrix();

                glPushMatrix();
			                 
                glTranslatef(0.5f, -0.5f, 0);
                glScalef(0.5f, 0.5f, 1);
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
                glPopMatrix();

                glBindTexture(GL_TEXTURE_2D, _pStaticTextureIDs[CA_TEXTURE_INITIAL_MAP]);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
			                 
                glTranslatef(-0.5f, 0.5f, 0);
                glScalef(0.5f, 0.5f, 1);
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
                glPopMatrix();

                glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP]);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
			                 
                glTranslatef(0.5f, 0.5f, 0);
                glScalef(0.5f, 0.5f, 1);
                glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
                glPopMatrix();
			    
            }
            break;
        case CA_DO_NOT_RENDER:
            break;
        }
    }
   	else
	{
		// skip rendering this frame
		nSkip++;
	}   
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::Tick                                         |
//|   Description: Take a single step in the cellular automaton.               |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::Tick()
{
    // make the pbuffer the current target
    _pPixelBuffer->Activate();

    // Disable culling
	glDisable(GL_CULL_FACE);

    if(_bReset)
	{
		_bReset = false;
		_iFlipState = 0;
        glClear(GL_COLOR_BUFFER_BIT);
	}

	if(0 == _iFlipState)
	{
		_iCurrentSourceID = _pStaticTextureIDs[CA_TEXTURE_INITIAL_MAP];
	}
	else if(_bAnimate || _bSingleStep )
	{
		switch( _iFlipState )
		{
		case 1:
			// Here, over a single time step, rendering goes
			//   from source field to intermediate texture,
			//   then back to source field (new generation)
			//   So there is only one flipstate besides the
			//   initial one of "0" which uses the initial
			//   conditions texture.

			_iCurrentSourceID = _pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP];

			break;
		}
	}

	if(_bAnimate)
	{
		// Update the textures for one step of the 
		//   game or cellular automata 
		_DoSingleTimeStep3Pass();
    }
	else if(_bSingleStep)
	{
		_DoSingleTimeStep3Pass();
		_bSingleStep = false;
	}
	else
	{
		// slow down the rendering and give any other apps a chance
#ifdef _WIN32
		Sleep(70);
#else
        usleep(70);
#endif
	}

	if( _bSlow && (_iSlowDelay > 0) )
	{
#ifdef _WIN32
		Sleep(_iSlowDelay);
#else
        usleep(_iSlowDelay);
#endif
	}

    _pPixelBuffer->Deactivate();
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::_DoSingleTimeStep3Pass                       |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::_DoSingleTimeStep3Pass()
{
    // variable for writing to constant memory which uv-offsets to use
    static float offset[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// even if wireframe mode, render to texture as solid
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
    /////////////////////////////////////////////////////////
    // Comment from Greg James' original D3D implementation:
    //
	// First, write the source texture into the blue channel
	// I do this in preparation for a 2D dependent green-blue lookup
	// into a "rules" texture which governs how old pixels spawn
	// or die into new pixels.  
	// The logic for the game of life depends on 9 pixels:  the source
	// pixel and it's 8 neightbors.  These are accumulated in three
	// passes.
    //
    // in D3D, he sets a render target and renders directly to it.  
    // in GL, we will render to the back buffer, and then copy the pixels
    // to the appropriate texture
    /////////////////////////////////////////////////////////////////

    // Set simple pixel shader to multiply input texture by RGB = 0,0,1
	// to select only blue, and add a small component of green + blue to 
	// that in order to properly offset the dependent green-blue lookup
	// to be done in a later stage

    glCallList(_pDisplayListIDs[CA_REGCOMBINER_TEXELS_TO_BLUE_WITH_BIAS]);

    // set current source texture for stage 0 texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, _iCurrentSourceID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glEnable(GL_TEXTURE_2D);

    // disable blending
    glDisable(GL_BLEND);
    
    if (_bAlphaTest)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, _rAlphaRef);
    }
        
    // render a screen quad.
    glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);

  	////////////////////////////////////////////////////////////////
	// Add in nearest neighbors
    // Render set of neighbors using bilinear filtering to sample
	//   equal weight of all 8 neighbors in just 4 texture samples
    ////////////////////////////////////////////////////////////////   
    glCallList(_pDisplayListIDs[CA_REGCOMBINER_EQ_WEIGHT_COMBINE]);
    
    // Add result of pixel operations into the dest texture:
	// This adds the new green component from neighbor sampling
	// into the previous blue on/off result from above
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);  

    offset[0] = 3.0f;
    glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_OFFSET_TO_USE, offset);
    
    // bind the vertex program to be used for this step:
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, _iVertexProgramID);
    glEnable(GL_VERTEX_PROGRAM_ARB);

    // use the same source texture for 4 texture addresses:  this will equal weight the 
    // 8 neighbors of each texel.
    int i;
    for (i = 0; i < 4; i++)
    {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glBindTexture(GL_TEXTURE_2D, _iCurrentSourceID);
        glEnable(GL_TEXTURE_2D);
    }
    
    // need bilinear filtering to count 8 texels with only 4 samples.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum wrapMode = _bWrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // render a screen quad
    glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);

    glDisable(GL_ALPHA_TEST);

    // reset filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /////////////////////////////////////////////////////////////////
	//	Excitation of Game Of Life simulation
	//	Every 400 steps draw the cell field into itself
	//   at offset coordinates to spawn new activity.
	/////////////////////////////////////////////////////////////////

	static unsigned int iCount = 0;

	if( _bOccasionalExcitation )
	{
		iCount++;

		if( iCount > _iExcitationInterval )
		{
			// Additive blending
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_BLEND);

            glActiveTextureARB(GL_TEXTURE0_ARB);
            glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE]);
            
			// Render using scrolling offsets of a few texels
			offset[0] = 4.0f;
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_OFFSET_TO_USE, offset);

			glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
			iCount = 0;
		}
	}

    // Now we need to copy the resulting pixels into the intermediate texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE]);

    // use CopyTexSubImage for speed (even though we copy all of it) since we pre-allocated the texture
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _pInitialMapDimensions[0], _pInitialMapDimensions[1]);

    /////////////////////////////////////////////////////////////////
	// Now do dependent 2D lookup into rules texture to set new source pixels;
    // (Write white colors to final target)
    ////////////////////////////////////////////////////////////////
    glDisable(GL_BLEND);
    
    glCallList(_pDisplayListIDs[CA_TEXSHADER_DEPENDENT_GB]);
  
    // we are now effectively rendering to the target cell field.
    // in the D3D version, this is where he sets the render target to that texture.
    
    // VP not used.
    glDisable(GL_VERTEX_PROGRAM_ARB);

    // use the newly copied intermediate texture for the first stage.  Since we already bound and copied it
    // in stage zero above, no need to re-bind it here.
	
    // use the rules texture in the second stage.  Bind it here.
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, _pStaticTextureIDs[CA_TEXTURE_RULES]);

    // Draw the quad to render in all new cells 
    glCallList(_pDisplayListIDs[CA_DRAW_SCREEN_QUAD]);
    
    // always put away your toys when you are done! (without this, you get death, not life!)
    glDisable(GL_TEXTURE_SHADER_NV);
    
    // disable other textures -- this is important because if all four are disabled it is a performance 
    // hit, even if the results of the lookups are not used! (2 texels per cycle, 4 takes two cycles!)
    for (i = 1; i < 4; i++)
    {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glDisable(GL_TEXTURE_2D);
    }

    // Now we need to copy the resulting pixels into the final cell field texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, _pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP]);

    // use CopyTexSubImage for speed (even though we copy all of it) since we pre-allocated the texture
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _pInitialMapDimensions[0], _pInitialMapDimensions[1]);

    ///////////////////////////////////////////////////////////
	// Flip the state variable for the next round of rendering (only 1 state besides initial start state.)
    ///////////////////////////////////////////////////////////
    _iFlipState = 1;
}


//.----------------------------------------------------------------------------.
//|   Function   : CreateTextureObject                                         |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void CreateTextureObject(unsigned int ID, unsigned int iWidth, unsigned int iHeight, unsigned char *pData)
{
    glBindTexture(  GL_TEXTURE_2D, ID);
    glTexImage2D (  GL_TEXTURE_2D, 
                    0, 
                    GL_RGB8, 
                    iWidth,
                    iHeight, 
                    0, 
                    GL_RGB,
                    GL_UNSIGNED_BYTE, 
                    pData);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::LoadRulesAndInitialMap                       |
//|   Description: Load the textures used to run the CA.                       |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::_LoadRulesAndInitialMap(const char *pRulesFilename, const char *pInitialMapFilename)
{
    unsigned char   *pRulesTexture          = NULL; // The rules texture used at each step.
    unsigned char   *pInitialMapTexture     = NULL; // The map of initial life.

    // load the two starting textures
    LoadPPM(pRulesFilename,      pRulesTexture,      _pRulesDimensions[0],      _pRulesDimensions[1]);
    LoadPPM(pInitialMapFilename, pInitialMapTexture, _pInitialMapDimensions[0], _pInitialMapDimensions[1]);

    glGenTextures(CA_NUM_STATIC_TEXTURES, _pStaticTextureIDs); 
    glGenTextures(CA_NUM_DYNAMIC_TEXTURES, _pDynamicTextureIDs); // also create intermediate texture object

    // upload the rules texture
    CreateTextureObject(_pStaticTextureIDs[CA_TEXTURE_RULES], 
                        _pRulesDimensions[0], 
                        _pRulesDimensions[1], 
                        pRulesTexture);
    delete [] pRulesTexture;


    // upload the initial map texture
    CreateTextureObject(_pStaticTextureIDs[CA_TEXTURE_INITIAL_MAP],
                        _pInitialMapDimensions[0], 
                        _pInitialMapDimensions[1], 
                        pInitialMapTexture);

    // now create a dummy intermediate texture from the initial map texture
    CreateTextureObject(_pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE],
                        _pInitialMapDimensions[0], 
                        _pInitialMapDimensions[1], 
                        pInitialMapTexture);

    // now create a dummy target texture from the initial map texture
    CreateTextureObject(_pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP],
                        _pInitialMapDimensions[0], 
                        _pInitialMapDimensions[1], 
                        pInitialMapTexture);
    delete [] pInitialMapTexture;

 
}


//.----------------------------------------------------------------------------.
//|   Function   : CA_GameOfLife::_CreateAndWriteUVOffsets                     |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void CA_GameOfLife::_CreateAndWriteUVOffsets(unsigned int width, unsigned int height)
{
    // This sets vertex shader constants used to displace the
	//  source texture over several additive samples.  This is 
	//  used to accumulate neighboring texel information that we
	//  need to run the game - the 8 surrounding texels, and the 
	//  single source texel which will either spawn or die in the 
	//  next generation.
	// Label the texels as follows, for a source texel "e" that
	//  we want to compute for the next generation:
	//		
    //          abc
    //          def
    //          ghi:

    // first the easy one: no offsets for sampling center
	//  occupied or unoccupied
	// Use index offset value 0.0 to access these in the 
	//  vertex shader.
    
    float   rPerTexelWidth  = 1.0f / static_cast<float>(width);
    float   rPerTexelHeight = 1.0f / static_cast<float>(height);

  	// Offset set 0 : center texel sampling
    float noOffsetX[4] = { 0, 0, 0, 0 };
    float noOffsetY[4] = { 0, 0, 0, 0 };

    // Not Currently Used:
	// Offset set 1:  Nearest neighbors -  d,f,b,h texels
	// Use index offset 1.0 to access these.
    float type1OffsetX[4] = { -rPerTexelWidth, rPerTexelWidth, 0.0f, 0.0f };
    float type1OffsetY[4] = { 0.0f, 0.0f, rPerTexelHeight, -rPerTexelHeight };

    // Not Currently Used:
  	// These are a,g,c,i texels == diagonal neightbors
	// Use index offset 2.0 to use these
    float type2OffsetX[4] = { -rPerTexelWidth, -rPerTexelWidth,  rPerTexelWidth,   rPerTexelWidth  };
    float type2OffsetY[4] = { -rPerTexelHeight, rPerTexelHeight, rPerTexelHeight, -rPerTexelHeight };

  	// These offsets are for use with bilinear filtering
	// of the neighbors, to sample all 8 neighbors in
	// one pass instead of two.  Bilinear averages the
	// two bordering texels, but the coordinate must be
	// exactly on the texel border to make this work.
	//	[0] = on the border of the ab texels
	//  [1] = between cf texels
	//  [2] = between ih texels
	//	[3] = between gd texels
    float type3OffsetX[4] = { -rPerTexelWidth * 0.5f, rPerTexelWidth, 
                              rPerTexelWidth * 0.5f, -rPerTexelWidth };
    float type3OffsetY[4] = { rPerTexelHeight,	rPerTexelHeight * 0.5f,
                              -rPerTexelHeight, -rPerTexelHeight * 0.5f };

    /////////////////////////////////////////////////////////////////////
	//  One final rarely used set of offsets
	//  This one offsets by 3.5 texels in the vertical.  It's used by
	//    the class when the m_bOccasionalExcitation variable is true.
	//  This is here because Conway's game tends toward a stead state.
	//  With m_bOccasionalExcitation, every N frames the cell field is
	//    rendered additively into itself using these offsets set 4.
	//  This spawns new active regions from a populated cell field and
	//    keeps interesting activity going.

  	float rHoff = 1.5f * rPerTexelHeight;

    float type4OffsetX[4] = { -rPerTexelWidth, rPerTexelWidth, 0.0f, 0.0f };
    float type4OffsetY[4] = { 0.0f + rHoff, 0.0f + rHoff, 
                              -rPerTexelHeight + rHoff, rPerTexelHeight + rHoff };

    // write all these offsets to constant memory
    for (int i = 0; i < 4; ++i)
    {
        float noOffset[]    = { noOffsetX[i],    noOffsetY[i],    0.0f, 0.0f };
        float type1Offset[] = { type1OffsetX[i], type1OffsetY[i], 0.0f, 0.0f };
        float type2Offset[] = { type2OffsetX[i], type2OffsetY[i], 0.0f, 0.0f };
        float type3Offset[] = { type3OffsetX[i], type3OffsetY[i], 0.0f, 0.0f };
        float type4Offset[] = { type4OffsetX[i], type4OffsetY[i], 0.0f, 0.0f };

        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_T0_NO_OFFSET + 5*i, noOffset);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_T0_TYPE1     + 5*i, type1Offset);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_T0_TYPE2     + 5*i, type2Offset);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_T0_TYPE3     + 5*i, type3Offset);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, CV_UV_T0_TYPE4     + 5*i, type4Offset);
    }
}

