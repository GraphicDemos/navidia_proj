/*********************************************************************NVMH1****
File:
texshd_offset_2d.cpp

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    This example demonstrates how to use the NV_texture_shaders extension
    with the Texture Offset 2D program.

    S閎astien Domin?
    sdomine@nvidia.com

******************************************************************************/

#if defined(WIN32)
# include <windows.h>
#elif defined(UNIX)
# include <GL/glx.h>
#endif

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#if !defined(GL_TEXTURE_RECTANGLE_NV) && defined(GL_EXT_texture_rectangle)
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <nvparse.h>
#include <nv_tga.h>
#include <shared/data_path.h>
#include <shared/quitapp.h>

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

int map_size = 512;
int width, height;
float aspect;
float znear = 1.0f, zfar = 10.0f;
float fov = 60.0f;

GLuint noiseid;
GLuint pictid;
GLuint gridtileid;
GLuint rendered_textureid;
GLuint rendered_texture_rectid;

bool b[256];

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void menu(int entry) { key((unsigned char)entry, 0, 0); }
void idle();

// my functions
void init_opengl();

void GLErrorReport()
{
    GLuint errnum;
    const char *errstr;
    while ((errnum = glGetError())) 
    {
        errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
        printf(errstr);
    }
    return;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize(512,512);
	glutCreateWindow("NV_texture_shader demo - Offset Texture 2D / Offset Texture Rectangle using 2D texture dsdt");

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -2;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);

	glutCreateMenu(menu);
	glutAddMenuEntry("toggle wireframe [w]", 'w');
    glutAddMenuEntry("toggle texture rectangle [t]", 't');
	glutAddMenuEntry("quit [esc]", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

extern float turbulence2(float *v, float freq);
extern int B;
extern int BM;

static float clamp_0_to_1(float x)
{
    x = ( x > 1.0) ? 1.0 : x;
    x = ( x < 0.0) ? 0.0 : x;
    return x;
}

void make_noise( GLubyte *dsdt, int width, int height, int depth, float ds_freq, float dt_freq  )
{
    float v[2];

    B = width;
    BM = width - 1;

    unsigned int tid = 0;
    for ( int k = 0; k < depth; k++ )
    {
        for ( int j = 0; j < height; j++ )
        {
            for ( int i = 0; i < width; i++ )
            {
                unsigned int t = 2*tid;
                v[0] = (j * width) / float(width);
                v[1] = i/float(height);
                
                dsdt[t+0] = (GLubyte)(255 * clamp_0_to_1(turbulence2(v, ds_freq)));
                dsdt[t+1] = (GLubyte)(255 * clamp_0_to_1(turbulence2(v, dt_freq)));
                tid++;
            }
        }
        ds_freq /= 1.5f;
        dt_freq /= 1.5f;
    }
}

void load_tex(std::string & filename)
{
    tga::tgaImage * tga = tga::read(filename.c_str()); //自己写的实现 可能不正确

	if (tga == NULL) {
		fprintf(stderr, "could not open: %s\n", filename.c_str());
		quitapp(1);
	}

    glGenTextures(1,&pictid);
    glBindTexture(GL_TEXTURE_2D, pictid);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 
               GL_RGBA, 
               tga->width, tga->height,
               GL_RGBA, 
               GL_UNSIGNED_BYTE, tga->pixels);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//need to delete the tga
}

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(.25f, .25f, .25f, 1);

    if (!glh_init_extensions("GL_ARB_multitexture GL_VERSION_1_2 GL_NV_texture_shader "))
    {
	printf("Necessary extensions are not supported by your video card!\n");
	quitapp(-1);
    }

    if (!glh_extension_supported("GL_EXT_texture_rectangle") && !glh_extension_supported("GL_NV_texture_rectangle"))
    {
        printf("nnooooo\n");
        quitapp(-1);
    }

    // make 3D noise
    int width  = 128;
    int height = 128;
    int depth = 2; // we put 2 textures into one texture, ie. 0,0,128,128 and 128,0,256,128
    float ds_noise = 10.0f;
    float dt_noise = 11.0f;
    GLubyte *dsdt_noise = new GLubyte[width*height*depth*2];
    
    make_noise( dsdt_noise , width, height, depth, ds_noise, dt_noise );

    glGenTextures( 1, &rendered_textureid);
    glGenTextures( 1, &rendered_texture_rectid);
    glGenTextures( 1, &gridtileid);
    glGenTextures( 1, &noiseid );
    glBindTexture( GL_TEXTURE_2D, noiseid );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    glTexImage2D( GL_TEXTURE_2D, 0,	
                   GL_DSDT_NV,
                   width * depth, height, 0,
                   GL_DSDT_NV, GL_UNSIGNED_BYTE, dsdt_noise );
    
    delete [] dsdt_noise;

    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA/textures/2D");
    media.path.push_back("../../../../MEDIA/textures/2D");
    media.path.push_back("../../../../../../../MEDIA/textures/2D");

    std::string filename = media.get_file("nvlogo.tga");
    load_tex(filename);

    filename = media.get_file("gridtile.tga");
    tga::tgaImage * tga = tga::read(filename.c_str());

	if (tga == NULL) {
		fprintf(stderr, "could not open: %s\n", filename.c_str());
		quitapp(1);
	}

    glBindTexture(GL_TEXTURE_2D, gridtileid);

    glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA, tga->width, tga->height, 0, GL_RGB, 
               GL_UNSIGNED_BYTE, tga->pixels);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27) exit(0);
	glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

void display()
{
    static float angle = 0.0f;
    static float scale = 1.0f;
    static float scale_inc = .2f;
    static float mat2d[4];
    static float offset = 0.0;

    // animate the noise texture map
    offset += 0.5;
    if (offset > 1.0)
        offset = 0.0;

    width = reshaper.width;
    height = reshaper.height;

    angle+= .15;
    scale+= scale_inc;
    if (scale > 4.0f || scale < -4.0f)
        scale_inc = -scale_inc;
    // simple 2d rotation matrix
    mat2d[0] = cos(angle) * scale / 16.0f;
    mat2d[1] = -sin(angle) * scale / 16.0f;
    mat2d[2] = -mat2d[1];
    mat2d[3] = mat2d[0];
    if (b['t'] || b['T'])
    {
        glViewport( 0, 0, width, height);
    }
    else
    {
        // try to maximize
        while (width > map_size * 2 || height > map_size * 2)
            map_size = map_size << 1;

        // try to minimize
        while (width < map_size || height < map_size)
            map_size = map_size >> 1;

        glViewport( 0, 0, map_size, map_size);
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, 1.0, znear, zfar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTextureARB( GL_TEXTURE0_ARB );
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, pictid );

    camera.apply_inverse_transform();

	object.apply_transform();

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glColor3f(1.,1.,1.);
    glutSolidTeapot(.6);

    // 2nd pass
    static GLubyte * buf = 0;
    static int map_width = 0;
    static int map_height = 0;
    static int tex_rect_width = 0;
    static int tex_rect_height = 0;
    
    if (b['w'] || b['W'])
    {
    }
    else if (b['t'] || b['T'])
    {
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, rendered_texture_rectid);
        if (tex_rect_width != width || tex_rect_height != height)
        {
            tex_rect_width = width;
            tex_rect_height = height;
            if (buf)
                buf = (GLubyte *)realloc(buf,tex_rect_width*tex_rect_height*4);
            else
                buf = (GLubyte *)malloc(tex_rect_width*tex_rect_height*4);
            glReadPixels(0, 0, tex_rect_width, tex_rect_height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
            glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA,
                  tex_rect_width, tex_rect_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
            glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, tex_rect_width, tex_rect_height);

        mat2d[0] *= tex_rect_width;
        mat2d[1] *= tex_rect_height;
        mat2d[2] *= tex_rect_width;
        mat2d[3] *= tex_rect_height;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, rendered_textureid);
        if (map_width != map_size || map_height != map_size)
        {
            map_width = map_size;
            map_height = map_size;
            if (buf)
                buf = (GLubyte *)realloc(buf,map_width*map_height*4);
            else
                buf = (GLubyte *)malloc(map_width*map_height*4);
            glReadPixels(0, 0, map_width, map_height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                  map_width, map_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, map_width, map_height);
    }

    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_SHADER_NV);

	// stage 0 -- displacement 2D map - dsdt
	glActiveTextureARB( GL_TEXTURE0_ARB );
    glBindTexture( GL_TEXTURE_2D, noiseid );
	// stage 1 -- offset texture 2D
	glActiveTextureARB( GL_TEXTURE1_ARB );
    if (b['w'] || b['W'])
    {
        glBindTexture(GL_TEXTURE_2D, gridtileid);
        nvparse(
            "!!TS1.0                                           \n"
            "texture_2d(); // gridtile texture                  \n"
            "offset_2d(tex0, 1.0, 0.0, 0.0, 1.0);              \n"
            "nop();                                            \n"
            "nop();                                            \n"
            );
    }
    else
    {
        if (b['t'] || b['T'])
        {
            glBindTexture( GL_TEXTURE_RECTANGLE_NV, rendered_texture_rectid );
            nvparse(
                "!!TS1.0                                           \n"
                "texture_2d(); // noise texture                    \n"
                "offset_rectangle(tex0, 1.0, 0.0, 0.0, 1.0);       \n"
                "nop();                                            \n"
                "nop();                                            \n"
                );
        }
        else
        {
            glBindTexture( GL_TEXTURE_2D, rendered_textureid );
            nvparse(
                "!!TS1.0                                           \n"
                "texture_2d(); // noise texture                    \n"
                "offset_2d(tex0, 1.0, 0.0, 0.0, 1.0);              \n"
                "nop();                                            \n"
                "nop();                                            \n"
                );
        }
    }

	glActiveTextureARB( GL_TEXTURE1_ARB );
    glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat2d);

    // draw...
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0 + offset,0);
    
    glMultiTexCoord2iARB(GL_TEXTURE1_ARB, 0,0);
	glVertex2f(-1,-1);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0 + offset,1);
    if (b['w'] || b['W'])
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0,10);
    else if (b['t'] || b['T'])
        glMultiTexCoord2iARB(GL_TEXTURE1_ARB, 0,tex_rect_height);
    else
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0,1);
	glVertex2f(-1, 1);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, .5 + offset,1);
    if (b['w'] || b['W'])
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 10,10);
    else if (b['t'] || b['T'])
        glMultiTexCoord2iARB(GL_TEXTURE1_ARB, tex_rect_width, tex_rect_height);
    else
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1,1);
	glVertex2f( 1, 1);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, .5,0);
    if (b['w'] || b['W'])
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 10,0);
    else if (b['t'] || b['T'])
        glMultiTexCoord2iARB(GL_TEXTURE1_ARB, tex_rect_width, 0);
    else
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1,0);
	glVertex2f( 1,-1);
	glEnd();
    
    glActiveTextureARB( GL_TEXTURE1_ARB );
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_SHADER_NV);

    glutSwapBuffers();
}
