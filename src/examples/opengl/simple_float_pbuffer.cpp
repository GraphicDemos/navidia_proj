#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <shared/data_path.h>
#include <shared/quitapp.h>
#include <nv_dds.h>

using namespace nv_dds;

#if defined(WIN32)
#include "float_pbuffer_win.h"
#define REQUIRED_EXTENSIONS "GL_ARB_texture_compression " \
                            "GL_EXT_texture_compression_s3tc " \
                            "GL_NV_texture_rectangle " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GL_NV_float_buffer " \
                            "WGL_ARB_pixel_format " \
                            "WGL_ARB_pbuffer "
#elif defined(UNIX)
#include "float_pbuffer_lnx.h"
#define REQUIRED_EXTENSIONS "GL_ARB_texture_compression " \
                            "GL_EXT_texture_compression_s3tc " \
                            "GL_NV_texture_rectangle " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GLX_NV_float_buffer " \
                            "GLX_SGIX_fbconfig " \
                            "GLX_SGIX_pbuffer "
#endif

// function definitions
void init_opengl();
void reshape(int w, int h);
void display(void);
void keyboard(unsigned char key, int x, int y);
void idle();

// variables 
CFPBuffer fpbuffer;
GLuint fptexture;
GLuint logotexture;

GLuint readTextureRECT;
GLuint readTexture2D;

void init_opengl()
{
    if (!glh_init_extensions(REQUIRED_EXTENSIONS))
    {
        printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
               glh_get_unsupported_extensions());
        quitapp(-1);
    }

    glClearColor(0.2, 0.2, 0.2, 1.0);

    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA/textures/2D");
    media.path.push_back("../../../../MEDIA/textures/2D");

    if (!fpbuffer.create(256, 256))
        quitapp(-1);
    
    fpbuffer.activate();
    reshape(256, 256);
    glClearColor(0.5, 0.6, 0.7, 1.0);
    fpbuffer.deactivate();
    
    glGenTextures(1, &logotexture);
    glBindTexture(GL_TEXTURE_2D, logotexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    CDDSImage image;
    if (image.load(media.get_file("nvlogo.dds")))
        image.upload_texture2D();

    glGenTextures(1, &fptexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, fptexture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);
    
    const char readTexRect[] =
    "!!FP1.0\n"
    "TEX o[COLR], f[TEX0].xyxx, TEX0, RECT;\n"
    "END\n";

    glGenProgramsNV(1, &readTextureRECT);
    glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, readTextureRECT);
    glLoadProgramNV(GL_FRAGMENT_PROGRAM_NV, readTextureRECT, strlen(readTexRect), (const GLubyte *)readTexRect);

    const char readTex2D[] =
    "!!FP1.0\n"
    "TEX R0, f[TEX0].xyxx, TEX0, 2D;\n"
    "MUL o[COLR], f[COL0], R0;\n"
    "END\n";

    glGenProgramsNV(1, &readTexture2D);
    glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, readTexture2D);
    glLoadProgramNV(GL_FRAGMENT_PROGRAM_NV, readTexture2D, strlen(readTex2D), (const GLubyte *)readTex2D);
}

void display(void)
{
    static float rot = 0.0;
    int width = fpbuffer.getWidth();
    int height = fpbuffer.getHeight();

    fpbuffer.activate();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw rotating quad with nvidia logo
    glColor3f(1.0, 1.0, 1.0);
    glRotatef(rot, 0.0, 0.0, 1.0); 
    glTranslatef(-0.5, -0.5, -1.3);

    glEnable(GL_FRAGMENT_PROGRAM_NV);
    glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, readTexture2D);
    glBindTexture(GL_TEXTURE_2D, logotexture);
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
        glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
    glEnd();
    
    glDisable(GL_FRAGMENT_PROGRAM_NV);

    // copy contents of pbuffer to a texture
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, fptexture);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, width, height);
    fpbuffer.deactivate();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // draw a single quad with the texture containing a snapshot of the 
    // floating point pbuffer
    glEnable(GL_FRAGMENT_PROGRAM_NV);
    glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, readTextureRECT);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, fptexture);
    
    glTranslatef(-0.5, -0.5,-0.9);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);         glVertex2f(0.0, 0.0);
        glTexCoord2f(width, 0.0);       glVertex2f(1.0, 0.0);
        glTexCoord2f(width, height);    glVertex2f(1.0, 1.0);
        glTexCoord2f(0.0, height);      glVertex2f(0.0, 1.0);
    glEnd();
    
    glDisable(GL_FRAGMENT_PROGRAM_NV);

    rot += 0.5;
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) 
    {
        case 27:
        case 'q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Simple Floating Point PBuffer");
    
    init_opengl();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    
    glutMainLoop();

    return 0;
}
