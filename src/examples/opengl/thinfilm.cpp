/*********************************************************************NVMH3****

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

******************************************************************************/

#if defined(WIN32)
#  include <windows.h>
#  pragma warning (disable : 4786)
#endif

#if defined(UNIX)
#define GL_GLEXT_PROTOTYPES
#endif

#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_linear.h>

#include <shared/data_path.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

#include "nvbHelper.h"

using namespace glh;

// key mapping
bool b[256];

glut_simple_mouse_interactor object;

bool show_wireframe;
bool animate = true;

GLhandleARB programObject = 0;

GLint lightVectorParam = -1;
GLint filmDepthParam = -1;
GLint eyeVectorParam = -1;

nvbHelper nvbModel;

GLuint create_fringe()
{
	unsigned char data[256*3];
	
	// these lambdas are in 100's of nm,
	//  they represent the wavelengths of light for each respective 
	//  color channel.  They are only approximate so that the texture
	//  can repeat.

	float lamR = 6;  // 600 nm
	float lamG = 5;  // 500 nm, should be more like 550
	float lamB = 4;  // 400 nm. should be more like 440

	// these offsets are used to perturb the phase of the interference
	//   if you are using very thick "thin films" you will want to
	//   modify these offests to avoid complete contructive interference
	//   at a particular depth.. Just a tweak able.
	float offsetR = 0;
	float offsetG = 0;
	float offsetB = 0;

	// p is the period of the texture, it is the LCM of the wavelengths,
	//  this is the depth in nm when the pattern will repeat.  I was too
	//  lazy to write up a LCM function, so you have to provide it.
	float p = 60;   //lcm(6,5,4)

	// vd is the depth of the thin film relative to the texture index
	float vd = 1/256.0 * p;

	// now compute the color values using this formula:
	//  1/2 ( Sin( 2Pi * d/lam* + Pi/2 + O) + 1 )
	//   where d is the current depth, or "i*vd" and O is some offset* so that
	//   we avoid complete constructive interference in all wavelenths at some depth.
	float pi = 3.1415926535f;
	for(int i=0; i<256; ++i)
	{
		data[i*3 + 0] = (unsigned char)((.5*(sin(2*pi*(i*vd)/lamR + pi/2.0 + offsetR) + 1))*255);
		data[i*3 + 1] = (unsigned char)((.5*(sin(2*pi*(i*vd)/lamG + pi/2.0 + offsetG) + 1))*255);
		data[i*3 + 2] = (unsigned char)((.5*(sin(2*pi*(i*vd)/lamB + pi/2.0 + offsetB) + 1))*255);
	}

	// Now just load the texture, I use mipmapping since the depth may change very
	// fast in places.

    GLuint id;
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB8,
		256,
		1,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		data);

    return id;
}

void cleanExit(int exitval)
{
    if (programObject) 
        glDeleteObjectARB(programObject);

    if(exitval == 0) { exit(0); }
    else { quitapp(exitval); }
}

void printInfoLog(GLhandleARB object)
{
    int maxLength = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

    char *infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);

    cout << infoLog << endl;
}

void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType)
{
    assert(programObject != 0);
    assert(shaderSource != 0);
    assert(shaderType != 0);

    GLhandleARB object = glCreateShaderObjectARB(shaderType);
    assert(object != 0);

    glShaderSourceARB(object, 1, &shaderSource, NULL);

    // compile vertex shader object
    glCompileShaderARB(object);

    // check if shader compiled
    GLint compiled = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (!compiled)
    {
        printInfoLog(object);
        cleanExit(-1);
    }

    // attach vertex shader to program object
    glAttachObjectARB(programObject, object);

    // delete vertex object, no longer needed
    glDeleteObjectARB(object);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void init()
{
    if(!glh_init_extensions("GL_VERSION_1_2 GL_ARB_shader_objects GL_ARB_vertex_program GL_ARB_vertex_shader GL_ARB_fragment_shader GL_SGIS_generate_mipmap"))
    {
        cout << "Necessary extensions unsupported: " <<  glh_get_unsupported_extensions() << endl;
        quitapp(-1);
    }

    programObject = glCreateProgramObjectARB();

    // Set data paths...
    data_path media;
  	media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    string filename = media.get_file("programs/glsl_thinfilm/thinfilm_vertex.glsl");
    if (filename == "")
    {
        printf("Unable to load thinfilm_vertex.glsl, exiting...\n");
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    filename = media.get_file("programs/glsl_thinfilm/thinfilm_fragment.glsl");
    if (filename == "")
    {
        printf("Unable to load thinfilm_fragment.glsl, exiting...\n");
        cleanExit(-1);
    }

    shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_FRAGMENT_SHADER_ARB);

    delete [] shaderData;

    glLinkProgramARB(programObject);

    GLint linked = false;
    glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked)
    {
        printInfoLog(programObject);
        cout << "Shaders failed to link, exiting..." << endl;
        cleanExit(-1);
    }

    glValidateProgramARB(programObject);

    GLint validated = false;
    glGetObjectParameterivARB(programObject, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
    if (!validated)
    {
        printInfoLog(programObject);
        cout << "Shaders failed to validate, exiting..." << endl;
        cleanExit(-1);
    }

    // get uniform locations
    lightVectorParam = glGetUniformLocationARB(programObject, "lightVector");
    assert(lightVectorParam >= 0);

    eyeVectorParam = glGetUniformLocationARB(programObject, "eyeVector");
    assert(eyeVectorParam >= 0);

    filmDepthParam = glGetUniformLocationARB(programObject, "filmDepth");
    assert(filmDepthParam >= 0);

    glUseProgramObjectARB(programObject);

    glUniform1iARB(glGetUniformLocationARB(programObject, "diffuseMap"), 0); 
    glUniform1iARB(glGetUniformLocationARB(programObject, "fringeMap"), 1); 

    // bind fringe map to second texture unit
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, create_fringe());

    // diffuse textures from model go to first texture unit
    glActiveTexture(GL_TEXTURE0);

    // load model
    string modelPath = media.get_path("models/UFO-01/UFO-01-NO-GLOW.nvb");
    if (modelPath == "")
    {
        cout << "Unable to locate UFO model, exiting..." << endl;
        cleanExit(-1);
    }

    nvbModel.Load(modelPath, "UFO-01-NO-GLOW.nvb");    

    glEnable(GL_DEPTH_TEST);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void display()
{
    glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.apply_transform();

    glRotatef(30, 1.0, 0.0, 0.0);
    glRotatef(10, 0.0, 0.0, 1.0);
    
	glUniform3fARB(lightVectorParam, 0.38f, 0.73f, 0.57f);
	glUniform3fARB(eyeVectorParam, 0.0f, 0.0f, 1.0f);
	glUniform1fARB(filmDepthParam, 0.63f);
	
    nvbModel.Draw();

    glutSwapBuffers();
}

void keyboard( unsigned char k, int x, int y )
{
	b[k] = !b[k];

    if (k == 27 || k == 'q' || k == 'Q') 
        cleanExit(0);

    if (k == 'w' || k == 'W')
    {
        show_wireframe = !show_wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, show_wireframe ? GL_LINE : GL_FILL);
    }

    if (k == ' ')
        animate = !animate;
        
	glutPostRedisplay();
}

void idle()
{
    if (animate)
        object.trackball.increment_rotation();

    glutPostRedisplay();
}

void menu(int k)
{
	keyboard((unsigned char)k, 0, 0);
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);

    object.reshape(w, h);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(600, 500);
    glutCreateWindow("Thin Film");

    // Do some OpenGL initialization.
	init();

    glut_helpers_initialize();

    object.configure_buttons(1);
    object.dolly.dolly[0] = -0.1;
    object.dolly.dolly[1] =  0.4;
    object.dolly.dolly[2] = -15;

    glut_add_interactor(&object);
    
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
	glutDisplayFunc(display);

	glutCreateMenu(menu);
    glutAddMenuEntry("Toggle wireframe [w]", 'w');
    glutAddMenuEntry("Toggle animation [ ]", ' ');
	glutAddMenuEntry("Quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Give control over to glut.
	glutMainLoop();
    
	return 0;
}
