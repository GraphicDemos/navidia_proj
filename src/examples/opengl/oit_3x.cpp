/*

   This demo shows using ARB_shadow, ARB_depth_texture, and ARB_fragment_program 
   to implement Rui Bastos's idea for getting the layers of z sorted
   by depth, and using the RGBA at each layer to re-order transparency
   correctly.
  Cass Everitt
  3-28-01

*/  

#if defined(WIN32)
#include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <map>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_convenience.h>
#include <glh/glh_glut.h>

#include <shared/quitapp.h>

#if !defined(GL_TEXTURE_RECTANGLE_NV) && defined(GL_EXT_texture_rectangle)
#  define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#endif

using namespace std;
using namespace glh;

// glut callbacks
void key(unsigned char k, int x, int y);
void display();
void menu(int entry);
void reshape(int w, int h);
void idle();

void init_opengl();

#define GL_ERRORS
//#define GL_ERRORS  check_gl_errors()

void check_gl_errors()
{
    GLuint e = glGetError();
    if(e > 0)
    {
        fprintf(stderr, "GL error %d: %s\n", (int)e, gluErrorString(e));
        //__asm int 3;
    }
}


vec3f rand_vec()
{
    vec3f r;
    float rand_max(RAND_MAX);
    r[0] = rand() / rand_max;
    r[1] = rand() / rand_max;
    r[2] = rand() / rand_max;
    return r;
}

class bouncey_sphere
{
public:
    bouncey_sphere()
    { init(); }

    void init()
    {
        r = .05;
        min = vec3f(-1,-1, 0) * .5;
        max = vec3f(1,1,1) * .5;
        v = rand_vec() * .01f;
        p = rand_vec() * (max - min) + min;
    }
    void incr()
    {
        p += v;
        bound(0); bound(1); bound(2);
    }

    void render()
    {
        glPushMatrix();
        glTranslatef(p[0], p[1], p[2]);
        glutSolidSphere(r, 40, 30);
        glPopMatrix();
    }

    void bound(int d)
    {
        if(p[d] >  max[d]) { p[d] -= v[d]; v[d] *= -1; }
        if(p[d] <  min[d]) { p[d] -= v[d]; v[d] *= -1; }
    }
    vec3f p, v, min, max;
    float r;
};

#define NUM_SPHERES 16
bouncey_sphere bsphere[NUM_SPHERES];

glut_callbacks cb;
glut_perspective_reshaper reshaper;
glut_simple_mouse_interactor camera, object;

#define MAX_LAYERS 12

tex_object_2D decal, simple_1x1_uhilo;
tex_object_rectangle  ztex, rgba_layer[MAX_LAYERS];
bool ztex_consistent = false;

display_list quad, geometry;
arb_fragment_program fp_peel, fp_nopeel;

bool b[256];

float alpha = 1;
float sphere_alpha = 1;

GLuint * zbuf, * zbuf2;
GLenum depth_format;
int layer = 0;

int fudge = -4;

double epsilon = 1.0/((1<<24)-1);


int w_offset = 0; // for "dual view"

string title_l = "bad";
string title_r = "good";

bool animate = true;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(1000, 500);
    glutInitDisplayString("alpha>=8 rgb double depth>=24");
    glutCreateWindow("Order Independent Transparency 2");

    GLint depth_bits;
    glGetIntegerv(GL_DEPTH_BITS, & depth_bits);
    
    if(depth_bits == 16)  depth_format = GL_DEPTH_COMPONENT16_ARB;
    else                  depth_format = GL_DEPTH_COMPONENT24_ARB;
    
    GLint alpha_bits;
    glGetIntegerv(GL_ALPHA_BITS, & alpha_bits);
    if (alpha_bits < 8)
    {
        printf("8 bit alpha not supported, exiting...\n");
        quitapp(-1);
    }
    
    cerr << alpha_bits << " bits of dst alpha" << endl;

    init_opengl();  
    
    glut_helpers_initialize();
    cb.keyboard_function = key;
    cb.display_function = display;
    cb.reshape_function = reshape;

    camera.configure_buttons(1);
    object.configure_buttons(1);

    object.dolly.dolly[2] = -1.25; // push plane forward
    object.trackball.r = rotationf(vec3f(1,0,0), to_radians(-80.f));

    camera.set_camera_mode(true);

    camera.set_parent_rotation( & camera.trackball.r);
    object.set_parent_rotation( & camera.trackball.r);
    reshaper.aspect_factor = .5f;


    camera.disable();
    // attach interactors to the event multiplexor
    glut_add_interactor(& cb);
    glut_add_interactor(& reshaper);
    glut_add_interactor(& object);
    glut_add_interactor(& camera);

    zbuf = 0;
    zbuf2 = 0;

    layer = 4;     // start with several layers rendered...
    alpha = .5;    // start with some transparency

    int tweakables = glutCreateMenu(menu);
    glutAddMenuEntry("animate [ ]", ' ');
    glutAddMenuEntry("increase alpha [a]", 'a');
    glutAddMenuEntry("decrease alpha [A]", 'A');
    glutAddMenuEntry("increase num z layers [z]", 'z');
    glutAddMenuEntry("decrease num z layers [Z]", 'Z');

    glutCreateMenu(menu);
    glutAddSubMenu("tweakables", tweakables);
    glutAddMenuEntry("quit [q]", 'q');

    glutAttachMenu(GLUT_RIGHT_BUTTON);

    b[' '] = true;
    
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}

void menu(int entry)
{
    key(entry, 0, 0);
}

// resize the texture on window resize...
void reshape(int width, int height)
{
    width /= 2; // dualview 
    ztex.bind();
    delete [] zbuf;
    delete [] zbuf2;
    zbuf = new GLuint [ width * height ];
    zbuf2 = new GLuint [ width * height ];

    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, depth_format, width, height, 0, 
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, zbuf);
    ztex_consistent = true;

    for(int i=0; i < MAX_LAYERS; i++)
    {
        rgba_layer[i].bind();
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, width, height, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, zbuf);
        rgba_layer[i].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        rgba_layer[i].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    }
    delete [] zbuf;
    delete [] zbuf2;
    zbuf = new GLuint [ width * height ];
    zbuf2 = new GLuint [ width * height ];

    glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, width, 0, 0, 0);
}

void init_opengl()
{
    if(! glh_init_extensions("GL_ARB_vertex_program "
                             "GL_ARB_fragment_program "
                             "GL_ARB_fragment_program_shadow "
                             "GL_ARB_depth_texture "
                             "GL_ARB_shadow "))
    {
        cerr << "Necessary extensions were not supported:" << endl
             << glh_get_unsupported_extensions() << endl;
        cerr << "Extensions: " << glGetString(GL_EXTENSIONS) << endl << endl;
        cerr << "Renderer: " << glGetString(GL_RENDERER) << endl << endl;
        quitapp(0);
    }

    if (!glh_extension_supported("GL_EXT_texture_rectangle") && !glh_extension_supported("GL_NV_texture_rectangle"))
    {
        cerr << "Texture rectangles not supported, exiting..." << endl;
        quitapp(0);
    }

    glClearColor(.2f, .2f, .2f, 1);

    ztex.bind();
    ztex.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ztex.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ztex.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ztex.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ztex.parameter(GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    ztex.parameter(GL_TEXTURE_COMPARE_FUNC_ARB, GL_GREATER);

    quad.new_list(GL_COMPILE);
    glPushMatrix();
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(-1, -1);
    glTexCoord2f(0,1);
    glVertex2f(-1,  1);
    glTexCoord2f(1,1);
    glVertex2f( 1,  1);
    glTexCoord2f(1,0);
    glVertex2f( 1, -1);
    glEnd();
    glPopMatrix();
    quad.end_list();

    geometry.new_list(GL_COMPILE);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, .25f, 0);
    glutSolidTeapot(.25f);
    glPopMatrix();
    geometry.end_list();

    const char peel_fp[] = 
    "!!ARBfp1.0\n"
    "OPTION ARB_fragment_program_shadow;\n"
    "PARAM offset = program.env[0];\n"
    "TEMP R0;\n"
    "MOV R0, fragment.position;\n"
    "ADD R0.x, R0.x, -offset;\n"
    "TEX R0.x, R0, texture[0], SHADOWRECT;\n" 
    "ADD R0.x, R0.x, -0.5;\n"           
    "KIL R0.x;\n"                       
    "MOV result.color, fragment.color;\n"
    "END\n";

    fp_peel.bind();
    fp_peel.load(peel_fp);
    GL_ERRORS;

    const char nopeel_fp[] = 
    "!!ARBfp1.0\n"
    "MOV result.color, fragment.color.primary;\n"
    "END\n";

    fp_nopeel.bind();
    fp_nopeel.load(nopeel_fp);
    GL_ERRORS;

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, vec4f(1,1,1,0).v);
    glLightfv(GL_LIGHT0, GL_AMBIENT, vec4f(0,0,0,0).v);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, vec4f(1,1,1,1).v);
    glLightfv(GL_LIGHT0, GL_SPECULAR, vec4f(1,1,1,1).v);
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, vec4f(1,1,1,1).v);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}

void idle()
{
    if (animate)
    {
        for(int i=0; i < NUM_SPHERES; i++)
            bsphere[i].incr();
    }

    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
    b[k] = !b[k];
    if(k==27 || k=='q') exit(0);
    if(k == '1' || k == '2')
    {
        object.disable();
        camera.disable();

        if     (k=='1') camera.enable();
        else   object.enable();

    }

    if(k==' ')
    {
        animate = b[k];
    }

    if('f' == k)
    {
        if(b[k])
            glDrawBuffer(GL_FRONT);
        else
            glDrawBuffer(GL_BACK);
    }

    if(k=='a')
        alpha += .05;
    if(k=='A')
        alpha -= .05;
    if(k=='s')
        sphere_alpha += .05;
    if(k=='S')
        sphere_alpha -= .05;
    if(k=='z')
        layer++;
    if(k=='Z')
        layer--;
    if(layer < 0)
        layer = 0;
    if(layer > 11)
        layer = 11;

    glutPostRedisplay();
}

vec4f random_vec4f()
{
    vec4f v;
    v[0] = .25 + (.75 * rand() / float(RAND_MAX));
    v[1] = .25 + (.75 * rand() / float(RAND_MAX));
    v[2] = .25 + (.75 * rand() / float(RAND_MAX));
    v[3] = sphere_alpha * ( .25 * rand() / float(RAND_MAX) + .75);
    return v;
}

void random_color()
{
    vec4f v = random_vec4f();
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, v.v);
    v *= .5;
    v += .5;
    glMaterialfv(GL_FRONT, GL_SPECULAR, v.v);
    v *= .5;
    glMaterialfv(GL_BACK, GL_SPECULAR, v.v);
}


void render_spheres()
{
    srand(10);
    for(int i=0; i < NUM_SPHERES; i++)
    {
        random_color();
        bsphere[i].render();
    }
}

bool good;
bool peel;

void render_scene()
{

    glPushMatrix();
    object.apply_transform();
    glTranslatef(0, 0, -.25f);

    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    if(peel)
        fp_peel.bind();
    else
        fp_nopeel.bind();

    GL_ERRORS;
    glFrontFace(GL_CCW);
    render_spheres();
    glFrontFace(GL_CW);

    glMaterialfv(GL_FRONT, GL_SPECULAR, vec4f(1,1,1,1).v);
    glMaterialfv(GL_BACK, GL_SPECULAR, vec4f(.5,.5,.5,.5).v);
    if(b['p']) // draw ground plane?
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec4f(0,0,1,1).v);
        quad.call_list();
    }

    GL_ERRORS;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec4f(.2f,1,.2f,alpha).v);
    glMaterialfv(GL_BACK, GL_DIFFUSE, vec4f(.5f,1,.2f,alpha).v);
    geometry.call_list();

    glPopMatrix();
    
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

void render_scene_from_camera_view(bool shadow, bool update_ztex, int l)
{
    peel = shadow;
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glLoadIdentity();
    camera.apply_inverse_transform();
    ztex.bind();
    render_scene();

    glPopMatrix();

    if(update_ztex)
    {
        // copy the z
        ztex.bind();
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, w_offset, 0, reshaper.width/2, reshaper.height);
    }

    // copy the RGBA of the layer
    rgba_layer[l].bind();
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, w_offset, 0, reshaper.width/2, reshaper.height);
    glFinish();
    glFinish();
}

// draw a textured quad for each layer and blend them all together correctly
void draw_sorted_transparency()
{
    int w = reshaper.width/2;
    int h = reshaper.height;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_2D);

    for(int i=layer; i >= 0; i--)
    {
        rgba_layer[i].bind();
        rgba_layer[i].enable();
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);
        glTexCoord2f(0, h);
        glVertex2f(0, h);
        glTexCoord2f(w, h);
        glVertex2f(w, h);
        glTexCoord2f(w, 0);
        glVertex2f(w, 0);
        glEnd();
        
    }
    rgba_layer[0].disable();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void bad_transparency()
{
    good = false;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    if(b['d'])
        glDisable(GL_DEPTH_TEST);
    render_scene_from_camera_view(false, false, 0);
    if(b['d'])
        glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void good_transparency()
{
    good = true;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    for(int l=0; l <= layer; l++) 
        render_scene_from_camera_view(l > 0, l < layer, l);
    draw_sorted_transparency();
}



void render_string(const char * str)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-.9f, .8f, 0);
    glScalef(.0006f, .0006f, 1);
    while(*str)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *str);
        str++;
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}



void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    GLint w = vp[2]/2;
    GLint h = vp[3];

    glEnable(GL_SCISSOR_TEST);

    // left
    glViewport(0, 0, w, h);
    glScissor(0, 0, w, h);
    bad_transparency();
    render_string(title_l.c_str());

    // right
    glViewport(w, 0, w, h);
    glScissor(w, 0, w, h);
    w_offset = w;
    good_transparency();
    render_string(title_r.c_str());

    glViewport(vp[0], vp[1], vp[2], vp[3]);

    if(! b['f'])
        glutSwapBuffers();
}

