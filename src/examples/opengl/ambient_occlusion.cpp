/*********************************************************************NVMH1****
File:
ambient_occlusion.cpp

Copyright (C) 2003 NVIDIA Corporation

This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Ambient Occlusion using Hardware Shadow Maps

Comments:
    This example demonstrates hardware-accelerated "ambient occlusion" using
    a hemisphere of shadow-mapped lights. Each light is rendered in a separate
    pass, and the results are summed together using a floating point accumulation
    buffer. The projection matrix is randomly jittered to provide anti-aliasing.

    http://www-viz.tamu.edu/students/bmoyer/617/ambocc/
    http://zj.deathfall.com/depthbasedOcc.htm

Author:
    sgreen
    Based on "simple_shadow_map_render_depth_texture" by Sébastien Domin?

******************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

// use glh for easy extension
#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include "../nv_math/nv_math.h"
#include "../nv_manip/nv_manip.h"

#include "shared/NvIOModel.h"
#include "shared/accpersp.h"
#include "shared/pbuffer.h"
#include "shared/quitapp.h"

using namespace std;

// forward declarations
void init_opengl();
void init_cg();
void init_scene();
void shutdown();
void display();
void idle();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void menu(int item);
void keyboard(unsigned char key, int x, int y);
void reshape(int w, int h);

int shadowmap_size = 1024;

// camera standard manipulator
nv_manip * eye_manip = 0;
// light standard manipulator
nv_manip * light_manip = 0;

// the current manipulator
nv_manip * manip, * obj_manip;
// rendering setup
// near and far clip planes
nv_scalar near_z = .5;
nv_scalar far_z = 100.0;
// field of view
nv_scalar fovy = 40.0;

PBuffer *pbuffer = 0;   // pbuffer for shadow map

// wireframe
GLenum fill_mode = GL_FILL;
bool toggle_accum = true;
bool toggle_jitter = true;

// Display list for the floor
GLuint  scene_dl;

// GLUT Window
int     glut_wnd;

// Textures
GLenum  tex_target = GL_TEXTURE_2D;
//GLenum  tex_target = GL_TEXTURE_RECTANGLE_NV;
GLuint  light_depth;
GLuint  light_color;

// Polygon Offset scale and bias
nv_scalar po_scale, po_bias;

bool show_light_depth = false;

int sample = 0;
int xsamples = 8, ysamples = 16;
int nsamples = xsamples*ysamples;

float dist = -15.0; // light distance from origin

//int win_w = 1024, win_h = 768;
int win_w = 512, win_h = 512;

NvIOModel *model = 0;

int main(int argc, char* argv[])
b{
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_ACCUM);
    glutInitWindowSize(win_w, win_h);
	glutCreateWindow("hardware occlusion using shadow maps");

	if(! glh_init_extensions( "GL_ARB_multitexture "
                              "GL_SGIX_depth_texture "
                              "GL_SGIX_shadow "
                              "WGL_ARB_pbuffer "
                              "WGL_ARB_render_texture "
                              "WGL_ARB_pixel_format "
                              "WGL_NV_render_depth_texture "
                              "GL_NV_fragment_program "
                              "GL_NV_vertex_program "
                              "GL_NV_float_buffer "
                              ))
	{
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl << endl;
		cerr << "Extensions: " << glGetString(GL_EXTENSIONS) << endl << endl;
		cerr << "Renderer: " << glGetString(GL_RENDERER) << endl << endl;
        quitapp(0);
	}

    // load obj model
    if (argc > 1) {
        model = new NvIOModel();
        if (!model->ReadOBJ(argv[1])) {
            fprintf(stderr, "Error reading model '%s'\n", argv[1]);
            quitapp(0);
        }
        printf("%d verts, %d tris\n", model->m_nverts, model->m_nindices/3);
        model->Rescale();
    }

  	init_opengl();

    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
  	glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
	glutDisplayFunc(display);

  	glutCreateMenu(menu);
	glutAddMenuEntry("4 samples", 4);
	glutAddMenuEntry("32 samples", 32);
	glutAddMenuEntry("128 samples", 128);
	glutAddMenuEntry("512 samples", 512);
	glutAddMenuEntry("2048 samples", 2048);
	glutAddMenuEntry("Toggle accumulation [a]", 'a');
	glutAddMenuEntry("Move object [o]", 'o');
	glutAddMenuEntry("Move camera [c]", 'c');
	glutAddMenuEntry("Toggle jitter [j]", 'j');
	glutAddMenuEntry("Next light position [ ]", ' ');
	glutAddMenuEntry("Quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();

    return 0;
}

void shutdown()
{
    delete pbuffer;
    delete eye_manip;
    delete light_manip;
    delete obj_manip;
    exit(0);
}

void init_opengl()
{
    po_scale = 2.0;
    po_bias = 4.0;

    manip = eye_manip = new nv_manip();
    eye_manip->set_eye_pos(vec3(nv_zero, nv_zero, 6.0f));
    eye_manip->set_clip_planes(near_z,far_z);
    eye_manip->set_fov(fovy);

    light_manip = new nv_manip();
    light_manip->set_eye_pos(vec3(3.0f, 4.0f, 0.0f));
    light_manip->set_clip_planes(near_z,far_z);
    light_manip->set_fov(fovy);
    light_manip->set_screen_size(shadowmap_size, shadowmap_size);

    obj_manip = new nv_manip();
    obj_manip->set_manip_behavior(nv_manip::OBJECT | nv_manip::PAN | nv_manip::TRANSLATE | nv_manip::ROTATE);

    glClearColor(1.0, 1.0, 1.0, 1.0);

    // use light to help self-shadowing artifacts
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, vec4_null.vec_array);
    GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, black);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0, 1.0, 1.0);

    init_scene();

    if (tex_target == GL_TEXTURE_2D)
      pbuffer = new PBuffer("rgba depth texture2D=depth");
    else
      pbuffer = new PBuffer("rgba depth textureRECT=depth");

    pbuffer->Initialize(shadowmap_size, shadowmap_size, false, true);
    pbuffer->Activate();

    glEnable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);    // try and get fast Z mode

    // Initialize some state for the GLUT window's rendering context.
    pbuffer->Deactivate();

    // create textures for pbuffer rtt

    glGenTextures(1,&light_depth);
    glBindTexture(tex_target, light_depth);
	glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set up hardware shadow mapping
    glTexParameteri(tex_target, GL_TEXTURE_COMPARE_SGIX, GL_TRUE);
    glTexParameteri(tex_target, GL_TEXTURE_COMPARE_OPERATOR_SGIX, GL_TEXTURE_LEQUAL_R_SGIX);

    glGenTextures(1,&light_color);
    glBindTexture(tex_target, light_color);
	glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
}

void init_scene()
{
    // generate a display list for the scene
    scene_dl = glGenLists(1);

    glNewList(scene_dl,GL_COMPILE);

    // floor
    const float s = 6.0;
    const float y = -1.0;
    const float delta = -0.1;

	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-s, y, -s );
	glVertex3f(-s, y,  s );
	glVertex3f( s, y,  s );
	glVertex3f( s, y, -s );
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f( s, y + delta, -s );
	glVertex3f( s, y + delta,  s );
	glVertex3f(-s, y + delta,  s );
	glVertex3f(-s, y + delta, -s );
	glEnd();

    if (!model) {
        glPushMatrix();
        glTranslatef(2.0, 0.0, 2.0);
        glutSolidSphere(1.0, 20, 40);
        glPopMatrix();

        glPushMatrix();
        glutSolidTeapot(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-2.0, -0.5, -1.0);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(2.0, -0.5, -2.0);
        glutSolidCone(1.0, 3.0, 10, 2);
        glPopMatrix();
    }

    glEndList();
}

void menu(int item)
{
    switch(item) {
    case 4:
        xsamples = 2; ysamples = 2;
        break;
    case 32:
        xsamples = 4; ysamples = 8;
        break;
    case 128:
        xsamples = 8; ysamples = 16;
        break;
    case 512:
        xsamples = 16; ysamples = 32;
        break;
    case 2048:
        xsamples = 32; ysamples = 64;
        break;
    default:
        keyboard((unsigned char)item, 0, 0);
    }
    nsamples = xsamples * ysamples;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
    case 'q':
    case 27:
      shutdown();
      break;

    case 'w':
      fill_mode = (fill_mode == GL_FILL) ? GL_LINE : GL_FILL;
      glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
      break;

    case 'r':
      eye_manip->set_eye_pos(vec3(nv_zero, nv_zero, 6.0f));
      eye_manip->set_focus_pos(vec3_null);
      light_manip->set_eye_pos(vec3(3.0f, 4.0f, 0.0f));
      light_manip->set_focus_pos(vec3_null);
      break;

    case 'd':
      show_light_depth = !show_light_depth;
      break;

    case 'l':
      manip = light_manip;
      break;

    case 'c':
      manip = eye_manip;
      break;      

    case 'o':
      manip = obj_manip;
      break;      

    case ' ':
      sample = (sample + 1) % nsamples;
      break;

    case 'a':
      toggle_accum ^= 1;
      break;

    case 'j':
      toggle_jitter ^= 1;
      break;
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    int input_state = 0;
    // mouse keyboard state mask
    input_state |= (button == GLUT_LEFT_BUTTON) ? 
        ((state == GLUT_DOWN) ? nv_manip::LMOUSE_DN : nv_manip::LMOUSE_UP) : 0;
    input_state |= (button == GLUT_MIDDLE_BUTTON) ? 
        ((state == GLUT_DOWN) ? nv_manip::MMOUSE_DN : nv_manip::MMOUSE_UP) : 0;
    input_state |= (button == GLUT_RIGHT_BUTTON) ? 
        ((state == GLUT_DOWN) ? nv_manip::RMOUSE_DN : nv_manip::RMOUSE_UP) : 0;
    // build keyboard state mask
    int key_state = glutGetModifiers();
    input_state |= (key_state & GLUT_ACTIVE_CTRL) ? nv_manip::CTRL_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_ALT) ? nv_manip::ALT_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_SHIFT) ? nv_manip::SHIFT_DN : 0;

    // dispatch appropriately
    if (input_state & nv_manip::MOUSE_DN)
        manip->mouse_down(vec2(x,y),input_state);
    if (input_state & nv_manip::MOUSE_UP)
        manip->mouse_up(vec2(x,y),input_state);

    glutPostRedisplay();
}

void motion(int x, int y)
{
    manip->mouse_move(vec2(x,y),0);
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    eye_manip->set_screen_size(w,h);
    obj_manip->set_screen_size(w,h);

    pbuffer->Deactivate();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( eye_manip->get_fov(), 
                    eye_manip->get_screen_ratio(), 
                    eye_manip->get_near_z(), 
                    eye_manip->get_far_z());

    glViewport(0,0,w,h);

}

void idle()
{
}

void render_scene(const mat4 &mat)
{
	glColor3f(1,1,1);
    glLoadMatrixf( mat.mat_array );

    glCallList(scene_dl);

    if (model) {
        glPushMatrix();
        glMultMatrixf( obj_manip->get_mat().mat_array );
        glRotatef(-90.0, 1.0, 0.0, 0.0);
        model->Render();
        glPopMatrix();
    }
}

// generate light positions on hemisphere
// we should really use a more uniform distribution here

void build_light_matrix(int i, mat4 &mat)
{
    i = i % (xsamples * ysamples);

    int ix = i / ysamples;
    int iy = i % ysamples;

    float rx = (90.0*ix  / (float) xsamples);
    float ry = (360.0*iy / (float) ysamples);

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0, 0.0, dist);
    glRotatef(rx, 1.0, 0.0, 0.0);
    glRotatef(ry, 0.0, 1.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, &mat.mat_array[0]);
    glPopMatrix();
}

void render_light_positions()
{
    glDisable(GL_LIGHTING);

    for(int i=0; i<xsamples*ysamples; i++) {
        mat4 light_matrix;
        build_light_matrix(i, light_matrix);

        mat4 inverse;
        invert(inverse, light_matrix);
        vec4 light_pos(inverse.a03, inverse.a13, inverse.a23, 1.0);

        if (i==sample) {
          glPointSize(6.0);
          glColor3f(1.0, 0.0, 0.0);
        } else {
          glPointSize(3.0);
          glColor3f(0.0, 0.0, 1.0);
        }
        glBegin(GL_POINTS);
        glVertex3fv(light_pos.vec_array);
        glEnd();
    }
}


void render_scene_from_light_view(mat4 &mat)
{
	// place light at the origin
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION,vec4_w.vec_array);

    glViewport(0,0,shadowmap_size, shadowmap_size);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( light_manip->get_fov(), 
                    light_manip->get_screen_ratio(), 
                    light_manip->get_near_z(), 
                    light_manip->get_far_z());
    glMatrixMode(GL_MODELVIEW);

    render_scene(mat);
}

void render_light_depth(mat4 &mat)
{
    pbuffer->Activate();
    glClear(GL_DEPTH_BUFFER_BIT);

	glPolygonOffset(po_scale, po_bias);
	glEnable(GL_POLYGON_OFFSET_FILL);

	render_scene_from_light_view(mat);

	glDisable(GL_POLYGON_OFFSET_FILL);

    pbuffer->Deactivate();
}

float frand()
{
    return rand() / (float) RAND_MAX;
}

// using fixed-function pipeline
void display_pass_fixed(int sample)
{
    mat4 light_matrix;
    build_light_matrix(sample, light_matrix);

    render_light_depth(light_matrix);

    mat4 inverse;
    invert(inverse, light_matrix);
    vec4 light_pos(inverse.a03, inverse.a13, inverse.a23, 1.0);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Load our camera matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( eye_manip->get_mat().mat_array );

	// Place the light
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos.vec_array);

    glEnable(GL_LIGHTING);

    // Set up depth compare
	glTexGenfv(GL_S, GL_EYE_PLANE, vec4_x.vec_array);
    glTexGenfv(GL_T, GL_EYE_PLANE, vec4_y.vec_array);
    glTexGenfv(GL_R, GL_EYE_PLANE, vec4_z.vec_array);
    glTexGenfv(GL_Q, GL_EYE_PLANE, vec4_w.vec_array);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

   	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

    // Set up the depth texture projection
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
    // Offset
	glTranslatef(.5f, .5f, .5f);
    // Bias
	glScalef(.5f, .5f, .5f);
    // light frustum
    gluPerspective( light_manip->get_fov(), 
                    light_manip->get_screen_ratio(), 
                    light_manip->get_near_z(), 
                    light_manip->get_far_z());
    // light matrix
    glMultMatrixf( light_matrix.mat_array );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (!toggle_jitter) {
      gluPerspective( eye_manip->get_fov(), 
                      eye_manip->get_screen_ratio(), 
                      eye_manip->get_near_z(), 
                      eye_manip->get_far_z());
    } else {
      // randomly jitter projection matrix
      float dx = frand();
      float dy = frand();
      accPerspective(eye_manip->get_fov(), 
                      eye_manip->get_screen_ratio(), 
                      eye_manip->get_near_z(), 
                      eye_manip->get_far_z(),
                      dx,
                      dy,
                      0.0,
                      0.0,
                      1.0);
    }

    glViewport(0, 0, eye_manip->get_screen_width(), eye_manip->get_screen_height());
    glMatrixMode(GL_MODELVIEW);

    // bind the depth texture for the shadow mapping
	glEnable(tex_target);
	glBindTexture(tex_target, light_depth);

    pbuffer->Bind(WGL_DEPTH_COMPONENT_NV);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    render_scene(eye_manip->get_mat());

    pbuffer->Release(WGL_DEPTH_COMPONENT_NV);

    glDisable(tex_target);
   	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
}

// accumulate samples
void display_accum()
{
    srand(1973);
    int start_time = glutGet(GLUT_ELAPSED_TIME);

    // accumlate samples
    glClear(GL_ACCUM_BUFFER_BIT);

    for(int i=0; i<nsamples; i++) {
        display_pass_fixed(i);
        glAccum(GL_ACCUM, 1.0 / nsamples);
    }

    glAccum(GL_RETURN, 1.f);

    int end_time = glutGet(GLUT_ELAPSED_TIME);
    float time = (end_time - start_time) / 1000.0;
    printf("%f secs, %f fps, %d samples, %f secs/sample\n", time, 1.0 / time, nsamples, time/nsamples);
}

void display_single()
{
    glClear(GL_ACCUM_BUFFER_BIT);

    srand(1973);
    display_pass_fixed(sample);
    render_light_positions();

    glAccum(GL_ACCUM, 1.0);
    glAccum(GL_RETURN, 1.f);
}

void display()
{
  if (toggle_accum)
    display_accum();
  else
    display_single();

  glutSwapBuffers();
}
