// Fragment program physics demo
// http://developer.nvidia.com/docs/IO/4449/SUPP/GDC2003_OpenGLShaderTricks.pdf

#if defined(WIN32)
#  include <windows.h>
#  pragma warning (disable : 4786)
#endif

#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_linear.h>

#include <nv_png.h>
#include <shared/array_texture.h>
#include <shared/quitapp.h>

#include "ParticleSystem.h"

using namespace glh;

ParticleSystem *psys;

// view parameters
int ox, oy;
int buttonState = 0;
float tx = 0, ty = -2, tz = -5;
float rx = 0, ry = 0;
int mode = 0;
enum { M_VIEW = 0, M_MOVE };

float last_time;

GLboolean toggle[256];
vec3f spherePos(0.0, 2.0, 0.0);

// check for OpenGL errors
void checkGLErrors(char *s)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "%s: error - %s\n", s, (char *) gluErrorString(error));
    }
}

// Draw a grid of lines
void drawWireGrid(int rows, int cols)
{
    int i;
    glLineWidth(1.0);

    glBegin(GL_LINES);
    for(i=0; i<=rows; i++) {
        glVertex2f(0.0, (float) i / rows);
        glVertex2f(1.0, (float) i / rows);
    }
    for(i=0; i<=cols; i++) {
        glVertex2f((float) i / cols, 0.0);
        glVertex2f((float) i / cols, 1.0);
    }
    glEnd();
}

void display(void)
{   
    if (toggle['a']) 
    {
        // simulate particle system
#if 0
        float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        psys->TimeStep(time - last_time);
        last_time = time;
#else
        psys->TimeStep(0.01);
#endif
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view transform
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(tx, ty, tz);
    glRotatef(rx, 1.0, 0.0, 0.0);
    glRotatef(ry, 0.0, 1.0, 0.0);

    // grid
    if (toggle['g']) {
        glColor3f(0.0, 1.0, 0.0);
        glPushMatrix();
        glRotatef(90.0, 1.0, 0.0, 0.0);
        glScalef(8.0, 8.0, 8.0);
        glTranslatef(-0.5, -0.5, 0.0);
        drawWireGrid(20, 20);
        glPopMatrix();
    }

    // draw sphere
    if (toggle['s']) {
        glPushMatrix();
        glTranslatef(spherePos[0], spherePos[1], spherePos[2]);
        glColor3f(0.0, 0.0, 1.0);
        glutSolidSphere(0.95, 30, 30);
        glPopMatrix();
    }
    psys->SetSphere(spherePos);

    // draw particle system
    if (toggle['p']) {
        psys->ReadBack();
        psys->Display();
    } else {
        psys->DisplayShaded2();
    }

    if (toggle['t'])
        psys->DisplayTextures();

    checkGLErrors("display");
    glutSwapBuffers();
}

void motion(int x, int y)
{
    float dx, dy;
    dx = x - ox;
    dy = y - oy;

    switch(mode) 
    {
        case M_VIEW:
            if (buttonState == 3) {
                // left+middle = zoom
                tz += dy / 100.0;
            } 
            else if (buttonState & 2) {
                // middle = translate
                tx += dx / 100.0;
                ty -= dy / 100.0;
            }
            else if (buttonState & 1) {
                // left = rotate
                rx += dy / 5.0;
                ry += dx / 5.0;
            }
            break;

        case M_MOVE:
            if (buttonState==1) {
                spherePos[0] += dx / 100.0;
                spherePos[1] -= dy / 100.0;
            } else if (buttonState==2) {
                spherePos[2] += dy / 100.0;
            }
            break;
    }

    ox = x; oy = y;
    glutPostRedisplay();
}


void mouse(int button, int state, int x, int y)
{
    int mods;

    if (state == GLUT_DOWN)
        buttonState |= 1<<button;
    else if (state == GLUT_UP)
        buttonState = 0;

    mods = glutGetModifiers();
    if (mods & GLUT_ACTIVE_SHIFT) {
        buttonState = 2;
    } else if (mods & GLUT_ACTIVE_CTRL) {
        buttonState = 3;
    }

    ox = x; oy = y;
    glutPostRedisplay();
}


void key(unsigned char key, int x, int y)
{
    toggle[key] ^= 1;

    switch (key) {
    case '\033':
    case 'q':
        exit(0);
        break;
    case 'w':
        glPolygonMode(GL_FRONT_AND_BACK, toggle['w'] ? GL_LINE : GL_FILL);
        break;

    case 'v':
        mode = M_VIEW;
        break;

    case 'm':
        mode = M_MOVE;
        break;

    case 'r':
        psys->InitGrid( vec3f(-2.0, 5.0, -2.0), vec3f(4.0 / psys->GetWidth(), 0.0, 0.0), vec3f(0.0, 0.0, 4.0 / psys->GetHeight()) );
        break;

    case ' ':
        psys->TimeStep(0.0);
        break;
    }

    glutPostRedisplay();
}

void idle(void)
{
    if (toggle['a'])
        glutPostRedisplay();
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) w / (float) h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void mainMenu(int i)
{
    key((unsigned char) i, 0, 0);
}

void initMenus(void)
{
    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Toggle animation [a]", 'a');
    glutAddMenuEntry("Step animation [ ]", ' ');
    glutAddMenuEntry("View mode [v]", 'v');
    glutAddMenuEntry("Move mode [m]", 'm');
    glutAddMenuEntry("Display particles [p]", 'p');
    glutAddMenuEntry("Display texture [t]", 't');
    glutAddMenuEntry("Toggle sphere [s]", 's');
    glutAddMenuEntry("Toggle grid [g]", 'g');
    glutAddMenuEntry("Reset [r]", 'r');
    glutAddMenuEntry("Quit (esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// get extension pointers
void getExts(void)
{
    if (!glh_init_extensions("GL_ARB_multitexture "
                             "GL_ARB_vertex_program "
                             "GL_ARB_shader_objects "
                             "GL_ARB_vertex_shader "
                             "GL_ARB_fragment_shader "
                             "WGL_ARB_pbuffer "
                             "WGL_ARB_pixel_format "
                             "WGL_ARB_render_texture "
                             "GL_NV_float_buffer "
                             "GL_ARB_vertex_buffer_object "
                             "GL_EXT_pixel_buffer_object ")) 
    {
        fprintf(stderr, "Error - required extensions were not supported: %s", glh_get_unsupported_extensions());
        quitapp(-1);
    }
}

void initGL()
{  
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glClearColor(0.5, 0.5, 0.5, 1.0);

    checkGLErrors("initGL");
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);
    (void) glutCreateWindow("Fragment program physics");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);

    toggle['g'] = toggle['s'] = true;
    toggle['a'] = true;

    initMenus();
    getExts();
    initGL();

    // create particle system
    psys = new ParticleSystem(32, 32, 2);
    psys->InitGrid( vec3f(-2.0, 5.0, -2.0), vec3f(4.0 / psys->GetWidth(), 0.0, 0.0), vec3f(0.0, 0.0, 4.0 / psys->GetHeight()) );

    last_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

    glutMainLoop();
    return 0;
}
