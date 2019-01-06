#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE

#if defined(WIN32)
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_ARB_vertex_program " \
                            "GL_NV_texture_shader " \
                            "GL_NV_register_combiners " \
                            "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format "
#elif defined(UNIX)
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_ARB_vertex_program " \
                            "GL_NV_texture_shader " \
                            "GL_NV_register_combiners " \
                            "GLX_SGIX_pbuffer " \
                            "GLX_SGIX_fbconfig "
#elif defined(MACOS)
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_ARB_vertex_program " \
                            "GL_NV_texture_shader " \
                            "GL_NV_register_combiners "
#endif

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <nvparse.h>
#include <shared/data_path.h>
#include <shared/quitapp.h>
#include <math.h>

#include "CA_GameOfLife.hpp"

using namespace glh;

// Global variables

bool b[256];								// Toggle state for all the keys (characters)

CA_GameOfLife *pGameOfLife;

float rAlphaRef = 0.0f;

glut_perspective_reshaper reshaper;
glut_simple_mouse_interactor object;

// glut-ish callbacks
void display();
void Key(unsigned char k, int x, int y);
void Menu( int v );
void Reshape(int w, int h);
void Idle();

// other functions
void Initialize();

//.----------------------------------------------------------------------------.
//|   Function   : main                                                        |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA );
    glutInitWindowSize(512, 512);
    glutCreateWindow("Game of Life Demo");

    b['a'] = false;
    b['w'] = false;
    b['p'] = false;
    b[' '] = true;
 
    Initialize();

    glut_helpers_initialize();
    object.configure_buttons(1);
    object.dolly.dolly[2] = -1.7;

    glut_add_interactor(& reshaper);
	glut_add_interactor(& object);

    glutCreateMenu( Menu );
    glutAddMenuEntry( "toggle Wireframe [w]",					    'w' );
    glutAddMenuEntry( "toggle animation [ ]",				        ' ' );
	glutAddMenuEntry( "single step (when paused) [s]",   		    's' );
    glutAddMenuEntry( "toggle Border wrapping [b]",		            'b' );
    glutAddMenuEntry( "Reset [r]",					                'r' );
    //glutAddMenuEntry( "toggle Alpha test in texture update [a]",    'a' );
    //glutAddMenuEntry( "cycle alpha value to use (0 or 0.125) [v])", 'v' );
    glutAddMenuEntry( "Display Neighbor Accumulation Map [1]",      '1' );
    glutAddMenuEntry( "Display Current Generation [2]", 		    '2' );
    glutAddMenuEntry( "Draw Initial, Temp, and Output Frames [3]",  '3' );
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );

    glutKeyboardFunc(Key);
    //glutReshapeFunc(Reshape);
    glutIdleFunc(Idle);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}


//.----------------------------------------------------------------------------.
//|   Function   : Initialize                                                  |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void Initialize()
{
    if (!glh_init_extensions(REQUIRED_EXTENSIONS))
    {
        printf("Demo requires the following extension(s): %s\n", glh_get_unsupported_extensions());
        quitapp(0);
    }

    glClearColor(0, 0.2f, 0.5f, 0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    gluPerspective(90, 1, .1, 10);
    
    pGameOfLife = new CA_GameOfLife;

    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA/textures");
    media.path.push_back("../../../../MEDIA/textures");

    string ruletableImg = media.get_file("2D/RuleTable.ppm");
    string startImg = media.get_file("2D/start.ppm");

    pGameOfLife->Initialize(ruletableImg.c_str(), startImg.c_str());//"fieldsmall.ppm");//"1-bit_adder.ppm");
}


//.----------------------------------------------------------------------------.
//|   Function   : Menu                                                        |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void Menu( int v )
{
    b[v] = !b[v];

    switch (v)
    {
    case 27:
    case 'q':
        exit(0);
    case 'w':
        pGameOfLife->EnableWireframe(b['w']);
        break;
    case ' ':
        pGameOfLife->EnableAnimation(b[' ']);
        break;
    case 'b':
        pGameOfLife->EnableBorderWrapping(b['b']);
        break;
    case 's':
        pGameOfLife->SingleStep();
        break;
    case '1':
        pGameOfLife->SetRenderMode(CA_GameOfLife::CA_FULLSCREEN_NEIGHBOR_CALC);
        break;
    case '2':
        pGameOfLife->SetRenderMode(CA_GameOfLife::CA_FULLSCREEN_FINALOUT);
        break;
    case '3':
        pGameOfLife->SetRenderMode(CA_GameOfLife::CA_TILED_THREE_WINDOWS);
        break;
    case 'r':
        pGameOfLife->Reset();
        break;    
    case 'i':
        gluPerspective(90, 1, .01, 10);
        break;
    /*case 'a':
        pGameOfLife->EnableAlphaTest(b['a']);
        break;
    case 'v':
        if (rAlphaRef < 0.125f)
            rAlphaRef = 0.125f;
        else
            rAlphaRef = 0;

        printf("Setting Alpha Reference Value to %f\n", rAlphaRef);
        pGameOfLife->SetAlphaReference(rAlphaRef);*/
    case 'o':
    default:
        break;
    }

    glutPostRedisplay();
}


//.----------------------------------------------------------------------------.
//|   Function   : Key                                                         |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void Key(unsigned char k, int x, int y)
{
    Menu((int)k);
}

//.----------------------------------------------------------------------------.
//|   Function   : Reshape                                                     |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void Reshape(int w, int h)
{
    glViewport(0,0,w,h);
}


//.----------------------------------------------------------------------------.
//|   Function   : Idle                                                        |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void Idle()
{
    glutPostRedisplay();
}

//.----------------------------------------------------------------------------.
//|   Function   : Display                                                     |
//|   Description:                                                             |
//.----------------------------------------------------------------------------.
void display()
{
    pGameOfLife->Tick();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	object.apply_transform();

    pGameOfLife->Display();
    
    glPopMatrix();
    glutSwapBuffers();
}
