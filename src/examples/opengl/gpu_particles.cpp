/*
  GPU particle system

  Based on cg_physics, inspired by Lutz Latta's "Building a Million Particle System":
  http://www.2ld.de/gdc2004/
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#include <Cg/cgGL.h>
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
vec3f camera_trans(0, -2, -5);
vec3f camera_rot(0, 0, 0);
vec3f camera_trans_lag = camera_trans;
vec3f camera_rot_lag = camera_rot;
const float intertia = 5.0;

int mode = 0;
int draw_mode = 1;
enum { M_VIEW = 0, M_MOVE, M_MOVE_EMITTER };
float modelView[16];

float last_time;

GLboolean toggle[256];
vec3f sphere_pos(0.0, 2.0, 0.0), sphere_pos_prev, sphere_vel;
vec3f emitter_pos(0.0, 4.0, 0.0), emitter_pos_prev, emitter_vel;
vec2f emitter_ang(0.0, -GLH_PI/4.0);
int emitter_rate = 0;

bool have_mrt = false;

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

void glPrint(int x, int y, const char *s, void *font)
{
  int i, len;

  glRasterPos2f(x, y);
  len = (int) strlen(s);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, s[i]);
  }
}

void display(void)
{   
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    float time_step = time - last_time;
    last_time = time;
    if (time_step < 0.01) time_step = 0.01;
    if (time_step > 0.5) time_step = 0.0;
//    printf("%f\n", time_step);

    if (toggle['o']) {
        time_step *= 0.25;   // slow motion
    }

    if (toggle['a']) {
        emitter_vel = emitter_pos - emitter_pos_prev;
        psys->HoseEmitter(emitter_rate * time_step, emitter_pos, emitter_vel, emitter_ang[0], emitter_ang[1], 0.2, 0.5, 1.0, 10.0);
        emitter_pos_prev = emitter_pos;

        // simulate particle system
        psys->TimeStep(time_step);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view transform
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#if 0
    glTranslatef(camera_trans[0], camera_trans[1], camera_trans[2]);
    glRotatef(camera_rot[0], 1.0, 0.0, 0.0);
    glRotatef(camera_rot[1], 0.0, 1.0, 0.0);
#else
    camera_trans_lag += (camera_trans - camera_trans_lag) * intertia * time_step;
    camera_rot_lag += (camera_rot - camera_rot_lag) * intertia * time_step;
    glTranslatef(camera_trans_lag[0], camera_trans_lag[1], camera_trans_lag[2]);
    glRotatef(camera_rot_lag[0], 1.0, 0.0, 0.0);
    glRotatef(camera_rot_lag[1], 0.0, 1.0, 0.0);
#endif
	  glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

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

    if (toggle['e']) {
        psys->DisplayTerrain();
    }

    // draw sphere
    if (toggle['s']) {
        glPushMatrix();
        glTranslatef(sphere_pos[0], sphere_pos[1], sphere_pos[2]);
        glColor3f(0.0, 0.0, 1.0);
        glutSolidSphere(1.0, 60, 30);
//        glutSolidSphere(0.1, 20, 10);
        glPopMatrix();
    }
    sphere_vel = sphere_pos - sphere_pos_prev;
    psys->SetSphere(sphere_pos, sphere_vel);
    sphere_pos_prev = sphere_pos;

    // draw emitter
    if (toggle['N']) {
        glPushMatrix();
        glTranslatef(emitter_pos[0], emitter_pos[1], emitter_pos[2]);
        glColor3f(1.0, 0.0, 0.0);
        glutSolidSphere(0.05, 20, 10);
        glPopMatrix();
    }

    // draw particle system
    switch(draw_mode) {
    case 0:
      psys->ReadBack();
      psys->Display();
      break;
    case 1:        
        psys->DisplayFast(false, toggle['p']);
        break;
    case 2:        
        psys->DisplayFast(true, toggle['p']);
        break;
    case 3:
        psys->DisplayLines(toggle['l']);
        break;
    }

    if (toggle['t'])
        psys->DisplayTextures();

    if (toggle['h']) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);

        glDisable(GL_DEPTH_TEST);

        int n = psys->GetNoActiveParticles();
        char str[100];
        sprintf(str, "particles: %d", n);
        glPrint(0, 5, str, GLUT_BITMAP_9_BY_15);

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
    }

    glutReportErrors();
    glutSwapBuffers();
}

// transform vector by transpose of matrix
vec3f ixform(vec3f a, GLfloat *m)
{
  vec3f r;
  r[0] = a[0]*m[0] + a[1]*m[1] + a[2]*m[2];
  r[1] = a[0]*m[4] + a[1]*m[5] + a[2]*m[6];
  r[2] = a[0]*m[8] + a[1]*m[9] + a[2]*m[10];
  return r;
}

void motion(int x, int y)
{
    float dx, dy;
    dx = x - ox;
    dy = y - oy;

    const float translateSpeed = 0.01;

    switch(mode) 
    {
        case M_VIEW:
            if (buttonState == 3) {
                // left+middle = zoom
                camera_trans[2] += dy / 100.0;
            } 
            else if (buttonState & 2) {
                // middle = translate
                camera_trans[0] += dx / 100.0;
                camera_trans[1] -= dy / 100.0;
            }
            else if (buttonState & 1) {
                // left = rotate
                camera_rot[0] += dy / 5.0;
                camera_rot[1] += dx / 5.0;
            }
            break;

        case M_MOVE:
            if (buttonState==1) {
                sphere_pos += ixform(vec3f(dx*translateSpeed, -dy*translateSpeed, 0.0), modelView);
            } else if (buttonState==2) {
                sphere_pos += ixform(vec3f(0.0, 0.0, dy*translateSpeed), modelView);
            }
            break;

        case M_MOVE_EMITTER:
            if (buttonState==3) {
                emitter_ang += vec2f(dx*0.01, dy*0.01);
            } else if (buttonState==1) {
                emitter_pos += ixform(vec3f(dx*translateSpeed, -dy*translateSpeed, 0.0), modelView);
            } else if (buttonState==2) {
                emitter_pos += ixform(vec3f(0.0, 0.0, dy*translateSpeed), modelView);
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

    case 'n':
        mode = M_MOVE_EMITTER;
        break;

    case 'r':
        psys->Reset();
//        psys->InitParticlesGrid( vec3f(-2.0, 5.0, -2.0), vec3f(4.0 / psys->GetWidth(), 0.0, 0.0), vec3f(0.0, 0.0, 4.0 / psys->GetHeight()) );
        break;

    case ' ':
        psys->TimeStep(0.05);
        break;

    case 'a':
        last_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        break;

    case '1':
    case '2':
    case '3':
    case '4':
        draw_mode = key - '1';
        break;

    case 'x':
        psys->HoseEmitter(10, emitter_pos, emitter_vel, 0.0, 0.0, 0.1, 0.5, 1.0, 10.0);
        break;

    case '=':
    case '+':
        emitter_rate += 1000;
        break;

    case '-':
        emitter_rate -= 1000;
        if (emitter_rate < 0) emitter_rate = 0;
        break;
    }

    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    switch(key) {
    case GLUT_KEY_F1:
        psys->InitParticlesGrid( vec3f(-2.0, 5.0, -2.0), vec3f(4.0 / psys->GetWidth(), 0.0, 0.0), vec3f(0.0, 0.0, 4.0 / psys->GetHeight()) );
        break;
    case GLUT_KEY_F2:
        psys->InitParticlesRand( vec3f(4.0, 4.0, 4.0), vec3f(-2.0, 4.0, -2.0) );
        break;
    case GLUT_KEY_F3:
        psys->InitParticlesStream( vec3f(-2.0, 2.0, -4.0), vec3f(0.0, 1.0, 1.0), 0.05, 0.2 );
        break;
    }
}

void idle(void)
{
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

void drawModeMenu(int i)
{
    draw_mode = i;
}

void initMenus(void)
{
    int drawmode_menu = glutCreateMenu(drawModeMenu);
    glutAddMenuEntry("points using CPU readback", 0);
    glutAddMenuEntry("points", 1);
    glutAddMenuEntry("points w/motion blur", 2);
//    glutAddMenuEntry("lines", 3);
    
    glutCreateMenu(mainMenu);
    glutAddSubMenu("Draw mode", drawmode_menu);
    glutAddMenuEntry("View mode [v]", 'v');
    glutAddMenuEntry("Move sphere mode [m]", 'm');
    glutAddMenuEntry("Move emitter mode [n]", 'n');
    glutAddMenuEntry("Toggle animation [a]", 'a');
    glutAddMenuEntry("Step animation [ ]", ' ');
    glutAddMenuEntry("Increase emitter rate [=]", '=');
    glutAddMenuEntry("Decrease emitter rate [-]", '-');
    glutAddMenuEntry("Toggle point sprites [p]", 'p');
    glutAddMenuEntry("Toggle smooth lines [l]", 'l');
    glutAddMenuEntry("Display textures [t]", 't');
    glutAddMenuEntry("Display terrain [e]", 'e');
    glutAddMenuEntry("Toggle sphere [s]", 's');
    glutAddMenuEntry("Toggle grid [g]", 'g');
    glutAddMenuEntry("Reset [r]", 'r');
    glutAddMenuEntry("Quit (esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// get extension pointers
void getExts(void)
{
    printf("%s\n", glGetString(GL_EXTENSIONS) );

    if (!glh_init_extensions("GL_ARB_multitexture "
                             "GL_NV_vertex_program "
                             "GL_NV_fragment_program "
                             "WGL_ARB_pbuffer "
                             "WGL_ARB_pixel_format "
                             "WGL_ARB_render_texture "
                             "GL_NV_float_buffer "
                             "GL_ARB_vertex_program "
                             "GL_ARB_vertex_buffer_object "
                             "GL_EXT_pixel_buffer_object "
                             "GL_NV_primitive_restart "
                             "GL_ARB_point_sprite "
                             "GL_ARB_point_parameters "
                             )) 
    {
        fprintf(stderr, "Error - required extensions were not supported: %s", glh_get_unsupported_extensions());
        quitapp(-1);
    }

    if (glh_init_extensions("GL_NV_fragment_program2 "
                            "GL_ARB_fragment_program "
                            "GL_ATI_draw_buffers "
                            ))
    {
        fprintf(stderr, "Multiple draw buffers available\n");
        have_mrt = true;
    }
}

void initGL()
{  
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glutReportErrors();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);
    (void) glutCreateWindow("gpu particle system");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutIdleFunc(idle);

    toggle['g'] = true;
    toggle['s'] = true;
    toggle['a'] = true;
    toggle['p'] = true;
    toggle['l'] = true;
    toggle['e'] = true;
    toggle['N'] = true;
    toggle['h'] = true;

    initMenus();
    getExts();
    initGL();

    int w = 256, h = 256;
    if (argc > 2) {
      w = atoi(argv[1]);
      h = atoi(argv[2]);
    }
    emitter_rate = w * h / 10.0;

    // create particle system
    psys = new ParticleSystem(w, h, 1, false);
//    psys = new ParticleSystem(w, h, 1, have_mrt);
//    psys->InitParticlesGrid( vec3f(-2.0, 5.0, -2.0), vec3f(4.0 / psys->GetWidth(), 0.0, 0.0), vec3f(0.0, 0.0, 4.0 / psys->GetHeight()) );

    last_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    sphere_pos_prev = sphere_pos;

    glutMainLoop();
    return 0;
}
