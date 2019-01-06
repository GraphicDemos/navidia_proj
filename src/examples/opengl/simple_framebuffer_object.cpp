/*
    Simple framebuffer object (FBO) example

    This example demonstrates how to use the framebuffer object extension to
    perform simple rendering to a texture in OpenGL.

    http://www.nvidia.com/dev_content/nvopenglspecs/GL_EXT_framebuffer_object.txt
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor object;

GLuint fb;  // color render target
GLuint depth_rb; // depth render target
GLuint stencil_rb; // depth render target
GLuint tex; // texture

// texture dimensions
int texWidth  = 256;
int texHeight = 256;
GLenum texInternalFormat = GL_RGBA8;
GLenum texTarget = GL_TEXTURE_2D;
GLenum filterMode = (texTarget == GL_TEXTURE_RECTANGLE_NV) ? GL_NEAREST : GL_LINEAR;
int maxCoordS = (texTarget == GL_TEXTURE_RECTANGLE_NV) ? texWidth : 1;
int maxCoordT = (texTarget == GL_TEXTURE_RECTANGLE_NV) ? texHeight : 1;
GLuint textureProgram = 0, renderProgram = 0;
float teapot_rot = 0.0f;

#define GET_GLERROR(ret)                                          \
{                                                                 \
    GLenum err = glGetError();                                    \
    if (err != GL_NO_ERROR) {                                     \
    fprintf(stderr, "[%s line %d] GL Error: %s\n",                \
    __FILE__, __LINE__, gluErrorString(err));                     \
    fflush(stderr);                                               \
    }                                                             \
}

void CheckFramebufferStatus()
{
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, duplicate attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            break;
        default:
            assert(0);
    }
}

void init_opengl()
{
     if (!glh_init_extensions("GL_ARB_fragment_program "
                              "GL_ARB_vertex_program "
                              "GL_NV_float_buffer "
                              "GL_EXT_framebuffer_object "))
     {
         printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
                glh_get_unsupported_extensions());
         quitapp(-1);
     }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    glGenFramebuffersEXT(1, &fb);
    glGenTextures(1, &tex);
    glGenRenderbuffersEXT(1, &depth_rb);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);    

    // init texture
    glBindTexture(texTarget, tex);
    glTexImage2D(texTarget, 0, texInternalFormat, texWidth, texHeight, 0, 
                 GL_RGBA, GL_FLOAT, NULL);
    GET_GLERROR(NULL);
    glTexParameterf(texTarget, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameterf(texTarget, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameterf(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                              texTarget, tex, 0);

    GET_GLERROR(0);

    // initialize depth renderbuffer
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, texWidth, texHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
                                 GL_RENDERBUFFER_EXT, depth_rb);

    GET_GLERROR(0);

    CheckFramebufferStatus();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // load fragment programs
    const char* strTextureProgram2D = 
        "!!ARBfp1.0\n"
        "TEX result.color, fragment.texcoord[0], texture[0], 2D;\n"
        "END\n";

    const char* strTextureProgramRECT = 
        "!!ARBfp1.0\n"
        "TEX result.color, fragment.texcoord[0], texture[0], RECT;\n"
        "END\n";

    glGenProgramsARB(1, &textureProgram);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, textureProgram);
    // load correct program based on texture target
    if (texTarget == GL_TEXTURE_RECTANGLE_NV) {
        glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        (GLsizei)strlen(strTextureProgramRECT), strTextureProgramRECT);
    } else {
        glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        (GLsizei)strlen(strTextureProgram2D), strTextureProgram2D);
    }

    GET_GLERROR(0);

    const char* strRenderProgram = 
        "!!ARBfp1.0\n"
        "MOV result.color, fragment.color;\n"
        "END\n";
    
    glGenProgramsARB(1, &renderProgram);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, renderProgram);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, 
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       (GLsizei)strlen(strRenderProgram), strRenderProgram);

    GET_GLERROR(0);
}

void cleanup()
{
    // destroy objects
    glDeleteRenderbuffersEXT(1, &depth_rb);
    glDeleteTextures(1, &tex);
    glDeleteFramebuffersEXT(1, &fb);

    glDeleteProgramsARB(1, &textureProgram);
    glDeleteProgramsARB(1, &renderProgram);
}

void display()
{
    // render to the render target texture first
    glBindTexture(texTarget, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    {
        glPushAttrib(GL_VIEWPORT_BIT); 
        glViewport(0, 0, texWidth, texHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -1.5);
        glRotatef(teapot_rot, 0.0, 1.0, 0.0);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, renderProgram);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);

        glColor3f(0.0, 1.0, 0.0);
        glutWireTeapot(0.5f);

        glPopMatrix();
        glPopAttrib();
    }
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // now render to the screen using the texture...
    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    object.apply_transform();

    // draw textured quad
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, textureProgram);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    glBindTexture(texTarget, tex);
    glEnable(texTarget);  

    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0,         0);         glVertex2f(-1, -1);
        glTexCoord2f(maxCoordS, 0);         glVertex2f( 1, -1);
        glTexCoord2f(maxCoordS, maxCoordT); glVertex2f( 1,  1);
        glTexCoord2f(0,         maxCoordT); glVertex2f(-1,  1);
    }
    glEnd();

    glPopMatrix();
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    glutSwapBuffers();
}

void idle()
{
    if (b[' ']) {
        object.trackball.increment_rotation();
        teapot_rot += 0.5;
    }

    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
    if(k==27 || k=='q') {
        cleanup();
        exit(0);
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
    
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
    object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    object.motion(x, y);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("Simple Framebuffer Object");

	init_opengl();

    object.configure_buttons(1);
    object.dolly.dolly[2] = -2;

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    b[' '] = true;

	glutMainLoop();

	return 0;
}
