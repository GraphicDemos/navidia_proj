#if defined(WIN32)
#  include <windows.h>
#  pragma warning (disable : 4786)
#endif

#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <shared/data_path.h>
#include <shared/objload.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

using namespace glh;

void display();
void idle();
void resize(int w, int h);
void key(unsigned char k, int x, int y);
void special(int k, int x, int y);
void init_opengl();
void menu(int i) { key((unsigned char) i, 0, 0); }

GLhandleARB vertexLighting = 0;
GLhandleARB fragmentLighting = 0;
GLhandleARB curProgram = 0;

// data for obj-format load
unsigned int nverts = 0;
unsigned int nindices = 0;
unsigned int *indices = NULL;
float *vertexdata = NULL;
float *normaldata = NULL;
float *tangendata = NULL;
float *binormdata = NULL;
float *texcoords = NULL;

glut_simple_mouse_interactor object;
glut_callbacks cb;
bool b[256];

void cleanExit(int exitval)
{
    if (vertexLighting) 
        glDeleteObjectARB(vertexLighting);
    if (fragmentLighting) 
        glDeleteObjectARB(fragmentLighting);

    if(exitval == 0) { exit(0); }
    else { quitapp(exitval); }
}

void printInfoLog(GLhandleARB object)
{
    int maxLength = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

    char *infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);

    printf("%s\n", infoLog);
}

void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType, bool linkProgram)
{
    assert(programObject != 0);
    assert(shaderSource != 0);
    assert(shaderType != 0);

    GLhandleARB object = glCreateShaderObjectARB(shaderType);
    assert(object != 0);

    GLint length = (GLint)strlen(shaderSource);
    glShaderSourceARB(object, 1, &shaderSource, &length);

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

    if (linkProgram)
    {
        glLinkProgramARB(programObject);

        GLint linked = false;
        glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
        if (!linked)
        {
            printInfoLog(programObject);
            cout << "Shaders failed to link, exiting..." << endl;
            cleanExit(-1);
        }
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    if(!glh_init_extensions("GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader"))
	{
        cout << "Necessary extensions unsupported: " << glh_get_unsupported_extensions() << endl;
        quitapp(-1);
	}

    // define search path for simple_lighting.glsl file
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    string filename = media.get_file("programs/glsl_simple_lighting/vertex_lighting.glsl");
    if (filename == "")
    {
        printf("Unable to locate vertex_lighting.glsl, exiting...\n");
        cleanExit(-1);
    }

    // load and create vertex lighting program
    vertexLighting = glCreateProgramObjectARB();

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(vertexLighting, shaderData, GL_VERTEX_SHADER_ARB, true);

    filename = media.get_file("programs/glsl_simple_lighting/passthrough_vp.glsl");
    if (filename == "")
    {
        printf("Unable to locate passthrough_vp.glsl, exiting...\n");
        cleanExit(-1);
    }

    // load and create fragment lighting program
    fragmentLighting = glCreateProgramObjectARB();

    shaderData = read_text_file(filename.c_str());
    addShader(fragmentLighting, shaderData, GL_VERTEX_SHADER_ARB, false);

    filename = media.get_file("programs/glsl_simple_lighting/fragment_lighting.glsl");
    if (filename == "")
    {
        printf("Unable to locate fragment_lighting.glsl, exiting...\n");
        cleanExit(-1);
    }

    shaderData = read_text_file(filename.c_str());
    addShader(fragmentLighting, shaderData, GL_FRAGMENT_SHADER_ARB, true);

    // default to using vertex lighting program
    curProgram = vertexLighting;
    
    // import external geometry
    filename = media.get_file("models/Rayguns/Raygun_01.OBJ");
    if (filename == "")
    {
        printf("Unable to locate Raygun_01.OBJ, exiting...\n");
        cleanExit(-1);
    }

    if (!LoadObjModel(filename.c_str(),nverts,nindices,indices,vertexdata,normaldata,tangendata,binormdata,texcoords))
    {
        printf("Unable to load Raygun_01.OBJ, exiting...\n");
        cleanExit(-1);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.apply_transform();
    glRotatef(-15, 1.0, 0.0, 0.0);
    glRotatef( 25, 0.0, 1.0, 0.0);

    glUseProgramObjectARB(curProgram);

    glUniform3fARB(glGetUniformLocationARB(curProgram, "lightVec"), 0.0, 0.0, 1.0);
 
    glPushMatrix();
    glVertexPointer(3, GL_FLOAT, 0, vertexdata);
    glNormalPointer(GL_FLOAT, 0, normaldata);
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, indices);
    glPopMatrix();

    glUseProgramObjectARB(0);

    glutSwapBuffers();
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if(k==27 || k=='q') cleanExit(0);

    if (k == 'w' || k == 'W')
    {
        if (b[k])
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (k == '1')
    {
        curProgram = vertexLighting;
        glutSetWindowTitle("Simple lighting: Per-vertex");
    }
     
    if (k == '2')
    {
        curProgram = fragmentLighting;
        glutSetWindowTitle("Simple lighting: Per-fragment");
    }

    glutPostRedisplay();
}

void special(int k, int x, int y)
{
    glutPostRedisplay();
}

void idle()
{
    if (b[' '])
        object.trackball.increment_rotation();

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
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Simple lighting: Per-vertex");
    
    init_opengl();

    glutCreateMenu(menu);
    glutAddMenuEntry("Vertex lighting [1]", '1');
    glutAddMenuEntry("Fragment lighting [2]", '2');
    glutAddMenuEntry("Quit [q]", 'q');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glut_helpers_initialize();

    object.configure_buttons(1);
	b['t'] = true;
    object.dolly.dolly[2] = -5;

    cb.keyboard_function = key;
    cb.special_function = special;
	cb.display_function = display;
	cb.reshape_function = resize;

    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glutIdleFunc(idle);

    b[' '] = true;

    glutMainLoop();

    return 0;
}
