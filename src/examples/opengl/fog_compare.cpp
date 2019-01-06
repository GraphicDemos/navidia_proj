#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <string>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <shared/quitapp.h>

#include "TesselatedQuad.hpp"

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

unsigned int iFogMode;

string title_l = "depth fog";
string title_r = "radial fog";

string fog_titles[3] = {"linear", "exp", "exp^2"};
GLenum fog_modes[] = {GL_LINEAR, GL_EXP, GL_EXP2};

bool b[256];

// glut-ish callbacks
static void display();
static void Idle();
static void Menu(int v);
static void Key(unsigned char k, int x, int y);

float rParameter = 0;

TesselatedQuad *pQuad;

// my functions
void InitOpengl();

//.----------------------------------------------------------------------------.
//|   Function   : main                                                        |
//|   Description: setup and start displayloop.                                |
//.----------------------------------------------------------------------------.
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize( 512, 256 );
    glutCreateWindow("Fog Comparison");
    
    InitOpengl();

    pQuad = new TesselatedQuad(16);
    
    glut_helpers_initialize();
    
    cb.keyboard_function = Key;
    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    object.configure_buttons(1);
    object.dolly.dolly[2] = -1.5;
    reshaper.aspect_factor = 0.5f;
    
    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glut_add_interactor(&reshaper);
    
    glutCreateMenu( Menu );
    glutAddMenuEntry( "toggle wireframe [w]",					'w' );
    glutAddMenuEntry( "cycle fog mode [f]",				        'f' );
    glutAttachMenu( GLUT_RIGHT_BUTTON );

    glutDisplayFunc(display);
    glutIdleFunc(Idle);
    glutMainLoop();
    return 0;
}


//.----------------------------------------------------------------------------.
//|   Function   : InitOpengl                                                  |
//|   Description: Initialize extensions, setup GL settings.                   |
//.----------------------------------------------------------------------------.
void InitOpengl()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glClearColor(1, 1, 1, 1);
   
    float fogColor[] = { 1, 1, 1, 1 };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.4f);
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 1.5);
    glFogf(GL_FOG_END, 4);
    
    // The hardware must support the GL_NV_fog_distance extension in order to use radial distance fog.
    if (!glh_init_extensions("GL_NV_fog_distance"))
    {
        printf("Demo requires the following extension(s): %s\n", glh_get_unsupported_extensions());
        quitapp(0);  
    }
}


//.----------------------------------------------------------------------------.
//|   Function   : Menu                                                        |
//|   Description: Handle Keys and menu selections.                            |
//.----------------------------------------------------------------------------.
static void Menu( int v )
{
    b[v] = ! b[v];
    if (v == 27 || v == 'q') exit(0);
    if (v == 'w')
    {
        if (b[v])
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (v == 'f')
    {
        iFogMode = (iFogMode + 1) % 3;
        glFogf(GL_FOG_MODE, fog_modes[iFogMode]);
    }
    glutPostRedisplay();
}


//.----------------------------------------------------------------------------.
//|   Function   : Key                                                         |
//|   Description: Handle keys.  Passes work on to Menu().                     |
//.----------------------------------------------------------------------------.
void Key(unsigned char k, int x, int y)
{
    Menu((int) k);    
}


//.----------------------------------------------------------------------------.
//|   Function   : Idle                                                        |
//|   Description: Do animation.                                               |
//.----------------------------------------------------------------------------.
void Idle()
{
    static float rValue = 0;
    if (!b[' '])
    {
        rValue += 0.002f;
        rParameter = 1 + fabs(sin(rValue));
        
        glutPostRedisplay();
    }
}



//.----------------------------------------------------------------------------.
//|   Function   : DisplaySide                                                 |
//|   Description: Display a bunch of blocks on a quad.  The quad is           |
//|                tesselated since fog is computed per vertex.  This makes the|
//|                radial fog more apparent.                                   |
//.----------------------------------------------------------------------------.
void DisplaySide()
{
    
    int iNumAcross  =  10;
    int iNumDeep    =  10;
    float rCubeSize =  .25;
    float rLeftSide = -0.5f * ((1.6 * iNumAcross - 1) * rCubeSize) + 0.5f * rCubeSize;
    float rNearPos  =  0.5f * ((1.6 * iNumDeep   - 1) * rCubeSize) - 0.5f * rCubeSize;
    
    glColor3f(0, 0.2f, 0.9f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(rLeftSide, rNearPos, 0);
    for (int i = 0; i < iNumAcross; i++)
    {
        for (int j = 0; j < iNumDeep; j++)
        {
            float rScale = rParameter * 2;
            glScalef(1, 1, rScale);
            glutSolidCube(rCubeSize);
            glScalef(1, 1, 1 / rScale);
            glTranslatef(1.6 * rCubeSize, 0, 0);
        }
        glTranslatef(-1.6 * iNumDeep * rCubeSize, -1.6 * rCubeSize, 0);
    }
    glPopMatrix();
    glPushMatrix();
    
    
    glRotatef(90, 1, 0, 0);
    glScalef(5, 1, 5);
    glTranslatef(0, -rCubeSize + 0.01f, 0);
    glColor3f(0, 0, 0);
    pQuad->Display();
    glPopMatrix();
}


//.----------------------------------------------------------------------------.
//|   Function   : DisplayScene                                                |
//|   Description: Display 6 sides of "the cube".                              |
//.----------------------------------------------------------------------------.
void DisplayScene()
{

    glPushMatrix();

    for (int i = 0; i < 6; i++)
    {
        glPushMatrix();
            glTranslatef(0, 0, -2.5);
            DisplaySide();
        glPopMatrix();
        if (i < 3)
            glRotatef(-90, 1, 0, 0);
        else if (i == 3)
            glRotatef(-90, 0, 1, 0);
        else
            glRotatef(180, 0, 1, 0);
    }
    
    glPopMatrix();
}


//.----------------------------------------------------------------------------.
//|   Function   : DisplayLeft                                                 |
//|   Description: Display the left viewport with depth-based fog.             |
//.----------------------------------------------------------------------------.
void DisplayLeft()
{
    glPushMatrix();
    camera.apply_inverse_transform();
    object.apply_transform();
    
    // This is the key: setting the distance mode controls whether the fog is computed based on the 
    // eye-space Z distance, as is the case when GL_EYE_PLANE_ABSOLUTE_NV is used, or based on the 
    // actual distance from the eye (radial distance), as is the case when GL_EYE_RADIAL_NV is used.
    glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);
    glDisable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    
    DisplayScene();
 
    glDisable(GL_LIGHTING);
    
    glDisable(GL_FOG);
    glPopMatrix();
}

// right
void DisplayRight()
{
    glPushMatrix();
    camera.apply_inverse_transform();
    object.apply_transform();
    
    // This is the key: setting the distance mode controls whether the fog is computed based on the 
    // eye-space Z distance, as is the case when GL_EYE_PLANE_ABSOLUTE_NV is used, or based on the 
    // actual distance from the eye (radial distance), as is the case when GL_EYE_RADIAL_NV is used.
    glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);

    DisplayScene();

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    
    glPopMatrix();
}



//.----------------------------------------------------------------------------.
//|   Function   : RenderString                                                |
//|   Description: Render a bitmap string at position (x, y) in the viewport.  |
//.----------------------------------------------------------------------------.
void RenderString(int x, int y, const char * str)
{
    int WW = glutGet((GLenum)GLUT_WINDOW_WIDTH);
    int WH = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WW-1, 0, WH-1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i(x,y);
    while(*str)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);
        str++;
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}


//.----------------------------------------------------------------------------.
//|   Function   : Display                                                     |
//|   Description: Main display function (GLUT callback).                      |
//.----------------------------------------------------------------------------.
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    
    GLint w = vp[2] / 2;
    GLint h = vp[3];
    
    std::string title = title_l;
    title.append(" : ");
    title.append(fog_titles[iFogMode]);

    // left viewport
    glViewport(0,0, w, h);
    DisplayLeft();

    glColor3f(0.1, 0.3, 0.5);
    RenderString(25, 25, title.c_str());
    
    title = title_r;
    title.append(" : ");
    title.append(fog_titles[iFogMode]);

    // right viewport
    glViewport(w,0, w, h);
    DisplayRight();
    glColor3f(0.1, 0.3, 0.5);
    RenderString(25, 25, title.c_str());
    
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    
    glutSwapBuffers();
}
