#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <sys/time.h>
#  define GL_GLEXT_PROTOTYPES
#endif

#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_genext.h>
#include <glh/glh_glut.h>
#include <glh/glh_obs.h>

#include <shared/load_cubemap.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

// Parameters for the surface.
float *vertices = NULL;
unsigned int nverts;
float fstart;
int   dwGridSize;
float fGridStep;
float fGridExtent;

#if defined(WIN32)
UINT64 qwStartTicks = 0;
UINT64 qwTicksPerSec = 0;
#else
float StartTicks=0;
#endif

float t = 0;

bool animate = true;
float sineCounter = 1;
float frustumAngle = 90;
float camHeight = 4;
float angleY = 10;

float angle = 0;

GLhandleARB programObject;

GLint timeParam;
GLint waveDataParam;

#define NUM_WAVES 5

// environment map
tex_object_cube_map cubemap;
tex_object_cube_map cubemapDark;

void init_opengl();
void keyboard(unsigned char k, int x, int y);
void display();
void menu(int entry);
void idle();
void buildGeometry();
    
void mouseEvent(int button, int state, int x, int y);
void mouseMove(int x, int y);

void reshape(int w, int h)
{
    glViewport(0,0,(GLsizei)w,(GLsizei)h);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutCreateWindow("GLSL Vertex water");

    // Do some OpenGL initialization.
    init_opengl();

    glut_helpers_initialize();

    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    camera.pan.pan = vec3f(0.0, 1.0, 0.0);
    object.configure_buttons(1);
    object.dolly.dolly[2] = -1;

    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMove);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutCreateMenu(menu);
    glutAddMenuEntry("Toggle Wireframe [w]", 'w' );
    glutAddMenuEntry("Increase Frustum Angle [+]", '[');
    glutAddMenuEntry("Decrease Frustum Angle [-]", ']');
    glutAddMenuEntry("Increase Camera Height [a]", 'a');
    glutAddMenuEntry("Decrease Camera Height [z]", 'z');
    glutAddMenuEntry("Toggle Animation [ ]", ' ');
    glutAddMenuEntry("quit [esc]", 27);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
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

    printf("%s\n", infoLog);
}

void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType)
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

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void init_opengl()
{
    if(!glh_init_extensions("GL_VERSION_1_4 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_EXT_texture_filter_anisotropic"))
	{
        cout << "Necessary extensions unsupported: " << glh_get_unsupported_extensions() << endl;
        quitapp(-1);
	}

    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA/programs");
    media.path.push_back("../../../../MEDIA/programs");

    programObject = glCreateProgramObjectARB();

    string filename = media.get_file("glsl_vertex_water/water.glsl");
    if (filename == "")
    {
        cout << "Unable to locate water.glsl, exiting..." << endl;
        cleanExit(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    filename = media.get_file("glsl_vertex_water/water_fragment.glsl");
    if (filename == "")
    {
        cout << "Unable to locate water_fragment.glsl, exiting..." << endl;
        quitapp(-1);
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

    timeParam = glGetUniformLocationARB(programObject, "Time");
    assert(timeParam >= 0);

    // Load the cube map.
    cubemap.bind();
    load_png_cubemap("Terrain_%s_0001.png", true);
    cubemap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    cubemap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cubemap.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f );

    // Load the cube map.
    cubemapDark.bind();
    load_png_cubemap("Terrain_%s_0001_dark.png", true);
    cubemapDark.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    cubemapDark.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cubemapDark.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);

    glUseProgramObjectARB(programObject);

    glUniform1iARB(glGetUniformLocationARB(programObject, "environmentMap"), 0); 
    glUniform1iARB(glGetUniformLocationARB(programObject, "environmentMapDark"), 1); 

    glUseProgramObjectARB(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;

    // Initialize timing-keeping parameters.
#if defined(WIN32)
    if(!QueryPerformanceFrequency((LARGE_INTEGER *) &qwTicksPerSec))
        qwTicksPerSec = 1000;
#endif

    buildGeometry();

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glEnable(GL_DEPTH_TEST);
}

int mouseDown = 0;
int oldX, oldY;
void mouseEvent(int button, int state, int x, int y)
{
    mouseDown = !state;
    oldX = x;
    oldY = y;
}

void mouseMove(int x, int y)
{
    if (mouseDown)
    {
        angle += (float)(x - oldX) / 10;
        angleY -= (float)(y - oldY) / 10;
        oldX = x;
        oldY = y;
        if (angleY > 90)
            angleY = 90;
        if (angleY < -90)
            angleY = -90;
    }
}

float maxDistSq = 5;
// builds a trapezois of quds with the specified parameters
void buildCenter(float start, float end, int divisionsX, int divisionsY)
{
    float diffX = (end - start) / divisionsX;
    float diffY = (end - start) / divisionsY;
    for (float y = start+diffY; y <= end+0.0001; y+= diffY)
    {
        glBegin(GL_QUAD_STRIP);
        for (float x = y-diffX; x <= -y+0.0001+diffX; x+=diffX)
        {
            glColor4f(max(1-(x*x+(y+diffY)*(y+diffY))/maxDistSq,0.0f),1,0,0);
            glVertex3f(x,y-diffY,0);
            glColor4f(max(1-(x*x+y*y)/maxDistSq,0.0f),1,0,0);
            glVertex3f(x,y,0);
        }
        glEnd();
    }
}

// draw square subdivided into quad strips
void draw_plane(float w, float h, float centerY, int rows, int cols)
{
    int x, y;
    GLfloat vx, vy, s, t;
    GLfloat ts, tt, tw, th;

    ts = 1.0 / cols;
    tt = 1.0 / rows;

    tw = w / cols;
    th = h / rows;

    glNormal3f(0.0, 0.0, 1.0);

    for(y=0; y<rows; y++) 
    {
        glBegin(GL_QUAD_STRIP);
        for(x=0; x<=cols; x++) 
        {
            vx = tw * x -(w/2.0);
            vy = th * y -(h/2.0);
            s = ts * x;
            t = tt * y;

            glColor4f(max(1-(vx*vx+vy*vy)/maxDistSq,0.0f),1,0,0);
            glVertex3f(vx, vy + centerY, 0.0);

            glColor4f(max(1-(vx*vx+(vy + th)*(vy + th))/maxDistSq,0.0f),1,0,0);
            glVertex3f(vx, vy + th + centerY, 0.0);
        }
        glEnd();
    }

}

display_list geomList;
void buildGeometry()
{
    geomList.new_list( GL_COMPILE );
    draw_plane(12, 10, 2.5,     100, 100);
    draw_plane(20, 7.5, -6.25,  60, 160);
    buildCenter(-20, -10,       60, 30);        //10
    buildCenter(-30, -20,       40, 20);    //10
    buildCenter(-50, -30,       40, 20);    //20
    geomList.end_list();
}

void drawSkyBox(void)
{
    cubemap.bind();
    cubemap.enable();

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glScalef(1.0, 1.0, 1.0);
    
    glRotatef(angle, 0,1,0);
    glRotatef(-angleY, 1,0,0);

    glutSolidSphere(10.0, 40, 20);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
}

void populateWave(int waveSlot, vec3f direction, float frequency, float height)
{
    // create the parameters for the first wave
    float radAngle = angle * M_PI/180.;
    direction.normalize();
    vec3f nvo;
    nvo[0] = cos(radAngle) * direction[0] - sin(radAngle) * direction[1];
    nvo[1] = sin(radAngle) * direction[0] + cos(radAngle) * direction[1];

    GLint waveData = glGetUniformLocationARB(programObject, "WaveData[0]");
    assert(waveData >= 0);
    glUniform4fARB(waveData + waveSlot, nvo[0], nvo[1], frequency, height);
}

void display()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(frustumAngle*3./4., (float)4/3, 0.1, 50);
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // bind the wave data
    if (animate)
        sineCounter+= 0.1;

    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslatef(0,-1.55,0);
    drawSkyBox();
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);

    glUseProgramObjectARB(programObject);

    // create Pi-related constants 
    glUniform1fARB(timeParam, sineCounter);

    populateWave(0, vec3f(0,-1,0), .15f, 2.0f);
    populateWave(1, vec3f(-1,-1,0), .4f, .30f * (sinf(sineCounter)*.5+1));
    populateWave(2, vec3f(.5,1,0), 1.0f, .1f);
    populateWave(3, vec3f(.8,.5,0), 2.5f, .01f * (sinf(.6f*sineCounter+2)*.3+1));
    populateWave(4, vec3f(.1,.9,0), 2.0f, .01f * (sinf(.3f*sineCounter)*.5+1));

    // Bind the cube map and draw.
    glColor3f(1.0, 1.0, 1.0);

    glActiveTexture(GL_TEXTURE0);
    cubemap.bind();
    cubemap.enable();
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 1.0f);

    // set up the texure matrix for the cubemap and rotate to accomodate the viewers direction
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(angle, 0, 1, 0);
    glRotatef(-angleY, 1, 0, 0);
    glMatrixMode(GL_MODELVIEW);

    glActiveTexture(GL_TEXTURE1);
    cubemapDark.bind();
    cubemapDark.enable();

    glActiveTexture(GL_TEXTURE0);

    // rotate the geometry to accomodate the viewer looking up/down
    glPushMatrix();
    glRotatef(angleY, 1, 0, 0);
    glTranslatef(0, -camHeight, 0);

    // draw the geometry plane
    geomList.call_list();

    glPopMatrix();

    glActiveTexture(GL_TEXTURE1);
    cubemapDark.disable();
    glActiveTexture(GL_TEXTURE0);
    cubemap.disable();

    glUseProgramObjectARB(0);

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void keyboard( unsigned char k, int x, int y )
{
    if (k == 27 || k == 'q') 
        cleanExit(0);

    if (k == 'w')
    {
        GLint mode[2];
        glGetIntegerv(GL_POLYGON_MODE, mode);
        if (mode[1] == GL_FILL)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
        }
        else if (mode[1] == GL_LINE)
        {
            glPointSize(4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        }
    }

    if (k == '=' || k == '+')
        frustumAngle += 0.5;
    if (k == '-')
        frustumAngle -= 0.5;
    if (k == 'a')
        camHeight += 0.5;
    if (k == 'z')
        camHeight -= 0.5;
    if (k == ' ')
        animate = !animate;
}

void idle()
{
    glutPostRedisplay();
}

void menu(int entry)
{
    keyboard((unsigned char)entry, 0, 0);
}

