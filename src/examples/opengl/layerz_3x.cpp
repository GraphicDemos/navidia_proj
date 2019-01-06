/*

   This demo shows using ARB_shadow, ARB_depth_texture and 
   NV_texture_shader to implement Rui Bastos's idea for getting 
   the layers of z sorted by depth.
  Cass Everitt
  2-16-01

*/  

#if defined(WIN32)
#  include <windows.h>
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

glut_callbacks cb;
glut_perspective_reshaper reshaper;
glut_simple_mouse_interactor camera, object;

tex_object_2D decal;
tex_object_rectangle  ztex;

arb_fragment_program fp_peel;
arb_fragment_program fp_nopeel;

display_list quad, geometry;


bool b[256];

GLuint * zbuf, * zbuf2;
int layer = 0;

// glut callbacks
void key(unsigned char k, int x, int y);
void display();
void menu(int entry);
void reshape(int w, int h);
void idle();

void init_opengl();

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayString("rgb double depth>=24");
    glutCreateWindow("Depth Peeling 2");


    init_opengl();  
    
    glut_helpers_initialize();
    cb.keyboard_function = key;
    cb.display_function = display;
    cb.reshape_function = reshape;


    camera.configure_buttons(1);
    object.configure_buttons(1);

    object.dolly.dolly[2] = -3; // push plane forward

    camera.set_camera_mode(true);

    camera.set_parent_rotation( & camera.trackball.r);
    object.set_parent_rotation( & camera.trackball.r);

    camera.disable();
    // attach interactors to the event multiplexor
    glut_add_interactor(& cb);
    glut_add_interactor(& reshaper);
    glut_add_interactor(& object);
    glut_add_interactor(& camera);

    zbuf = 0;
    zbuf2 = 0;

    b['/'] = true; // use DOT_PRODUCT_DEPTH_REPLACE to compute z/w per-pixel

    glutCreateMenu(menu);
    glutAddMenuEntry("peel layer [z]", 'z');
    glutAddMenuEntry("unpeel layer [Z]", 'Z');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

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
    ztex.bind();
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_DEPTH_COMPONENT, width, height, 0, 
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
    delete [] zbuf;
    delete [] zbuf2;
    zbuf = new GLuint [ width * height ];
    zbuf2 = new GLuint [ width * height ];
}

void init_opengl()
{
    if(! glh_init_extensions("GL_ARB_fragment_program "
							 "GL_ARB_fragment_program_shadow "
							 "GL_ARB_vertex_program "
                             "GL_ARB_depth_texture "
                             "GL_ARB_shadow "
							 "GL_EXT_shadow_funcs "))
    {
        cerr << "Necessary extensions were not supported:" << endl
             << glh_get_unsupported_extensions() << endl;
        cerr << "Extensions: " << glGetString(GL_EXTENSIONS) << endl << endl;
        cerr << "Renderer: " << glGetString(GL_RENDERER) << endl << endl;
        quitapp(0);
    }
    
    if (!glh_extension_supported("GL_EXT_texture_rectangle") && !glh_extension_supported("GL_NV_texture_rectangle"))
    {
        cerr << "Texture rectangles are unsupported, exiting..." << endl;
        quitapp(0);
    }

    glClearColor(.3f, .3f, .1f, 1);


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

	char _fp_peel[] = 
        "!!ARBfp1.0\n"
        "OPTION ARB_fragment_program_shadow;\n"
        "TEMP R0;\n"
        "TEX R0.x, fragment.position, texture[0], SHADOWRECT;\n" 
        "ADD R0.x, R0.x, -0.5;\n"           
        "KIL R0.x;\n"                       
        "MOV result.color, fragment.color;\n"
        "END\n";

	fp_peel.bind();
	fp_peel.load(_fp_peel);

	
	char _fp_nopeel[] = 
        "!!ARBfp1.0\n"
        "MOV result.color, fragment.color.primary;\n"
        "END\n";

	fp_nopeel.bind();
	fp_nopeel.load(_fp_nopeel);
	

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, &vec4f(0,0,0,0)[0]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, &vec4f(1,1,1,1)[0]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, &vec4f(1,1,1,1)[0]);
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &vec4f(1,1,1,1)[0]);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glFrontFace(GL_CW);
    glEnable(GL_NORMALIZE);

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

    if(k=='z') layer++;
    if(k=='Z') layer--;
    if(layer < 0) layer = 0;

    glutPostRedisplay();
}


void render_scene()
{
    glPushMatrix();
    object.apply_transform();
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &vec4f(0,0,1,1)[0]);
    quad.call_list();
    glMaterialfv(GL_FRONT, GL_DIFFUSE, &vec4f(1,0,0,1)[0]);
    glMaterialfv(GL_BACK, GL_DIFFUSE, &vec4f(0,1,0,1)[0]);
    geometry.call_list();
    glPopMatrix();
}


void render_scene_from_camera_view(bool shadow, bool update_ztex)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glLoadIdentity();
    camera.apply_inverse_transform();

    ztex.bind();
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

    if(shadow)
		fp_peel.bind();
	else
		fp_nopeel.bind();

    render_scene();

    glDisable(GL_FRAGMENT_PROGRAM_ARB);


    glPopMatrix();

    if(update_ztex)
    {
        ztex.bind();
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, reshaper.width, reshaper.height);
    }

}


void display()
{
    for(int l=0; l <= layer; l++) 
        render_scene_from_camera_view(l > 0, l < layer);

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}
