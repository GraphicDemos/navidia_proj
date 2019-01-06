/*
This class implements a particle system using float textures and fragment programs.
It uses two float textures to represent the current and previous particle positions.
A fragment program calculates the new positions using Verlet integration,
and writes the result to a float buffer

sgreen 6/2002
*/

#include <glh/glh_linear.h>
#include <glh/glh_extensions.h>
#include <GL/glu.h>
#include <string>
#include <shared/pbuffer.h>
#include <nv_png.h>
#include <shared/array_texture.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

#include "ParticleSystem.h"
#include <iostream>

using namespace std;

void printInfoLog(GLhandleARB object)
{
    int maxLength = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

    char *infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);

    printf("%s\n", infoLog);
}

ParticleSystem::
ParticleSystem(int w, int h, int iterations) : m_w(w), m_h(h), m_iterations(iterations)
{
    m_x = new vec3f [m_w * m_h];
    m_debug = new float [m_w * m_h];

    m_pbuffer[0] = CreatePBuffer("float=32 rgba textureRECT", m_pbuffer_tex[0], false);
    m_pbuffer[1] = CreatePBuffer("float=32 rgba textureRECT", m_pbuffer_tex[1], false);
    m_pbuffer[2] = CreatePBuffer("float=32 rgba textureRECT", m_pbuffer_tex[2], false);
    m_normal_pbuffer = CreatePBuffer("rgb textureRECT", m_normal_pbuffer_tex, true);

    m_current = 0;
    m_previous = 1;
    m_dest = 2;

    m_gravity = vec3f(0.0, -2.0, 0.0);
    m_damping = 0.99f;

    InitShaders();

    m_vertexArray = new RenderVertexArray(w * h, 4);
    InitVertexArray();
};

ParticleSystem::
~ParticleSystem()
{
    if (m_physicsProg) glDeleteObjectARB(m_physicsProg);
    if (m_physicsProg2) glDeleteObjectARB(m_physicsProg2);
    if (m_passthru_prog) glDeleteObjectARB(m_passthru_prog);
    if (m_normal_prog) glDeleteObjectARB(m_normal_prog);
    if (m_shader_prog) glDeleteObjectARB(m_shader_prog);

    delete [] m_x;
    delete m_pbuffer[0], m_pbuffer[1], m_pbuffer[2];
    delete m_indices;
    delete m_vertexArray;
}

PBuffer *
ParticleSystem::CreatePBuffer(char *mode, GLuint &tex, bool filter)
{
    PBuffer *pbuffer = new PBuffer(mode);
    if (!pbuffer->Initialize(m_w, m_h, false, true))
        quitapp(-1);

    pbuffer->Activate();

    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, m_w, m_h);

    // create texture
    const GLenum target = GL_TEXTURE_RECTANGLE_NV;
    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    if (filter) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    pbuffer->Deactivate();

    return pbuffer;
}

void
ParticleSystem::addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType, bool doLink)
{
    assert(programObject != 0);
    assert(shaderSource != 0);
    assert(shaderType != 0);

    GLhandleARB object = glCreateShaderObjectARB(shaderType);
    assert(object != 0);

    GLint length = (GLint)strlen(shaderSource);
    glShaderSourceARB(object, 1, &shaderSource, &length);

    // compile shader object
    glCompileShaderARB(object);

    // check if shader compiled
    GLint compiled = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (!compiled)
    {
        printInfoLog(object);
        quitapp(-1);
    }

    // attach shader to program object
    glAttachObjectARB(programObject, object);

    // delete object, no longer needed
    glDeleteObjectARB(object);

    if (doLink)
    {
        // link shader object
        glLinkProgramARB(programObject);

        GLint linked = false;
        glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
        if (!linked)
        {
            printInfoLog(programObject);
            printf("Shaders failed to link, exiting...\n");
            quitapp(-1);
        }
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        printf("OpenGL error: %s\n", gluErrorString(err));
}

GLhandleARB
ParticleSystem::loadProgram(char *vertFilename, char *fragFilename, data_path &path, bool doLink)
{
    GLhandleARB programObject = glCreateProgramObjectARB();

    // load vertex shader
    std::string pathname = path.get_file(vertFilename);
    if (pathname == "") {
        printf("Unable to load '%s', exiting...\n", vertFilename);
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(pathname.c_str());
    if (shaderData == 0)
    {
        printf("Unable to read '%s', exiting...\n", pathname);
        quitapp(-1);
    }

    addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB, false);
    delete [] shaderData;

    // load fragment shader
    pathname = path.get_file(fragFilename);
    if (pathname == "") {
        printf("Unable to load '%s', exiting...\n", fragFilename);
        quitapp(-1);
    }

    shaderData = read_text_file(pathname.c_str());
    if (shaderData == 0)
    {
        printf("Unable to read '%s', exiting...\n", pathname);
        quitapp(-1);
    }

    addShader(programObject, shaderData, GL_FRAGMENT_SHADER_ARB, doLink);
    delete [] shaderData;

    return programObject;
}

GLhandleARB
ParticleSystem::loadProgram(char *fragFilename, data_path &path, bool doLink)
{
    GLhandleARB programObject = glCreateProgramObjectARB();

    // load fragment shader
    std::string pathname = path.get_file(fragFilename);
    if (pathname == "") {
        printf("Unable to load '%s', exiting...\n", fragFilename);
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(pathname.c_str());
    if (shaderData == 0)
    {
        printf("Unable to read '%s', exiting...\n", pathname);
        quitapp(-1);
    }

    addShader(programObject, shaderData, GL_FRAGMENT_SHADER_ARB, doLink);
    delete [] shaderData;

    return programObject;
}

void 
ParticleSystem::InitShaders()
{
    data_path path;
    path.path.push_back(".");
    path.path.push_back("../../../MEDIA/programs");
    path.path.push_back("../../../../MEDIA/programs");

    // load programs
    m_passthru_prog  = loadProgram("glsl_physics/passthru.glsl", path, true);
    m_normal_prog    = loadProgram("glsl_physics/normal.glsl", path, true);
    m_shader_prog    = loadProgram("glsl_physics/shader_vp.glsl", "glsl_physics/shader.glsl", path, true);

    m_physicsProg    = loadProgram("glsl_physics/physics.glsl", path, false);
    m_physicsProg2   = loadProgram("glsl_physics/physics2.glsl", path, false);

    // load Verlet integration code
    std::string pathname = path.get_file("glsl_physics/verlet.glsl");
    if (pathname == "") {
        printf("Unable to load 'verlet.glsl', exiting...\n");
        quitapp(-1);
    }

    GLcharARB *shaderData = read_text_file(pathname.c_str());
    if (shaderData == 0)
    {
        printf("Unable to read '%s', exiting...\n", pathname);
        quitapp(-1);
    }

    // add Verlet integration code to physics shaders
    addShader(m_physicsProg, shaderData, GL_FRAGMENT_SHADER_ARB, true);
    addShader(m_physicsProg2, shaderData, GL_FRAGMENT_SHADER_ARB, true);

    delete [] shaderData;

    // get parameters
    m_timestep_param       = glGetUniformLocationARB(m_physicsProg, "timestep");
    m_damping_param        = glGetUniformLocationARB(m_physicsProg, "damping");
    m_gravity_param        = glGetUniformLocationARB(m_physicsProg, "gravity");

    GLint meshSize_param   = glGetUniformLocationARB(m_physicsProg2, "meshSize");

    m_constraintDist_param = glGetUniformLocationARB(m_physicsProg2, "constraintDist");
    m_spherePos_param      = glGetUniformLocationARB(m_physicsProg2, "spherePos");

    m_scale_param          = glGetUniformLocationARB(m_passthru_prog, "scale");
    m_bias_param           = glGetUniformLocationARB(m_passthru_prog, "bias");

    // set initial uniform values for physics program
    glUseProgramObjectARB(m_physicsProg);
    glUniform1fARB(m_timestep_param, 0.01f);
    glUniform1fARB(m_damping_param, m_damping);
    glUniform3fvARB(m_gravity_param, 1, &m_gravity[0]);

    // set initial uniform values for physics2 program
    glUseProgramObjectARB(m_physicsProg2);
    glUniform2fARB(meshSize_param, m_w - 1.0f, m_h - 1.0f);

    // associate samplers with specific texture units for each program
    glUseProgramObjectARB(m_physicsProg);
    glUniform1iARB(glGetUniformLocationARB(m_physicsProg, "x_tex"), 0); 
    glUniform1iARB(glGetUniformLocationARB(m_physicsProg, "ox_tex"), 1); 

    glUseProgramObjectARB(m_physicsProg2);
    glUniform1iARB(glGetUniformLocationARB(m_physicsProg2, "x_tex"), 0); 

    glUseProgramObjectARB(m_passthru_prog);
    glUniform1iARB(glGetUniformLocationARB(m_passthru_prog, "tex"), 0); 
    
    glUseProgramObjectARB(m_normal_prog);
    glUniform1iARB(glGetUniformLocationARB(m_normal_prog, "x_tex"), 0); 

    glUseProgramObjectARB(m_shader_prog);
    glUniform1iARB(glGetUniformLocationARB(m_shader_prog, "normal_tex"), 0); 
    glUniform1iARB(glGetUniformLocationARB(m_shader_prog, "cloth_tex"), 1);

    glUseProgramObjectARB(0);

    // load cloth texture
    array2<vec3ub> img;
    read_png_rgb("nvidia_cloth.png", img);

    glGenTextures(1, &m_cloth_tex);
    glBindTexture(GL_TEXTURE_2D, m_cloth_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    make_rgb_texture(img, true);
}

void 
ParticleSystem::DrawImage()
{
    glUseProgramObjectARB(m_passthru_prog);

    glUniform4fARB(m_scale_param, 1.0, 1.0, 1.0, 1.0);
    glUniform4fARB(m_bias_param, 0.0, 0.0, 0.0, 0.0);

    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB_NV, m_w, m_h, 0, GL_RGB, GL_FLOAT, (GLfloat *) &m_x[0]);
    DrawQuad();

    glUseProgramObjectARB(0);
}

// initialize particles in grid pattern
void 
ParticleSystem::InitGrid(vec3f start, vec3f du, vec3f dv) {
    for(int y=0; y<m_h; y++) {
        for(int x=0; x<m_w; x++) {
            vec3f p = start + du * (float)x + dv * (float)y;
            m_x[m_w*y + x] = p;
        }
    }
    m_pbuffer[m_current]->Activate();
    glClear(GL_COLOR_BUFFER_BIT);
    DrawImage();
    m_pbuffer[m_current]->Deactivate();

    m_pbuffer[m_previous]->Activate();
    glClear(GL_COLOR_BUFFER_BIT);
    DrawImage();
    m_pbuffer[m_previous]->Deactivate();

    glUseProgramObjectARB(m_physicsProg2);
    glUniform1fARB(m_constraintDist_param, du[0]);
    glUseProgramObjectARB(0);
}

void 
ParticleSystem::SetSphere(vec3f pos)
{
    glUseProgramObjectARB(m_physicsProg2);
    glUniform3fARB(m_spherePos_param, pos[0], pos[1], pos[2]);
    glUseProgramObjectARB(0);
}

// step the simulation
void 
ParticleSystem::TimeStep(float dt)
{
    // integration pass
    // read from current and previous position buffers, write to destination
    m_pbuffer[m_dest]->Activate();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pbuffer_tex[m_current]);
    m_pbuffer[m_current]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pbuffer_tex[m_previous]);
    m_pbuffer[m_previous]->Bind(WGL_FRONT_LEFT_ARB);

    glUseProgramObjectARB(m_physicsProg);
    glUniform1fARB(m_timestep_param, dt);
    glUniform1fARB(m_damping_param, m_damping);
    glUniform3fvARB(m_gravity_param, 1, &m_gravity[0]);

    DrawQuad();

    glUseProgramObjectARB(0);

    m_pbuffer[m_current]->Release(WGL_FRONT_LEFT_ARB);
    m_pbuffer[m_previous]->Release(WGL_FRONT_LEFT_ARB);

    m_pbuffer[m_dest]->Deactivate();

    //    printf("c: %d p: %d d: %d\n", m_current, m_previous, m_dest);

    int temp = m_previous;
    m_previous = m_current;
    m_current = m_dest;
    m_dest = temp;

#if 1
    // constraint pass(es)
    for(int i=0; i<m_iterations; i++) {
        m_pbuffer[m_dest]->Activate();

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pbuffer_tex[m_current]);
        m_pbuffer[m_current]->Bind(WGL_FRONT_LEFT_ARB);

        glUseProgramObjectARB(m_physicsProg2);
        DrawQuad();
        glUseProgramObjectARB(0);

        m_pbuffer[m_current]->Release(WGL_FRONT_LEFT_ARB);
        m_pbuffer[m_dest]->Deactivate();

        int temp = m_current;
        m_current = m_dest;
        m_dest = temp;
    }
#endif
}

// calculate normals 
void 
ParticleSystem::CalculateNormals()
{
    m_normal_pbuffer->Activate();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pbuffer_tex[m_current]);
    m_pbuffer[m_current]->Bind(WGL_FRONT_LEFT_ARB);

    glUseProgramObjectARB(m_normal_prog);
    DrawQuad();
    glUseProgramObjectARB(0);

    m_pbuffer[m_current]->Release(WGL_FRONT_LEFT_ARB);
    m_normal_pbuffer->Deactivate();
}

void 
ParticleSystem::DrawQuad()
{
    glBegin(GL_QUADS);
    glTexCoord2f(0,          0);          glVertex2f(0,           0);
    glTexCoord2f((float)m_w, 0);          glVertex2f((float)m_w,  0);
    glTexCoord2f((float)m_w, (float)m_h); glVertex2f((float)m_w,  (float)m_h);
    glTexCoord2f(0,          (float)m_h); glVertex2f(0,           (float)m_h);
    glEnd();
}

// read frame buffer to main memory
void 
ParticleSystem::ReadBack()
{
    m_pbuffer[m_current]->Activate();
    glReadPixels(0, 0, m_w, m_h, GL_RGB, GL_FLOAT, (float *) m_x);
    //    glReadPixels(0, 0, m_w, m_h, GL_ALPHA, GL_FLOAT, (float *) m_debug);
    m_pbuffer[m_current]->Deactivate();
}

// display particle system as points and lines
void 
ParticleSystem::Display()
{
    // draw particles
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(3.0);
    glBegin(GL_POINTS);
    for(int i=0; i<m_w*m_h; i++) {
        glVertex3fv( (float *) &m_x[i] );
    }
    glEnd();

    // draw constraints
    glColor3f(1.0, 1.0, 0.0);
    for(int y=0; y<m_h; y++) {
        glBegin(GL_LINE_STRIP);
        for(int x=0; x<m_w; x++) {
            glVertex3fv( (float *) &m_x[y*m_w + x] );
        }
        glEnd();
    }
    for(int x=0; x<m_w; x++) {
        glBegin(GL_LINE_STRIP);
        for(int y=0; y<m_h; y++) {
            glVertex3fv( (float *) &m_x[y*m_w + x] );
        }
        glEnd();
    }
}

// display shaded mesh
void 
ParticleSystem::DisplayShaded()
{
    ReadBack();

    CalculateNormals();

    glUseProgramObjectARB(m_shader_prog);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_normal_pbuffer_tex);
    m_normal_pbuffer->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_cloth_tex);

    glColor3f(1.0, 1.0, 0.0);
    for(int y=0; y<m_h-1; y++) {
        glBegin(GL_TRIANGLE_STRIP);
        for(int x=0; x<m_w; x++) {
            glTexCoord2f((float)x - 0.5f, (float)y - 0.5f);
            glVertex3fv( (float *) &m_x[y*m_w + x] );
            glTexCoord2f((float)x - 0.5f, (float)y + 1.0f - 0.5f);
            glVertex3fv( (float *) &m_x[(y+1)*m_w + x] );
        }
        glEnd();
    }

    glUseProgramObjectARB(0);

    m_normal_pbuffer->Release(WGL_FRONT_LEFT_ARB);
}

void 
ParticleSystem::InitVertexArray()
{
    // create index list
    m_nindices = ((m_h-1) * (m_w+1) * 2);
    m_indices = new short [m_nindices];

    short *ptr = m_indices;
	int x = 0;
    for(int y=0; y<m_h-1; y++) {
        for( x=0; x<m_w; x++) {
            *ptr++ = y*m_w + x;
            *ptr++ = (y+1)*m_w + x;
        }
        *ptr++ = (y+1)*m_w + x-1;
        *ptr++ = (y+1)*m_w;
    }

    m_texCoords = new float [m_w * m_h * 3];
    float *texCoords = m_texCoords;
	int y = 0;
    for(y=0; y<m_h; y++) {
        for(int x=0; x<m_w; x++) {
            *texCoords++ = (float)x - 0.5f;
            *texCoords++ = (float)y - 0.5f;
            *texCoords++ = 0.0f;
        }
    }

    // use vbo for texcoords
    glGenBuffersARB(1, &m_texCoordBuffer);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_texCoordBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_w*m_h*3*sizeof(float), m_texCoords, GL_STATIC_DRAW_ARB);
    glTexCoordPointer(3, GL_FLOAT, 0, 0);
}

// display shaded mesh using render-to-vertex array
void 
ParticleSystem::DisplayShaded2()
{
    // copy geometry from framebuffer
    m_pbuffer[m_current]->Activate();
    m_vertexArray->Read(GL_FRONT, m_w, m_h);
    m_pbuffer[m_current]->Deactivate();

    CalculateNormals();

    glUseProgramObjectARB(m_shader_prog);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_normal_pbuffer_tex);
    m_normal_pbuffer->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_cloth_tex);

    // draw the geometry
    m_vertexArray->SetPointer(0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArrayARB(0);
    glDrawElements(GL_TRIANGLE_STRIP, m_nindices, GL_UNSIGNED_SHORT, m_indices);
    glDisableVertexAttribArrayARB(0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glUseProgramObjectARB(0);

    m_normal_pbuffer->Release(WGL_FRONT_LEFT_ARB);
}

// display float texture using texturing
void 
ParticleSystem::DisplayTexture(PBuffer *pbuffer, GLuint tex)
{
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
    pbuffer->Bind(WGL_FRONT_LEFT_ARB);

    DrawQuad();

    pbuffer->Release(WGL_FRONT_LEFT_ARB);
}

// display position and normal textures
void 
ParticleSystem::DisplayTextures()
{
    glPushMatrix();
    glScalef(0.1f, 0.1f, 1.0f);
    glTranslatef((float)-m_w, (float)-m_h, 0.0);

    glUseProgramObjectARB(m_passthru_prog);

    glUniform4fARB(m_scale_param, 0.25, 0.25, 0.25, 1.0);
    glUniform4fARB(m_bias_param, 0.5, 0.0, 0.5, 0.0);
    DisplayTexture(m_pbuffer[m_current], m_pbuffer_tex[m_current]);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1f, 0.1f, 1.0f);
    glTranslatef(0.0f, (float)-m_h, 0.0f);
    glUniform4fARB(m_scale_param, 1.0, 1.0, 1.0, 0.0);
    glUniform4fARB(m_bias_param, 0.0, 0.0, 0.0, 0.0);
    DisplayTexture(m_normal_pbuffer, m_normal_pbuffer_tex);
    glPopMatrix();

    glUseProgramObjectARB(0);
}

