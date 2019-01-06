// ----------------------------------------------------------------------------
// File:
//      main.cpp
//
// Content:
//      All C++ code of this demo.
//
// Description:
//      This is the main source file for the "shades" demo.
//          This demo shows how pixel shaders can be used to render
//      evenly spaced rectangular bars using a single quad. Examples
//      for such materials are window blinds, prison bars, air-duct covers,
//      et cetera.
//          The advantage of this technique over modeling such man-made, 
//      highly repetitive materials are
//        - rendering complexity is dominated by the number of pixel
//          shaded, not the number of geometric objects.
//        - the "virtual geometry" can be applied like a texture (or material
//          effect) to very simple base geometry. Artist simply tweaks the
//          spacing between slats/bars.
//
// Documentation:
//      With this software you should have received a copy of the 
//      whitepaper explaining the ideas and math behind the shaders as
//      well as a user guide explaining the features of this software.
//      Both these documents should be located in the Doc subfolder of
//      the demo's root directory.
//
// Author:
//      Frank Jargstorff (fjargstorff@nvidia.com)
//
// Note:
//      Copyright (c) 2004 by NVIDIA Corporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#if defined(WIN32)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

//
// Define used for our extension handling mechanism.
//
#define GLH_EXT_SINGLE_FILE


#include <Cg/cgGL.h>

#include <shared/data_path.h>
#include <shared/quitapp.h>
#include <paramgl.h>
#include <nv_dds.h>
#include <glh/glh_extensions.h>

#include <string>
#include <iostream>


//
// Namespaces
//

using namespace std;


//
// Define used for our extension handling mechanism.
//
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture "

//
// Global data
//

    //
    // Cg handles
    //

CGcontext hContext; 

CGprofile hFragmentProfile;
CGprofile hVertexProfile;

    // Vertex program
CGprogram hVertexProgram;

    // Vertex program parameters
CGparameter hN; // {hN, hU, hV} for a orthnormal basis for the
CGparameter hU; // material-coordinate-spaced used in the shaders
CGparameter hV; // (analogous to texture-coordinate space for bumpy-shiny)
CGparameter hSlatNormal;
CGparameter hLightPosition;
CGparameter hDiffuseLight;
CGparameter hAmbientLight;
CGparameter hDiffuseMaterial;
CGparameter hSpecularMaterial;
CGparameter hShininessVP;

CGparameter hModelViewProjection;
CGparameter hInverseModelView;

    // Fragment programs
CGprogram hAlphaTestShades;
CGprogram hDiscardFragmentShades;
CGprogram hDepthReplaceShades;
CGprogram hSelfShadowingShades;
CGprogram hAntiAliasedShades;
CGprogram hTexturedShades;
CGprogram hClippedEndsShades;

    // Fragment program parameters
CGparameter hAmbientColor;  // preevaluated ambient lighting term
CGparameter hDiffuseColor;
CGparameter hSpecularColor;
CGparameter hShininess;
CGparameter hSpacing;       // space between slats
CGparameter hHeight;        // slat height
CGparameter hOffset;        // slat offset
CGparameter hScaleTexture;
CGparameter hTexture;
CGparameter hAlphaTexture;


    //
    // GL
    //
    
GLuint ghTexture;
GLuint ghAlphaTexture;


    //
    // Interaction
    //

int  gnMouseOldX, gnMouseOldY;
bool gbLeftMouseButtonPressed = false;
bool gbMiddleMouseButtonPressed = false;

float mCameraRotation[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f};
                             
float gnCameraDistance = 5.0f;

bool gbWireFrame  = false;
bool gbDrawFrame  = true;
bool gbDrawSphere = false;
bool gbSlatsLast  = true;
bool gbAlphaBlend = true;


//
// Global constants
//

const float gvDirectionU[]       = {1.0f, 0.0f, 0.0f};
const float gvDirectionV[]       = {0.0f, 1.0f, 0.0f};
const float gvDirectionN[]       = {0.0f, 0.0f, 1.0f};

const float gvLightPosition[]    = {5.0f, 5.0f, 5.0f, 1.0f};
const float gcDiffuseLight[]     = {1.0f, 1.0f,  1.0f, 1.0f};
const float gcAmbientLight[]     = {0.2f, 0.2f,  0.2f, 1.0f};
const float gcDiffuseMaterial[]  = {0.2f, 0.8f,  0.2f, 1.0f};
const float gcSpecularMaterial[] = {0.5f, 0.5f,  0.5f, 1.0f};
const float gnShininess          = 50.0f;


//
// Sliders for GUI
//

bool            gbSliders = false;
ParamListGL	    gSliders;
Param<float>    gSlats(      "Slats",  10.0f,  2.0f, 50.0f, 1.0f);
Param<float>    gSlatHeight( "Height",  0.5f,  0.0f,  2.5f, 0.1f);
Param<float>    gSlatWidth(  "Width",   0.1f,  0.0f,  1.0f, 0.05f);
Param<float>    gSlatOffset( "Offset",  0.0f, -5.0f,  5.0f, 0.1f);


//
// Implementation
//

        // glAssert
        //
        // Description:
        //      Check for OpenGL errors.
        //          If error is found retrieve the error string
        //      and print meaningful message to stderr. Exit app with
        //      return value of -1 to indicate error.
        //
        // Parameters:
        //      zFile - name of the file in which the error check is performed.
        //      nLine - line number where the error check is performed.
        //
        void 
glAssert(const char * zFile, unsigned int nLine)
{
    GLenum nErrorCode = glGetError();

    if (nErrorCode != GL_NO_ERROR)
    {
        const GLubyte * zErrorString = gluErrorString(nErrorCode);
        std::cerr << "Assertion failed (" <<zFile << ":"
                  << nLine << ": " << zErrorString << std::endl;

        quitapp(-1);
    }
}

        // GL_ASSERT_NO_ERROR
        //
        // Description:
        //      This is a little helper macro asserting OpenGL is in good shape.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
#define GL_ASSERT_NO_ERROR {glAssert(__FILE__, __LINE__);}

        // initCG
        //
        // Description:
        //      Initialize all the Cg related stuff. 
        //          Checks for availability of required profiles, creates Cg context, and
        //      loads all the shader programms.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void 
initCG(void)
{
    if (cgGLIsProfileSupported(CG_PROFILE_VP30))
        hVertexProfile = CG_PROFILE_VP30;
    else
    {
        printf("Vertex programming extensions (GL_NV_vertex_program) not supported, exiting...\n");
        quitapp(0);
    }

    if (cgGLIsProfileSupported(CG_PROFILE_FP30))
        hFragmentProfile = CG_PROFILE_FP30;
    else
    {
        printf("Fragment programming extensions (GL_NV_fragment_program) not supported, exiting...\n");
        quitapp(0);
    }
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");

	hContext = cgCreateContext();

    string sFilename = media.get_file(("programs/cg_shades/SimpleShadesFP.cg"));
    assert(!sFilename.empty());
    hAlphaTestShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hAlphaTestShades);
    cgGLLoadProgram(hAlphaTestShades);
    
    sFilename = media.get_file(("programs/cg_shades/DiscardFragmentShadesFP.cg"));
    assert(!sFilename.empty());
    hDiscardFragmentShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hDiscardFragmentShades);
    cgGLLoadProgram(hDiscardFragmentShades);

    sFilename = media.get_file(("programs/cg_shades/SelfShadowingShadesFP.cg"));
    assert(!sFilename.empty());
    hSelfShadowingShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hSelfShadowingShades);
    cgGLLoadProgram(hSelfShadowingShades);

    sFilename = media.get_file(("programs/cg_shades/AntiAliasedShadesFP.cg"));
    assert(!sFilename.empty());
    hAntiAliasedShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hAntiAliasedShades);
    cgGLLoadProgram(hAntiAliasedShades);

    sFilename = media.get_file(("programs/cg_shades/TexturedShadesFP.cg"));
    assert(!sFilename.empty());
    hTexturedShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hTexturedShades);
    cgGLLoadProgram(hTexturedShades);
    
    sFilename = media.get_file(("programs/cg_shades/ClippedEndsShadesFP.cg"));
    assert(!sFilename.empty());
    hClippedEndsShades = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                              hFragmentProfile, 0, 0);
    assert(hClippedEndsShades);
    cgGLLoadProgram(hClippedEndsShades);
    

    sFilename = media.get_file(("programs/cg_shades/ShadesVP.cg"));
    assert(!sFilename.empty());
    hVertexProgram = cgCreateProgramFromFile(hContext, CG_SOURCE, sFilename.c_str(),
                                             hVertexProfile, 0, 0);
    assert(hVertexProgram);
    cgGLLoadProgram(hVertexProgram);
}

        // bindVertexProgram
        //
        // Description:
        //      Bind a vertex program and all its paramters.
        //          All parameters of the given program are retrived and
        //      assigned to the global parameter handles. Then the program
        //      is bound and all the parameter values get set according to the
        //      current state.
        //
        // Parameters:
        //      hVertexProgram - a handle to the vertex program to bind.
        //
        // Returns:
        //      None
        // 
        void 
bindVertexProgram(CGprogram hVertexProgram)
{
    hModelViewProjection = cgGetNamedParameter(hVertexProgram, "mModelView"); 
    assert(hModelViewProjection);
    hInverseModelView    = cgGetNamedParameter(hVertexProgram, "mInverseModelView" ); 
    assert(hInverseModelView);
    
    hN = cgGetNamedParameter(hVertexProgram, "vN");
    assert(hN);
    hU = cgGetNamedParameter(hVertexProgram, "vU");
    assert(hU);
    hV = cgGetNamedParameter(hVertexProgram, "vV");
    assert(hV);
    hSlatNormal = cgGetNamedParameter(hVertexProgram, "vSlatNormal");
    assert(hSlatNormal);
    
    hLightPosition  = cgGetNamedParameter(hVertexProgram, "vLightPosition");
    assert(hLightPosition);
    hDiffuseLight   = cgGetNamedParameter(hVertexProgram, "cDiffuseLight");
    assert(hDiffuseLight);
    hAmbientLight   = cgGetNamedParameter(hVertexProgram, "cAmbientLight");
    assert(hAmbientLight);
    hDiffuseMaterial = cgGetNamedParameter(hVertexProgram, "cDiffuseMaterial");
    assert(hDiffuseMaterial);
    hSpecularMaterial = cgGetNamedParameter(hVertexProgram, "cSpecularMaterial");
    assert(hSpecularMaterial);
    hShininessVP     = cgGetNamedParameter(hVertexProgram, "nShininess");
    assert(hShininessVP);
  
    cgGLBindProgram(hVertexProgram);
    
    cgGLSetParameter3fv(hU, gvDirectionU);
    cgGLSetParameter3fv(hV, gvDirectionV);
    cgGLSetParameter3fv(hN, gvDirectionN);

    cgGLSetParameter4fv(hLightPosition,    gvLightPosition);
    cgGLSetParameter4fv(hDiffuseLight,     gcDiffuseLight);
    cgGLSetParameter4fv(hAmbientLight,     gcAmbientLight);
    cgGLSetParameter4fv(hDiffuseMaterial,  gcDiffuseMaterial); 
    cgGLSetParameter4fv(hSpecularMaterial, gcSpecularMaterial);
    cgGLSetParameter1f( hShininessVP,      gnShininess);
}

        // bindFragmentProgram
        //
        // Description:
        //      Bind a fragment program and all its paramters.
        //          All parameters of the given program are retrived and
        //      assigned to the global parameter handles. Then the program
        //      is bound and all the parameter values get set according to the
        //      current state.
        //
        // Parameters:
        //      hFragmentProgram - a handle to the vertex program to bind.
        //
        // Returns:
        //      None
        // 
        void
bindFragmentProgram(CGprogram hFragmentProgram)
{
    hAmbientColor     = cgGetNamedParameter(hFragmentProgram, "cAmbientColor");
    assert(hAmbientColor);
    hDiffuseColor     = cgGetNamedParameter(hFragmentProgram, "cDiffuseColor");
    assert(hDiffuseColor);
    hSpecularColor    = cgGetNamedParameter(hFragmentProgram, "cSpecularColor");
    assert(hSpecularColor);
    hShininess        = cgGetNamedParameter(hFragmentProgram, "nShininess");
    assert(hShininess);
    hSpacing          = cgGetNamedParameter(hFragmentProgram, "nSpacing");
    assert(hSpacing);
    hHeight           = cgGetNamedParameter(hFragmentProgram, "nHeight");
    assert(hHeight);
    hOffset           = cgGetNamedParameter(hFragmentProgram, "nOffset");
    assert(hOffset);
    hScaleTexture     = cgGetNamedParameter(hFragmentProgram, "vScaleTexture");
    assert(hScaleTexture);
    hTexture          = cgGetNamedParameter(hFragmentProgram, "oTexture");
    assert(hTexture);
    hAlphaTexture     = cgGetNamedParameter(hFragmentProgram, "oAlphaTexture");
    assert(hAlphaTexture);
    
    cgGLBindProgram(hFragmentProgram);

    cgGLSetParameter4f(hAmbientColor,  gcAmbientLight[0]* gcDiffuseMaterial[0]
                                    ,  gcAmbientLight[1]* gcDiffuseMaterial[1]
                                    ,  gcAmbientLight[2]* gcDiffuseMaterial[2]
                                    ,  gcAmbientLight[3]* gcDiffuseMaterial[3]);
                                         
    cgGLSetParameter4f(hDiffuseColor,  gcDiffuseLight[0]* gcDiffuseMaterial[0]
                                    ,  gcDiffuseLight[1]* gcDiffuseMaterial[1]
                                    ,  gcDiffuseLight[2]* gcDiffuseMaterial[2]
                                    ,  gcDiffuseLight[3]* gcDiffuseMaterial[3]);
                                    
    cgGLSetParameter4f(hSpecularColor, gcDiffuseLight[0]* gcSpecularMaterial[0]
                                    ,  gcDiffuseLight[1]* gcSpecularMaterial[1]
                                    ,  gcDiffuseLight[2]* gcSpecularMaterial[2]
                                    ,  gcDiffuseLight[3]* gcSpecularMaterial[3]);
                                    
    cgGLSetParameter1f(hShininess, 50.0f);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_1D, ghAlphaTexture);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, ghTexture);    
}

        // initGL
        //
        // Description:
        //      Initialize OpenGL stuff.
        //          Check for availability of required extensions, set up necessary state,
        //      and the light source.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void 
initGL(void)
{
    // Get the entry points for the extension.
	if(!glh_init_extensions(REQUIRED_EXTENSIONS))
	{
        printf("Necessary extensions unsupported: %s\n", glh_get_unsupported_extensions());
        quitapp(0);
	}

    glClearColor(0.6, 0.6, 0.6, 1.0);
    glShadeModel(GL_SMOOTH);
    
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    if (gbAlphaBlend)
        glEnable(GL_BLEND);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glLightfv(GL_LIGHT0, GL_POSITION, gvLightPosition);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    GL_ASSERT_NO_ERROR;
}

        // initTextures
        //
        // Description:
        //      Load the NVIDIA logo texture and initialize the end-clip texture.
        //          The "end-clip texture" (ghAlphaTexture) is used to correctly 
        //      clip of the ends of the bars/slats. It's a 1D texture with 1 in the
        //      center and 0 at both end pixels.
        //   
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
void initTextures()
{
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    
    glGenTextures(1, &ghTexture);
    glBindTexture(GL_TEXTURE_2D, ghTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    string sFilename = media.get_file(("textures/2D/nvlogo.dds"));
    
    nv_dds::CDDSImage oImage;
    oImage.load(sFilename, true);
    oImage.upload_texture2D();
    GL_ASSERT_NO_ERROR;
    
    glGenTextures(1, &ghAlphaTexture);
    glBindTexture(GL_TEXTURE_1D, ghAlphaTexture);
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    GLuint oMap[258];
    for (int i = 0; i<256; i++)
        oMap[i] = 0xFFFFFFFF; // red = green = blue = alpha = 1.0
        
    oMap[0]   = 0x00000000;   // red = green = blue = alpha = 0.0
    oMap[255] = 0x00000000;   // red = green = blue = alpha = 0.0
    
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, oMap);
    GL_ASSERT_NO_ERROR;
}

        // initSliders
        //
        // Description:
        //      Adds the different sliders to the slider array.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
initSliders()
{
    gSliders.AddParam(&gSlats);
    gSliders.AddParam(&gSlatHeight);
    gSliders.AddParam(&gSlatWidth);
    gSliders.AddParam(&gSlatOffset);

    GL_ASSERT_NO_ERROR;
}

        // Forward declaration
        void 
menu(int nKey);

        // initMenu
        //
        // Description:
        //      Register the menu entries with the menu.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
initMenu()
{
    glutCreateMenu(menu);
    
    glutAddMenuEntry("Display sliders [h]",         'h');
    glutAddMenuEntry("Alpha rendering order [a]",   'a');
    glutAddMenuEntry("Display frame [f]",           'f');
    glutAddMenuEntry("Display sphere [s]",          's');
    glutAddMenuEntry("Shades first [r]",            'r');
    glutAddMenuEntry("Alpha shader [1]",            '1');
    glutAddMenuEntry("Discard shader [2]",          '2');
    glutAddMenuEntry("Self shadowing shades [3]",   '3');
    glutAddMenuEntry("Anti aliased shades [4]",     '4');
    glutAddMenuEntry("Textured shades [5]",         '5');
    glutAddMenuEntry("Clipped ends shades [6]",     '6');
    glutAddMenuEntry("Quit [q]",                    'q');
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

        // renderSphere
        //
        // Description:
        //      Render a sphere that intersect the shades.
        //          This demonstrates some of the visual artifacts
        //      caused by alpha rendering order and incorrect z-values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
renderSphere()
{
    GLfloat material[] = {0.8, 0.2, 0.2, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material);

    glutSolidSphere(0.5, 100, 100);

    GL_ASSERT_NO_ERROR;
}
        

        // renderFrame
        //
        // Description:
        //      Render a "window" frame around the shades.
        //
        // Parameters:
        //      nDepth - depth of the frame
        //      nWidth - width of the frame
        //      nOffset - offset the frame in z-direction.
        //      nSegments - subdivide the frames edges.
        //
        // Returns:
        //      None
        //
        void
renderFrame(float nWidth, float nDepth, float nOffset)
{
    GLfloat material[] = {0.8, 0.2, 0.8, 1.0};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material);

    glBegin(GL_QUADS);
                    // left bar
        glNormal3f(0, 0, 1);
        glVertex3f(-1, -1, nOffset);
        glVertex3f(-1,  1, nOffset);
        glVertex3f(-1 - nWidth,  1 + nWidth, nOffset);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset);
        
        glNormal3f(1, 0, 0);
        glVertex3f(-1, -1, nOffset);
        glVertex3f(-1, -1, nOffset - nDepth);
        glVertex3f(-1,  1, nOffset - nDepth);
        glVertex3f(-1,  1, nOffset);
        
        glNormal3f(0, 0, -1);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f(-1 - nWidth,  1 + nWidth, nOffset - nDepth);
        glVertex3f(-1,  1, nOffset - nDepth);
        glVertex3f(-1, -1, nOffset - nDepth);
        
        glNormal3f(-1, 0, 0);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset);
        glVertex3f(-1 - nWidth, 1 + nWidth, nOffset);
        glVertex3f(-1 - nWidth, 1 + nWidth, nOffset - nDepth);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset - nDepth);
        
                        // lower bar
        glNormal3f(0, 0, 1);
        glVertex3f(-1, -1, nOffset);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset);
        glVertex3f( 1 , -1, nOffset);
        
        glNormal3f(0, 1, 0);
        glVertex3f(-1, -1, nOffset);
        glVertex3f( 1, -1, nOffset);
        glVertex3f( 1, -1, nOffset - nDepth);
        glVertex3f(-1, -1, nOffset - nDepth);
        
        glNormal3f(0, 0, -1);
        glVertex3f( 1 , -1, nOffset - nDepth);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f(-1, -1, nOffset - nDepth);
        
        glNormal3f(0, -1, 0);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset);
        glVertex3f(-1 - nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset);
        
        
                        // right bar
        glNormal3f(0, 0, 1);
        glVertex3f( 1, -1, nOffset);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset);
        glVertex3f( 1 + nWidth,  1 + nWidth, nOffset);
        glVertex3f( 1 ,  1, nOffset);
        
        glNormal3f(-1, 0, 0);
        glVertex3f( 1, -1, nOffset);
        glVertex3f( 1,  1, nOffset);
        glVertex3f( 1,  1, nOffset - nDepth);
        glVertex3f( 1, -1, nOffset - nDepth);
        
        glNormal3f(0, 0, -1);
        glVertex3f( 1 ,  1, nOffset - nDepth);
        glVertex3f( 1 + nWidth,  1 + nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f( 1, -1, nOffset - nDepth);
         
        glNormal3f(1, 0, 0);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset);
        glVertex3f( 1 + nWidth, -1 - nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth,  1 + nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth,  1 + nWidth, nOffset);
        
        
                        // top bar
        glNormal3f(0, 0, 1);
        glVertex3f( 1, 1, nOffset);
        glVertex3f( 1 + nWidth, 1 + nWidth, nOffset);
        glVertex3f( -1 - nWidth,  1 + nWidth, nOffset);
        glVertex3f( -1 ,  1, nOffset);
        
        glNormal3f(0, -1, 0);
        glVertex3f( 1, 1, nOffset);
        glVertex3f( -1, 1, nOffset);
        glVertex3f( -1, 1, nOffset - nDepth);
        glVertex3f( 1, 1, nOffset - nDepth);
        
        glNormal3f(0, 0, -1);
        glVertex3f( -1 ,  1, nOffset - nDepth);
        glVertex3f( -1 - nWidth,  1 + nWidth, nOffset - nDepth);
        glVertex3f( 1 + nWidth, 1 + nWidth, nOffset - nDepth);
        glVertex3f( 1, 1, nOffset - nDepth);
         
        glNormal3f(0, 1, 0);
        glVertex3f( 1 + nWidth, 1 + nWidth, nOffset);
        glVertex3f( 1 + nWidth, 1 + nWidth, nOffset - nDepth);
        glVertex3f( -1 - nWidth,  1 + nWidth, nOffset - nDepth);
        glVertex3f( -1 - nWidth,  1 + nWidth, nOffset);
    glEnd();

    GL_ASSERT_NO_ERROR;
}

        // renderShades
        //
        // Description:
        //      Activate the shade shader and render a quad from (-1, -1) to (1, 1).
        //
        // Parameteres:
        //      None
        //
        // Returns:
        //      None
        //
        void 
renderShades(float nSlats, float nWidth, float nHeight, float nOffset)
{
    cgGLSetStateMatrixParameter(hModelViewProjection, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(hInverseModelView, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE);

    cgGLEnableProfile(hFragmentProfile);
    cgGLEnableProfile(hVertexProfile);

    cgGLSetParameter1f(hSpacing, 1.0f - nWidth);
    cgGLSetParameter1f(hHeight, nHeight);
    cgGLSetParameter1f(hOffset, nOffset);
    
    float nLength = sqrt(nOffset*nOffset + nHeight * nHeight);
    cgGLSetParameter3f(hSlatNormal, nHeight/nLength, 0, nOffset/nLength);
    
    cgGLSetParameter2f(hScaleTexture, nSlats, 1.0f);

    
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2f(nSlats, 0.0f);
		glVertex3f( 1.0f, -1.0f, 0.0f);
		glTexCoord2f(nSlats, 1.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
	glEnd();
    
    cgGLDisableProfile(hFragmentProfile);
    cgGLDisableProfile(hVertexProfile);
    
    GL_ASSERT_NO_ERROR;
}

        // display
        //
        // Description:
        //      GLUT display method.
        //          Depending on render settings renders the frame,
        //      shades, sphere, with either frame first or last.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void 
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (gbSlatsLast)
    {
        if (gbDrawFrame)
            renderFrame(0.2f, 2*gSlatHeight.GetFloatValue()/gSlats.GetFloatValue() + 0.2f, 0.1f);
        if (gbDrawSphere)
            renderSphere();
    }
    
    renderShades(   gSlats.GetFloatValue(), 
                    gSlatWidth.GetFloatValue(), 
                    gSlatHeight.GetFloatValue(),
                    gSlatOffset.GetFloatValue());
    
    if (!gbSlatsLast)
    {
        if (gbDrawFrame)
            renderFrame(0.2f, 2*gSlatHeight.GetFloatValue()/gSlats.GetFloatValue() + 0.2f, 0.1f);
        if (gbDrawSphere)
            renderSphere();
    }
    
    if (gbSliders)
        gSliders.Render(0,0);

	glutSwapBuffers();
}

        // reshape
        //
        // Description:
        //      GLUT reshape callback.
        // 
        // Parameters:
        //      nWidth  - new window width.
        //      nHeight - new window height.
        //
        // Returns:
        //      None
        //
        void 
reshape(int nWidth, int nHeight)
{
    glViewport(0, 0, (GLsizei) nWidth, (GLsizei) nHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat) nWidth/(GLfloat) nHeight, 1.0f, 20.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -gnCameraDistance);
    glMultMatrixf(mCameraRotation);
    display();
}


        // menu
        //
        // Description:
        //      Handle menu events.
        //
        // Parameters:
        //      nKey - the event number created by the menu. The algorithm
        //          uses the same events as the keys used for shortcuts.
        //
        // Returns:
        //      None
        //
        void 
menu(int nKey)
{
    switch (nKey)
    {
        case 'q':
        case 27:
            exit(0);
        break;
        
        case 'w':
        case 'W':
            gbWireFrame = !gbWireFrame;
            if (gbWireFrame)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
        
        case 'a':
        case 'A':
            gbAlphaBlend = !gbAlphaBlend;
            if (gbAlphaBlend)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
        break;
        
        case 'h':
        case 'H':
            gbSliders = !gbSliders;
        break;
        
        case 'f':
        case 'F':
            gbDrawFrame = !gbDrawFrame;
        break;
        
        case 's':
        case 'S':
            gbDrawSphere = !gbDrawSphere;
        break;
        
        case 'r':
        case 'R':
            gbSlatsLast = !gbSlatsLast;
        break;
        
        case '1':
            bindFragmentProgram(hAlphaTestShades);
        break;
        
        case '2':
            bindFragmentProgram(hDiscardFragmentShades);
        break;
        case '3':
            bindFragmentProgram(hSelfShadowingShades);
        break;
        
        case '4':
            bindFragmentProgram(hAntiAliasedShades);
        break;
        
        case '5':
            bindFragmentProgram(hTexturedShades);
        break;
        
        case '6':
            bindFragmentProgram(hClippedEndsShades);
        break;
        
        default:
        break;
    }
    
    glutPostRedisplay();
}

        // keyboard
        //
        // Description:
        //      GLUT keyboard callback.
        //          Forwards key-pressed events to the menu callback.
        //
        // Parameters:
        //      nKey - the key.
        //      nX   - cursor's x-position when key was hit.
        //      nY   - cursor's y-position when hey was hit.
        //
        // Returns:
        //      None
        //
        void 
keyboard(unsigned char nKey, int nX, int nY)
{
    menu(nKey);
}

        // mouse
        //
        // Description:
        //      Mouse-button event GLUT callback.
        //
        // Parameters:
        //      nButton - the button pressed or released.
        //      nState  - the new state of the button.
        //      nX      - cursor's x-position when button was pressed.
        //      nY      - cursor's y-position when button was pressed.
        //
        // Returns:
        //      None
        //
        void 
mouse(int nButton, int nState, int nX, int nY)
{
    if (gbSliders)
        gSliders.Mouse(nX, nY);
    else
    {
        if (GLUT_LEFT_BUTTON == nButton)
            if (GLUT_DOWN == nState)
            {   
                gbLeftMouseButtonPressed = true;
                gnMouseOldX = nX;
                gnMouseOldY = nY; 
            }
            if (GLUT_UP == nState)
            {
                gbLeftMouseButtonPressed = false;
                gnMouseOldX = 0;
                gnMouseOldY = 0;             
            }
        if (GLUT_MIDDLE_BUTTON == nButton)
            if (GLUT_DOWN == nState)
            {   
                gbMiddleMouseButtonPressed = true;
                gnMouseOldX = nX;
                gnMouseOldY = nY; 
            }
            if (GLUT_UP == nState)
            {
                gbMiddleMouseButtonPressed = false;
                gnMouseOldX = 0;
                gnMouseOldY = 0;             
            }
            
    }
    glutPostRedisplay();
}

        // mouseMove
        //
        // Description:
        //      GLUT mouse-move callback.
        //
        // Parameters:
        //      nX - cursor's new x-position.
        //      nY - cursor's new y-position.
        //
        // Returns:
        //      None
        //
        void 
mouseMove(int nX, int nY)
{
    if (gbSliders)
        gSliders.Motion(nX, nY);
    else
    {
        float nDeltaX = nX - gnMouseOldX;
        float nDeltaY = nY - gnMouseOldY;
        if (gbLeftMouseButtonPressed)
        {
                                    // Calculate the new camera rotation
                                    // (ab)using GL's model-view matrix stack.
            glLoadIdentity();
            glRotatef(sqrt((double) nDeltaX*nDeltaX + nDeltaY*nDeltaY), 
                    1000 * nDeltaY,
                    1000 * nDeltaX,
                    gnMouseOldX*nY - gnMouseOldY*nX);
            glMultMatrixf(mCameraRotation);
            glGetFloatv(GL_MODELVIEW_MATRIX, mCameraRotation);

            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -gnCameraDistance);
            glMultMatrixf(mCameraRotation);
        }
        if (gbMiddleMouseButtonPressed)
        {
                                    // Calculate the new camera rotation
                                    // (ab)using GL's model-view matrix stack.
            gnCameraDistance += 0.1 * nDeltaX;
                                    
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -gnCameraDistance);
            glMultMatrixf(mCameraRotation);
        }
        gnMouseOldX = nX;
        gnMouseOldY = nY;
    }
    glutPostRedisplay();
}

        // special
        //
        // Description:
        //      GLUT special-key (eg arrows, page-up/down) callback.
        //          The sliders library requires special key handling
        //      to navigate through the sliders.
        //
        // Parameters:
        //      nKey - special key.
        //      nX   - cursor's x-position when special key was hit.
        //      nY   - cursor's y-position when special key was hit.
        //
        // Returns:
        //      None
        //
        void
special(int nKey, int nX, int nY)
{
    if (gbSliders)
        gSliders.Special(nKey, nX, nY);
}

        // cgErrorCallback
        //
        // Description:
        //      Cg error handler.
        //          This function needs to be registered with the Cg runtime
        //      in order to handle Cg errors. The function queries for the error
        //      error string, prints a meaningfull message to stderr and exits
        //      the application with exit code -1.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
cgErrorCallback(void)
{
    CGerror hError = cgGetError();

    const char * zError = cgGetErrorString(hError);

    if(hError)
    {
        std::cerr << "---------------------------------------------------------\n"
                  << zError << "\n in \n\n"
                  << "---------------------------------------------------------\n";
        
        quitapp(-1);
    }
}

        // main
        //
        // Description:
        //      Application's main function.
        //          Initializes all modules and hands over application controll
        //      to the GLUT main loop.
        //
        // Parameters:
        //      Command line args (not used).
        //
        // Returns:
        //      -1 - if application fails,
        //      0  - on clean exit.
        //
        int 
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(200, 200);
    glutCreateWindow(argv[0]);
    
    cgSetErrorCallback(cgErrorCallback);

    initGL();
    initCG();
    initSliders();    
    initTextures();

    bindVertexProgram(hVertexProgram);
    bindFragmentProgram(hAlphaTestShades);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    
    initMenu();

    glutMainLoop();

    quitapp(0);

    return 0;
}
