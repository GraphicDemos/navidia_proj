// -------------------------------------------------------------------
//
// Contents:
//      Scotopic demo main program
//
// Description:
//      Scotopic demo shows the creation of a little image processing
//      network based on the nv_image_processing framework. 
//          
// Author:
//      Frank Jargstorff (2003)
//
// --------------------------------------------------------------------

//
// Includes
//

#include <windows.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <ShaderManager.h>
#include <LoadOperator.h>
#include <SaveOperator.h>
#include <ImageView.h>
#include <GaussFilter.h>
#include <GaussFilter1D.h>
#include <TwoPassGaussFilter.h>
#include <AssertGL.h>


#include "NightFilter.h"
#include "ScotopicFilter.h"
#include "InteractionController.h"


#include <iostream>
#include <string>

#include <math.h>

#include <shared/quitapp.h>

 

//
// Defines
//

#define INITIAL_WINDOW_WIDTH   1000
#define INITIAL_WINDOW_HEIGHT  800

#define DEFAULT_IMAGE_FILE "../../../../MEDIA/Textures/2D/TestImage.dds"


//
// Global data
//

unsigned int    gnWindowWidth;
unsigned int    gnWindowHeight;

int  gnOldX;
int  gnOldY;
bool gbMoving;
bool gbScaling;
bool gbSliders;
bool gbFullScreen;
bool gbUseFBO = true;


LoadOperator        * gpLoadOperator;
NightFilter         * gpNightFilter;
GaussFilter         * gpGaussFilter;
ScotopicFilter      * gpScotopicFilter;
GaussFilter1D       * gpGaussFilter1D;
TwoPassGaussFilter  * gpTwoPassGaussFilter;
SaveOperator        * gpSaveOperator;
ImageView           * gpView;

InteractionController * gpInteractionController;


//
// Function declarations
//

        // display
        //
        void 
display();

        // reshape
        //
        void
reshape(int nWidth, int nHeight);

        // menu
        //
        void 
menu(int nCommand);

        // mouse
        //
        void
mouse(int nButton, int nState, int nX, int nY);

        // mouseMove
        //
        void
mouseMove(int nX, int nY);

        // key
        //
        void 
key(unsigned char chKeyPressed, int nPositionX, int nPositionY);

        // special 
        //
        void
special(int nKey, int nX, int nY);


// -----------------------------------------------------------------------------
// Implementation
//

#define REQUIRED_EXTENSIONS "GL_VERSION_1_4 " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GL_ARB_shader_objects " \
                            "GL_ARB_vertex_shader " \
                            "GL_ARB_fragment_shader " \
                            "GL_ARB_vertex_program " \
                            "GL_ARB_fragment_program " \
                            "GL_ARB_vertex_buffer_object " \
                            "GL_ARB_multitexture " \
                            "GL_ARB_texture_compression " \
                            "GL_EXT_texture_compression_s3tc " \
                            "GL_NV_texture_rectangle " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GL_NV_float_buffer " \
                            "WGL_ARB_extensions_string " \
                            "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format " \
                            "WGL_ARB_render_texture "

#define OPTIONAL_EXTENSIONS "GL_EXT_framebuffer_object "

        // main
        //
        int
main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    GL_ASSERT_NO_ERROR;

    gnWindowWidth  = INITIAL_WINDOW_WIDTH;
    gnWindowHeight = INITIAL_WINDOW_HEIGHT;

    glutInitWindowSize(gnWindowWidth, gnWindowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Scotopic Vision Demo");
    GL_ASSERT_NO_ERROR;

    gbMoving     = false;
    gbScaling    = false;
    gbSliders    = false;
    gbFullScreen = false;

	glewInit();
	gbUseFBO = false;

    HDC hDC = wglGetCurrentDC();
    GL_ASSERT_NO_ERROR;

    ShaderManager::initialize();
    GL_ASSERT_NO_ERROR;

    gpLoadOperator = new LoadOperator;
    gpLoadOperator->setFileName(DEFAULT_IMAGE_FILE);
    GL_ASSERT_NO_ERROR;

    for (int iArgument = 1; iArgument < argc; ++iArgument)
    {
        if (!strcmp(argv[iArgument], "-h"))
        {
            std::cout << "Usage: " << "cg_scotopic [-h, -f] [Filename.jpg]\n"
                      << "     -h: prints this help screen\n"
                      << "     -f: starts the program in full-screen mode"
                      << "\n"
                      << "     Filename.dds is the input image. If no input image\n"
                      << "     is specified the program tries to load a default\n"
                      << "     example." << std::endl;
            quitapp(0);
        }
        else if (!strcmp(argv[iArgument], "-f"))
            gbFullScreen = true;
        else
        {
            gpLoadOperator->setFileName(argv[iArgument]);   
            break;
        }
    }

    gpNightFilter           = new NightFilter;
    GL_ASSERT_NO_ERROR;
    gpGaussFilter           = new GaussFilter;
    GL_ASSERT_NO_ERROR;
    gpScotopicFilter        = new ScotopicFilter;
    GL_ASSERT_NO_ERROR;
    
    gpGaussFilter1D         = new GaussFilter1D;
    GL_ASSERT_NO_ERROR;
    gpTwoPassGaussFilter    = new TwoPassGaussFilter;
    GL_ASSERT_NO_ERROR;

    gpSaveOperator          = new SaveOperator;
    GL_ASSERT_NO_ERROR;
    gpView                  = new ImageView;
    GL_ASSERT_NO_ERROR;

    gpInteractionController = new InteractionController(*gpLoadOperator,
                                                        *gpGaussFilter,
                                                        *gpNightFilter,
                                                        *gpScotopicFilter,
                                                        *gpGaussFilter1D,
                                                        *gpTwoPassGaussFilter,
                                                        *gpSaveOperator,
                                                        *gpView);

    GL_ASSERT_NO_ERROR;
                                // Register menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Quit\t[q]",                   'q');
    glutAddMenuEntry("Display\t[d]",                'd');
    glutAddMenuEntry("Gauss filter\t[g]",           'g');
    glutAddMenuEntry("Night filter\t[n]",           'n');
    glutAddMenuEntry("Scotopic filter\t[o]",        'o');
    glutAddMenuEntry("1D Gauss filter\t[1]",        '1');
    glutAddMenuEntry("Toggle direction\t[t]",       't');
    glutAddMenuEntry("2-pass Gauss filter \t[2]",   '2');
    glutAddMenuEntry("Display Sliders\t[h]",        'h');
    glutAddMenuEntry("Save file Scotopic.dds\t[f]", 'f');
    glutAttachMenu( GLUT_RIGHT_BUTTON );

                                // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);

    if (gbFullScreen)
        glutFullScreen();

    glutMainLoop();

    return 0;
}


        // display
        //
        void 
display()
{
    gpView->display();
    if (gbSliders)
        gpInteractionController->renderSliders();
        
    glutSwapBuffers();
}

        // reshape
        //
        void
reshape(int nWidth, int nHeight)
{
    gnWindowWidth  = nWidth;
    gnWindowHeight = nHeight;

    glViewport(0, 0, (GLsizei) gnWindowWidth, (GLsizei) gnWindowHeight);
    gpView->reshape(gnWindowWidth, gnWindowHeight);

    display();
}

        // menu
        //
        void 
menu(int nCommand)
{
    switch (nCommand)
    {
        case 27:
        case 'q':
            exit(0);
        break;
        case 'd':
            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
        break;
        case 'g':
            gpInteractionController->setPipelineMode(InteractionController::GAUSS_FILTER_MODE);
        break;
        case 'n':
            gpInteractionController->setPipelineMode(InteractionController::NIGHT_FILTER_MODE);
        break;
        case 'o':
            gpInteractionController->setPipelineMode(InteractionController::SCOTOPIC_FILTER_MODE);
        break;    
        case '1':
            gpInteractionController->setPipelineMode(InteractionController::GAUSS_1D_FILTER_MODE);
        break;
        case 't':
            if (gpGaussFilter1D->orientation() == GaussFilter1D::VERTICAL_FILTER)
                gpGaussFilter1D->setOrientation(GaussFilter1D::HORIZONTAL_FILTER);
            else
                gpGaussFilter1D->setOrientation(GaussFilter1D::VERTICAL_FILTER);
        break;
               
        case '2':
            gpInteractionController->setPipelineMode(InteractionController::TWO_PASS_GAUSS_FILTER_MODE);
        break;
        
        case 'h':
            gbSliders = !gbSliders;
        break;
 
        case 'f':
        {
            gpInteractionController->save();
        }

        default:
        break;
    }

    glutPostRedisplay();
}
        
        // key
        //
        void 
key(unsigned char chKeyPressed, int nPositionX, int nPositionY)
{
                                // Forward the key-pressed event to be
                                // handled by the menu callback.
    menu((int)chKeyPressed);
}

        // mouse
        //
        void
mouse(int nButton, int nState, int nX, int nY)
{
    if (gbSliders)
        gpInteractionController->mouse(nX, nY);
    else
    {
        switch (nButton)
        {
            case GLUT_LEFT_BUTTON:
            {
                if (GLUT_DOWN == nState)
                {
                    gnOldX   = nX;
                    gnOldY   = nY;
                    gbMoving = true;
                }   
                else 
                {
                    gbMoving = false;
                }
            }
            break;

            case GLUT_MIDDLE_BUTTON:
            {
                if (GLUT_DOWN == nState)
                {
                    gnOldX = nX;
                    gnOldY = nY;
                    gbScaling = true;
                }
                else
                {
                    gbScaling = false;
                }
            }
            break;

            case GLUT_RIGHT_BUTTON:
            {

            }
            break;
        }
    }
    

    glutPostRedisplay();
}

        // mouseMove
        //
        void
mouseMove(int nX, int nY)
{
    if (gbSliders)
        gpInteractionController->move(nX, nY);
    else
    {    
        if (gbMoving)
            gpView->setImagePosition(gpView->imagePositionX() + nX - gnOldX,
                                    gpView->imagePositionY() + gnOldY - nY);

        if (gbScaling)
            gpView->setZoomFactor(max(0, (gpView->zoomFactor() + 
                                ((nX - gnOldX) + (gnOldY - nY))/100.0f)));
            
        gnOldX = nX;
        gnOldY = nY;
    }
    glutPostRedisplay();
}

        // special 
        //
        void
special(int nKey, int nX, int nY)
{
    if (gbSliders)
        gpInteractionController->special(nKey, nX, nY);
}
