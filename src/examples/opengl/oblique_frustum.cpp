#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <iostream>
#include <list>
#include <vector>

#include <glh/glh_glut.h>

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object, plane;
glut_perspective_reshaper reshaper;

bool b[256];

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void idle();

void menu(int entry) { key(entry, 0, 0); }

// my functions
void init_opengl();

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("Oblique Frustum Demo");

    init_opengl();

    glut_helpers_initialize();

    cb.keyboard_function = key;
    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    object.configure_buttons(1);
    object.dolly.dolly[2] = -3;
    plane.configure_buttons(1);
    plane.dolly.dolly[2] = -2.5;

    camera.disable();
    object.disable();
    plane.enable();

    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glut_add_interactor(&plane);
    glut_add_interactor(&reshaper);

    glutCreateMenu(menu);
    glutAddMenuEntry("Toggle clipping [c]", 'c');
    glutAddMenuEntry("Use oblique clip (red background) [o]", 'o');
    glutAddMenuEntry("Manipulate camera [1]", '1');
    glutAddMenuEntry("Manipulate teapot [2]", '2');
    glutAddMenuEntry("Manipulate plane [3]", '3');
    glutAddMenuEntry("Quit [q]", 'q');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // initialize the toggles.
    b['c'] = b['o'] = true;
    glClearColor(.3,0,0,0);

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if(k==27 || k=='q' || k=='Q') {
        exit(0);
    }

    if('1'==k)
    {
        camera.enable();
        object.disable();
        plane.disable();
    }
    if('2'==k)
    {
        camera.disable();
        object.enable();
        plane.disable();
    }
    if('3'==k)
    {
        camera.disable();
        object.disable();
        plane.enable();
    }

    if(b['o'])
        glClearColor(.3,0,0,0);
    else
        glClearColor(0,0,.3,0);

    if(!b['c'])
        glClearColor(0,0,0,0);

    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

// here's where to add the thing to time...
void draw()
{
    glutSolidTeapot(1);
}

// draw with a slight offset so the clip plane doesn't clip our
// visual aid...
void draw_plane()
{
    float offset = -0.0001;

    glDisable(GL_LIGHTING);
    glColor4f(.6, .6, .1, .25);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-1, -1, offset);
    glVertex3f( 1, -1, offset);
    glVertex3f( 1,  1, offset);
    glVertex3f(-1,  1, offset);
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    glVertex3f(-1, -1, offset);
    glVertex3f( 1, -1, offset);
    glVertex3f( 1,  1, offset);
    glVertex3f(-1,  1, offset);
    glEnd();
    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    camera.apply_inverse_transform();

    matrix4f P, M;

    if( b['c'] ) // clip
    {
        glPushMatrix();
        plane.apply_transform();    

        if( b['o'] ) // use oblique frustum
        {
            P = get_matrix(GL_PROJECTION_MATRIX);
            M = get_matrix(GL_MODELVIEW_MATRIX);
            matrix4f invtrans_MVP = (P * M).inverse().transpose();
            vec4f oplane(0,0,-1,0);
            vec4f cplane;
            invtrans_MVP.mult_matrix_vec(oplane, cplane);
    
            cplane /= abs((int)cplane[2]); // normalize such that depth is not scaled
            cplane[3] -= 1;

            if(cplane[2] < 0)
                cplane *= -1;

            matrix4f suffix;
            suffix.set_row(2, cplane);
            matrix4f newP = suffix * P;
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(newP.m);
            glMatrixMode(GL_MODELVIEW);

        }
        else  // use user clip plane
        {
            GLdouble plane[] = { 0, 0, -1, 0 }; 
            glEnable(GL_CLIP_PLANE0);
            glClipPlane(GL_CLIP_PLANE0, plane);
        }

        glPopMatrix();
    }

    glPushMatrix();
    object.apply_transform();
    draw();
    glPopMatrix();

    glPushMatrix();
    plane.apply_transform();    
    draw_plane();
    glPopMatrix();

    glPopMatrix();

    if( b['c'] ) // clip
    {
        if( b['o'] ) // use oblique frustum
        {
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(P.m);
            glMatrixMode(GL_MODELVIEW);
        }
        else  // use user clip plane
        {
            glDisable(GL_CLIP_PLANE0);
        }
    }

    // swap then do FPS calculations
    glutSwapBuffers();
}
