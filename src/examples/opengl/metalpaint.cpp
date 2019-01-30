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
#include <shared/noise.h>
#include <shared/quitapp.h>

#include <nv_dds/nv_dds.h>

#include "nvbHelper.h"

using namespace glh;
using namespace nv_dds;

// key mapping
bool b[256];

glut_simple_mouse_interactor object;

bool show_wireframe;
bool animate = true;

GLhandleARB metalPaintProgram = 0;
GLhandleARB vertexLightingProgram = 0;

GLint diffuseColorParam = -1;
GLint specularColorParam = -1;
GLint ambientColorParam = -1;
GLint lightVectorParam = -1;

GLuint envMap = 0;
GLuint noiseMap = 0;

nvbHelper nvbModel;

void cleanExit(int exitval)
{
    if (metalPaintProgram) 
        glDeleteObjectARB(metalPaintProgram);
    if (vertexLightingProgram)
        glDeleteObjectARB(vertexLightingProgram);

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

void loadMetalPaintShader(data_path &media)
{
    metalPaintProgram = glCreateProgramObjectARB();

    string filename = media.get_file("../MEDIA/programs/glsl_metalpaint/metalpaint_vertex.glsl");
    if (filename == "")
    {
        printf("Unable to load metalpaint_vertex.glsl, exiting...\n");
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(metalPaintProgram, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    filename = media.get_file("programs/glsl_metalpaint/metalpaint_fragment.glsl");
    if (filename == "")
    {
        printf("Unable to load metalpaint_fragment.glsl, exiting...\n");
        cleanExit(-1);
    }

    shaderData = read_text_file(filename.c_str());
    addShader(metalPaintProgram, shaderData, GL_FRAGMENT_SHADER_ARB);

    delete [] shaderData;

    glLinkProgramARB(metalPaintProgram);

    GLint linked = false;
    glGetObjectParameterivARB(metalPaintProgram, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked)
    {
        printInfoLog(metalPaintProgram);
        cout << "Shaders failed to link, exiting..." << endl;
        cleanExit(-1);
    }

    // get uniform locations
    diffuseColorParam = glGetUniformLocationARB(metalPaintProgram, "diffuseColor");
    assert(diffuseColorParam >= 0);

    specularColorParam = glGetUniformLocationARB(metalPaintProgram, "specularColor");
    assert(specularColorParam >= 0);

    ambientColorParam = glGetUniformLocationARB(metalPaintProgram, "ambientColor");
    assert(ambientColorParam >= 0);

    lightVectorParam = glGetUniformLocationARB(metalPaintProgram, "lightVector");
    assert(lightVectorParam >= 0);

    glUseProgramObjectARB(metalPaintProgram);

    glUniform1iARB(glGetUniformLocationARB(metalPaintProgram, "decalMap"), 0); 
    glUniform1iARB(glGetUniformLocationARB(metalPaintProgram, "envMap"), 1); 
    glUniform1iARB(glGetUniformLocationARB(metalPaintProgram, "noiseMap"), 2); 

    glUseProgramObjectARB(0);
}

void loadVertexLightingShader(data_path &media)
{
    vertexLightingProgram = glCreateProgramObjectARB();

    string filename = media.get_file("programs/glsl_metalpaint/vertex_lighting.glsl");
    if (filename == "")
    {
        printf("Unable to load vertex_lighting.glsl, exiting...\n");
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(vertexLightingProgram, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    glLinkProgramARB(vertexLightingProgram);

    GLint linked = false;
    glGetObjectParameterivARB(vertexLightingProgram, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked)
    {
        printInfoLog(vertexLightingProgram);
        cout << "Shaders failed to link, exiting..." << endl;
        cleanExit(-1);
    }
}

void init()
{
    if(!glh_init_extensions("GL_VERSION_1_3 GL_ARB_shader_objects GL_ARB_vertex_program GL_ARB_vertex_shader GL_ARB_fragment_shader GL_SGIS_generate_mipmap GL_ARB_texture_cube_map"))
    {
        cout << "Necessary extensions unsupported: " <<  glh_get_unsupported_extensions() << endl;
        quitapp(-1);
    }

    // Set data paths...
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    loadMetalPaintShader(media);
    loadVertexLightingShader(media);

    // find and load environment cubemap
    string filename = media.get_file("textures/cubemaps/nvlobby_cube_mipmap.dds");
    if (filename == "")
    {
        cout << "Unable to load nvlobby_cube_mipmap.dds, exiting..." << endl;
        cleanExit(-1);
    }

    glGenTextures(1, &envMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    CDDSImage image;
    if (!image.load(filename))
    {
        cout << "Failed to load " << filename << ", exiting..." << endl;
        cleanExit(-1);
    }

    image.upload_textureCubemap();

    // create 3D noise texture consisting of un-normalized vectors
    glGenTextures(1, &noiseMap);
    glBindTexture(GL_TEXTURE_3D, noiseMap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    FilteredNoise noise(16, 16, 16);
    noise.createVectorNoiseTexture3D(64, 64, 64, 1.0, 1.0, FilteredNoise::NOISE);

    // load model
    string modelPath = media.get_path("models/RocketCar/RocketCar.nvb");
    if (modelPath == "")
    {
        cout << "Unable to locate RocketCar model, exiting..." << endl;
        cleanExit(-1);
    }

    nvbModel.Load(modelPath, "RocketCar.nvb");    

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void drawCarModel(bool opaque)
{
    const nv_scene &scene = nvbModel.GetScene();

    // simple texture state caching
    bool texturing = false; 

    if (opaque == false)
    {
        // enable transparency blending...
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
        
    for (unsigned int i = 0; i < scene.num_nodes; ++i)
    {
        nv_node *node = scene.nodes[i];

        if (node->get_type() == nv_node::GEOMETRY)
        {
            nv_model *model = reinterpret_cast<nv_model*>(node);

            if (model->num_meshes == 0)
                continue;

            // lets push the object hierarchy...
            glPushMatrix();
            glMultMatrixf(model->xform.mat_array);

            for (unsigned int j = 0; j < model->num_meshes; ++j)
            {
                nv_mesh &mesh = model->meshes[j];

                if (mesh.material_id == NV_BAD_IDX)
                    break;

                if (mesh.skin)
                    break;

                nv_material &mat = scene.materials[mesh.material_id];
 
                if ((opaque == true && mat.transparent == nv_one) ||
                    (opaque == false && mat.transparent < nv_one))
                {
                    // look for a diffuse texture...
                    nv_idx diffuse_id = NV_BAD_IDX;
                    for (unsigned int k = 0; k < mat.num_textures &&  diffuse_id == NV_BAD_IDX; ++k)
                    {
                        if (scene.textures[mat.textures[k]].type == nv_texture::DIFFUSE)
                            diffuse_id = mat.textures[k];
                    }

                    // only apply metal paint effect to certain parts of car
                    if (strcmp(mat.name,"Fender") == 0 ||
                        strcmp(mat.name,"Hood")   == 0 ||
                        strcmp(mat.name,"Door1")  == 0 ||
                        strcmp(mat.name,"Door2")  == 0)
                    {
                        glUseProgramObjectARB(metalPaintProgram);

                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_CUBE_MAP, envMap);
                        glEnable(GL_TEXTURE_CUBE_MAP);

                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_3D, noiseMap);
                        glEnable(GL_TEXTURE_3D);

                        glActiveTexture(GL_TEXTURE0);

                        glUniform3fvARB(diffuseColorParam, 1, mat.diffuse.vec_array);
                        glUniform3fvARB(specularColorParam, 1, mat.specular.vec_array);
                        glUniform3fvARB(ambientColorParam, 1, mat.ambient.vec_array);
                        glUniform3fvARB(lightVectorParam, 1, vec3_one.vec_array);
                    }
                    else
                    {
                        // set up the material...
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat.diffuse.vec_array);
                        glMaterialfv(GL_FRONT, GL_SPECULAR, mat.specular.vec_array);
                        glMaterialfv(GL_FRONT, GL_AMBIENT, mat.ambient.vec_array);

                        GLfloat position[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
                        glLightfv(GL_LIGHT0, GL_POSITION, position);

                        glUseProgramObjectARB(vertexLightingProgram);
                    }

                    glEnableClientState(GL_NORMAL_ARRAY);
                    glEnableClientState(GL_VERTEX_ARRAY);

                    glNormalPointer(GL_FLOAT, 0, mesh.normals);

                    if (mesh.num_texcoord_sets)
                        glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoord_sets[0].texcoords);

                    glVertexPointer(3, GL_FLOAT, 0, mesh.vertices);

                    if (diffuse_id != NV_BAD_IDX)
                    {
                        // we found a diffuse texture, we can enable 
                        // texturing if it hasn't been already set.
                        if (texturing == false)
                        {
                            glEnable(GL_TEXTURE_2D);
                            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                            texturing = true;
                        }
        
                        glBindTexture(GL_TEXTURE_2D, nvbModel.GetTexture(diffuse_id));
                    }
                    else
                    {
                        texturing = false;
                        glDisable(GL_TEXTURE_2D);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    }

                    glDrawElements(GL_TRIANGLES, mesh.num_faces * 3, GL_UNSIGNED_INT, mesh.faces_idx);

                    if (strcmp(mat.name,"Fender") == 0 ||
                        strcmp(mat.name,"Hood")   == 0 ||
                        strcmp(mat.name,"Door1")  == 0 ||
                        strcmp(mat.name,"Door2")  == 0 )
                    {
                        glActiveTexture(GL_TEXTURE1);
                        glDisable(GL_TEXTURE_CUBE_MAP);

                        glActiveTexture(GL_TEXTURE2);
                        glDisable(GL_TEXTURE_3D);

                        glActiveTexture(GL_TEXTURE0);
                    }
                    
                    glUseProgramObjectARB(0);

                    glDisableClientState(GL_NORMAL_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                }
            }

            glPopMatrix();
        }
    }

    if (opaque == false)
        glDisable(GL_BLEND);

    if (texturing)
    {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void display()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.apply_transform();

	glRotatef(75.0f, 1.0f, -8.0f, 1.8f );

    glScalef(0.1, 0.1, 0.1);

    drawCarModel(true);
    drawCarModel(false);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;

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
    glutInitWindowSize(512, 512);
    glutCreateWindow("Metal Paint");

    // Do some OpenGL initialization.
    init();

    glut_helpers_initialize();

    object.configure_buttons(1);
    object.dolly.dolly[0] = -0.1;
    object.dolly.dolly[1] =  0.4;
    object.dolly.dolly[2] = -20;

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
