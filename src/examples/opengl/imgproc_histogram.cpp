/*
    This example implements a simple image histogram using occlusion queries.

    For a histogram with n values, it renders the scene n times, each time
    using a fragment program that discards pixels outside the value of interest.
    An occlusion query is used each pass to count how many pixels passed the test.
*/

#if defined(WIN32)
#  include <windows.h>
#endif
#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include "histogram.h"

using namespace glh;

bool b[256];
glut_simple_mouse_interactor object;
int win_w = 512, win_h = 512;
GLuint scene_tex;
Histogram *histo;
int pass = 0;

GLuint create_texture(GLenum target, int w, int h);

void init_opengl()
{
    if(! glh_init_extensions(
        "GL_ARB_multitexture "
        "GL_ARB_vertex_program "
        "GL_ARB_fragment_program "
        "GL_NV_vertex_program "
        "GL_NV_fragment_program "
        "GL_ARB_occlusion_query "
        ))
    {
        fprintf(stderr, "Error - required extensions were not supported: %s\n", glh_get_unsupported_extensions());
    }

    glClearColor(0.0, 0.0, 0.0, 1.0);

    // initialize OpenGL lighting
    GLfloat matAmb[4] =    {1.0, 1.0, 1.0, 1.0};
    GLfloat matDiff[4] =   {1.0, 0.1, 0.2, 1.0};
    GLfloat matSpec[4] =   {1.0, 1.0, 1.0, 1.0};

    GLfloat lightPos[] =   {10.0, 10.0, 10.0, 0.0};
    GLfloat lightAmb[4] =  {0.0, 0.0, 0.0, 1.0};
    GLfloat lightDiff[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightSpec[4] = {1.0, 1.0, 1.0, 1.0};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
    GLfloat black[] =  {0.0, 0.0, 0.0, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    scene_tex = create_texture(GL_TEXTURE_RECTANGLE_NV, win_w, win_h);
}

GLuint create_texture(GLenum target, int w, int h)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    return tex;
}

void draw_quad(int w, int h)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
    glTexCoord2f(w, 0.0); glVertex2f(w, 0.0);
    glTexCoord2f(w, h); glVertex2f(w, h);
    glTexCoord2f(0.0, h); glVertex2f(0.0, h);
    glEnd();
}

void begin_window_coords()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, win_w, 0.0, win_h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void end_window_coords()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void draw_fullscreen_quad()
{
    begin_window_coords();
    draw_quad(win_w, win_h);
    end_window_coords();
}

void draw_scene_texture()
{
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable(GL_DEPTH_TEST);

    draw_fullscreen_quad();

    glDisable(GL_TEXTURE_RECTANGLE_NV);
}

void compute_histogram()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // draw scene texture n times, counting number of pixels
    begin_window_coords();
    for(int i=0; i<histo->GetWidth(); i++) {
        histo->BeginCount(i);
        draw_fullscreen_quad();
        histo->EndCount();
    }
    end_window_coords();
}

// show a single pass of the histogram process
void show_histogram_pass(int i)
{
    glClear(GL_COLOR_BUFFER_BIT);

    histo->BeginCount(i);
    draw_fullscreen_quad();
    histo->EndCount();
}

void display_histogram()
{
    begin_window_coords();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    histo->Display(10.0, 10.0, 256.0 / histo->GetWidth(), 200.0);

    end_window_coords();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // gradient background
    begin_window_coords();
    glBegin(GL_QUADS);
        glColor3f(0.2, 0.4, 0.8);
        glVertex2f(0.0, 0.0);
        glVertex2f(win_w, 0.0);
        glColor3f(0.05, 0.1, 0.2);
        glVertex2f(win_w, win_h);
        glVertex2f(0, win_h);
    glEnd();
    end_window_coords();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    object.apply_transform();

    // draw scene
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glutSolidTeapot(1.0);

    // copy window to texture
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, scene_tex);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, win_w, win_h);

    // compute histogram
    if (b['d']) {
        show_histogram_pass(pass);
    } else {
        compute_histogram();
    }

    // display histogram
    if (b['h']) {
        display_histogram();
    }

    glutSwapBuffers();
}

void idle()
{
    if (b[' '])
        object.trackball.increment_rotation();
    
    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];

    switch(k) {
    case 27:
    case 'q':
        exit(0);
        break;
    case '1':
    case '2':
    case '3':
    case '4':
        if (histo) {
            delete histo;
            int w = 32 << (k - '1');
            histo = new Histogram(w);
        }
        break;

    case '=':
    case '+':
        pass++;
        if (pass > histo->GetWidth()) pass = histo->GetWidth();
        break;

    case '-':
        pass--;
        if (pass < 0) pass = 0;
        break;

    case 'l':
        // luminance -> rgb
        histo->SetChannels(0.299, 0.587, 0.114);
        break;
    case 'r':
        histo->SetChannels(1.0, 0.0, 0.0);
        break;
    case 'g':
        histo->SetChannels(0.0, 1.0, 0.0);
        break;
    case 'b':
        histo->SetChannels(0.0, 0.0, 1.0);
        break;

    }

    object.keyboard(k, x, y);    
	glutPostRedisplay();
}

void resize(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 10.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.reshape(w, h);

    if (scene_tex) {
        glDeleteTextures(1, &scene_tex);
    }
    scene_tex = create_texture(GL_TEXTURE_RECTANGLE_NV, w, h);

    win_w = w; win_h = h;
}

void mouse(int button, int state, int x, int y)
{
    object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    object.motion(x, y);
}

void main_menu(int i)
{
    key((unsigned char) i, 0, 0);
}

void init_menu()
{    
    int width_menu = glutCreateMenu(main_menu);
    glutAddMenuEntry("32 entries", '1');
    glutAddMenuEntry("64 entries", '2');
    glutAddMenuEntry("128 entries", '3');
    glutAddMenuEntry("256 entries", '4');

    int channel_menu = glutCreateMenu(main_menu);
    glutAddMenuEntry("Luminance [l]", 'l');
    glutAddMenuEntry("Red [r]", 'r');
    glutAddMenuEntry("Green [g]", 'g');
    glutAddMenuEntry("Blue [b]", 'b');

    glutCreateMenu(main_menu);
    glutAddSubMenu("Histogram width", width_menu);
    glutAddSubMenu("Color channel", channel_menu);
    glutAddMenuEntry("Toggle histogram [h]", 'h');
    glutAddMenuEntry("Toggle pass display [d]", 'd');
    glutAddMenuEntry("Increment pass [=]", '=');
    glutAddMenuEntry("Decrement pass [-]", '-');
    glutAddMenuEntry("Quit [esc]", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(win_w, win_h);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("image histogram");

	init_opengl();

    histo = new Histogram(32);

    object.configure_buttons(1);
    object.dolly.dolly[2] = -3;
    object.trackball.incr = rotationf(vec3f(1, 1, 0), 0.05);

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);
    init_menu();

    b[' '] = true;
    b['h'] = true;

	glutMainLoop();

	return 0;
}
