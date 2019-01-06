#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif

#if defined(UNIX)
#define GL_GLEXT_PROTOTYPES
#endif

#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_obs.h>

#include <shared/array_texture.h>
#include <shared/data_path.h>

#include <nv_png.h>
#include <shared/bumpmap_to_normalmap.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>
 
using namespace glh;

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void idle();
void menu(int item) { key((unsigned char)item, 0, 0); }

// my functions
void init_opengl();
void draw_sphere();

GLhandleARB programObject;

GLint positionAttrib = -1;
GLint tangentBasisAttrib = -1;
GLint texcoordAttrib = -1;

GLint modelViewIParam = -1;
GLint lightParam = -1;
GLint halfangleParam = -1;

GLint parallaxMapParam = -1;

glut_simple_mouse_interactor object;
glut_perspective_reshaper reshaper;

display_list sphere;

float light_rotation = 0;

bool b[256];
bool parallaxMapping = false;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Simple Bump Mapping");

    init_opengl();

    glut_helpers_initialize();

    object.configure_buttons(1);
    object.dolly.dolly[2] = -2;

    glut_add_interactor(&object);
    glut_add_interactor(&reshaper);

    glutCreateMenu(menu);
    glutAddMenuEntry("Toggle parallax mapping [p]", 'p');
    glutAddMenuEntry("Toggle wireframe [w]", 'w');
    glutAddMenuEntry("Toggle animation [ ]", ' ');

    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);

    key(' ', 0, 0); // animate 

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

void init_opengl()
{
    if(!glh_init_extensions("GL_VERSION_1_2 GL_ARB_shader_objects GL_ARB_vertex_program GL_ARB_vertex_shader GL_ARB_fragment_shader"))
    {
        cout << "Necessary extensions unsupported: " <<  glh_get_unsupported_extensions() << endl;
        quitapp(-1);
    }

    programObject = glCreateProgramObjectARB();

    // define search path for GLSL shaders file
    data_path media;
    media.path.push_back(".");
	media.path.push_back("MEDIA");
	media.path.push_back("../MEDIA");
	media.path.push_back("../../MEDIA");
    media.path.push_back("../../../MEDIA");

    string filename = media.get_file("programs/glsl_bump_mapping/bump_mapping_vertex.glsl");
    if (filename == "")
    {
        printf("Unable to load bump_mapping_vertex.glsl, exiting...\n");
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    filename = media.get_file("programs/glsl_bump_mapping/bump_mapping_fragment.glsl");
    if (filename == "")
    {
        printf("Unable to load bump_mapping_fragment.glsl, exiting...\n");
        cleanExit(-1);
    }

    shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_FRAGMENT_SHADER_ARB);

    delete [] shaderData;

    // make sure position attrib is bound to location 0
    glBindAttribLocationARB(programObject, 0, "position");

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

    // get attribute locations
    positionAttrib = glGetAttribLocationARB(programObject, "position");
    assert(positionAttrib >= 0);

    tangentBasisAttrib = glGetAttribLocationARB(programObject, "tangentBasis");
    assert(tangentBasisAttrib >= 0);

    texcoordAttrib = glGetAttribLocationARB(programObject, "texcoord");
    assert(texcoordAttrib >= 0);

    // get uniform locations
    modelViewIParam = glGetUniformLocationARB(programObject, "modelViewI");
    assert(modelViewIParam >= 0);

    lightParam = glGetUniformLocationARB(programObject, "light");
    assert(lightParam >= 0);

    halfangleParam = glGetUniformLocationARB(programObject, "halfAngle");
    assert(halfangleParam >= 0);

    parallaxMapParam = glGetUniformLocationARB(programObject, "parallaxMapping");
    assert(parallaxMapParam >= 0);

    // initialize the decal
    array2<vec3ub> img;
    read_png_rgb("earth.png", img);
    
    GLuint decal;
    glGenTextures(1, &decal);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, decal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    make_rgb_texture(img, true);

    // load height map
    array2<unsigned char> bump_img;
    read_png_grey("earth_bump.png", bump_img);
    
    // create height map texture
    GLuint heightmap;
    glGenTextures(1, &heightmap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //make_rgb_texture(bump_img, true);
    make_scalar_texture(bump_img, GL_RGB, GL_LUMINANCE, true);

    // create normal map from heightmap
    bumpmap_to_normalmap(bump_img, img, vec3f(1.0, 1.0, 0.2));

    GLuint normalmap;
    glGenTextures(1, &normalmap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    make_rgb_texture(img, true);

    glActiveTexture(GL_TEXTURE0);
    
    sphere.new_list(GL_COMPILE);
    draw_sphere();
    sphere.end_list();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    glUseProgramObjectARB(programObject);

    glUniform1iARB(glGetUniformLocationARB(programObject, "decalMap"), 0); 
    glUniform1iARB(glGetUniformLocationARB(programObject, "heightMap"), 1); 
    glUniform1iARB(glGetUniformLocationARB(programObject, "normalMap"), 2); 

    glUniform1iARB(parallaxMapParam, (int)parallaxMapping);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if(k==27 || k=='q') cleanExit(0);

    if('w' == k)
    {
        if(b[k])
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if ('p' == k)
    {
        parallaxMapping = !parallaxMapping;
        glUniform1iARB(parallaxMapParam, (int)parallaxMapping);
    }

    glutPostRedisplay();
}

void idle()
{
    if (b[' '])
        light_rotation += to_radians(1);
    glutPostRedisplay();
}

vec3f sphere_position(float theta, float phi)
{
    vec3f p;
    p[0] =   cos(phi) * cos(theta);
    p[1] =   sin(phi);
    p[2] = - cos(phi) * sin(theta);
    return p;
}

vec3f sphere_tangent(float theta, float phi)
{
    vec3f t;
    t[0] = - sin(theta);
    t[1] =   0;
    t[2] = - cos(theta);
    return t;
}

vec3f sphere_binormal(float theta, float phi)
{
    vec3f b;
    b[0] = - sin(phi) * cos(theta);
    b[1] =   cos(phi);
    b[2] =   sin(phi) * sin(theta);
    return b;
}

vec3f sphere_normal(float theta, float phi)
{
    return sphere_position(theta, phi);
}

void draw_sphere()
{

    int stacks = 100;
    int slices = 200;
    float pi = 3.1415927f;
    for(int i=0; i < stacks-1; i++)
    {
        float t = i/(stacks-1.f);
        float t2 = (i+1)/(stacks-1.f);
        float phi = pi*t - pi/2;
        float phi2 = pi*t2 - pi/2;

        glBegin(GL_QUAD_STRIP);
        for(int j=0; j < slices; j++)
        {
            float s = j/(slices-1.f);
            float theta = 2*pi*s;

            glVertexAttrib2fARB(texcoordAttrib, s, t);
            glVertexAttrib3fvARB(tangentBasisAttrib, &sphere_tangent (theta, phi)[0]); 
            glVertexAttrib3fvARB(tangentBasisAttrib + 1, &sphere_binormal (theta, phi)[0]); 
            glVertexAttrib3fvARB(tangentBasisAttrib + 2, &sphere_normal (theta, phi)[0]);
            glVertexAttrib3fvARB(positionAttrib, &sphere_position (theta, phi)[0]); 

            glVertexAttrib2fARB(texcoordAttrib, s, t2);
            glVertexAttrib3fvARB(tangentBasisAttrib, &sphere_tangent (theta, phi2)[0]); 
            glVertexAttrib3fvARB(tangentBasisAttrib + 1, &sphere_binormal (theta, phi2)[0]); 
            glVertexAttrib3fvARB(tangentBasisAttrib + 2, &sphere_normal (theta, phi2)[0]);
            glVertexAttrib3fvARB(positionAttrib, &sphere_position (theta, phi2)[0]);
        }
        glEnd();
    }
}

void display()
{
    vec3f l(1,1,3);

    l.normalize();
    quaternionf q(vec3f(0,0,1), light_rotation);
    q.mult_vec(l); 
    object.trackball.r.inverse().mult_vec(l);   // rotate the light into object space
    vec3f eye(0,0,1);                           // infinite viewer
    object.trackball.r.inverse().mult_vec(eye); // rotate eye vector into object space
    vec3f h = l + eye;
    h.normalize();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    object.apply_transform();

    // set vertex program inputs
    glUniform3fARB(lightParam, l[0], l[1], l[2]);
    glUniform3fARB(halfangleParam, h[0], h[1], h[2]);

    matrix4f mat;
    glGetFloatv(GL_MODELVIEW_MATRIX, mat.m);
    glUniformMatrix4fvARB(modelViewIParam, 1, GL_FALSE, mat.inverse().m);

    sphere.call_list();

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;

    glutSwapBuffers();
}
 
