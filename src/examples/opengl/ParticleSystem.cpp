/*
  This class implements a particle system using float textures and fragment programs.
*/

#include <GL/glut.h>
#include <Cg/cgGL.h>
#include <glh/glh_linear.h>
#include <glh/glh_extensions.h>
#include <string>
#include "shared/pbuffer.h"
#include <nv_png.h>
#include "shared/array_texture.h"
#include "shared/renderVertexArray.h"

#include "ParticleSystem.h"

#define USE_FP16 0  // use fp16 for positions
#define NEW_CG   0  // Cg with FP40 profile?

CGcontext g_context;

void 
cgErrorCallback(void)
{
    CGerror lastError = cgGetError();
    if(lastError) {
        const char *listing = cgGetLastListing(g_context);
        printf("\n---------------------------------------------------\n");
        printf("%s\n\n", cgGetErrorString(lastError));
        printf("%s\n", listing);
        printf("-----------------------------------------------------\n");
        printf("Cg error, exiting...\n");
        cgDestroyContext(g_context);
        exit(-1);
    }
}

ParticleSystem::
ParticleSystem(int w, int h, int iterations, bool useMRT)
    : m_w(w), m_h(h), m_iterations(iterations), m_expand(2), m_useMRT(useMRT)
{
    m_pos = new vec3f [GetSize()];
    m_vel = new vec3f [GetSize()];

#if USE_FP16
    char *format = "float=16 rgba textureRECT";
#else
    char *format = "float=32 rgba textureRECT";
#endif
    m_pos_buffer[0] = CreatePBuffer(m_w, m_h, format);
    m_pos_buffer[1] = CreatePBuffer(m_w, m_h, format);

    if (!m_useMRT) {
        char *vel_format = "float=16 rgba textureRECT";
        m_vel_buffer[0] = CreatePBuffer(m_w, m_h, vel_format);
        m_vel_buffer[1] = CreatePBuffer(m_w, m_h, vel_format);
    }
#if 1
     m_geom_buffer = CreatePBuffer(m_w*m_expand, m_h, format);
#endif
    m_current = 0;
    m_previous = 1;

    m_gravity = vec3f(0.0, -1.0, 0.0);
    m_damping = 0.99f;
//    m_damping = 1.0f;

    m_point_alpha = 0.2f;
//    m_point_alpha = 1.0f;
    m_pointsize = 8.0f;
    m_blur_passes = 5;

    InitCg();

#if USE_FP16
    m_vertexArray[0] = new RenderVertexArray(GetSize()*m_expand, 4, GL_HALF_FLOAT_NV);
    m_vertexArray[1] = new RenderVertexArray(GetSize()*m_expand, 4, GL_HALF_FLOAT_NV);
#else
    m_vertexArray[0] = new RenderVertexArray(GetSize()*m_expand, 4);
    m_vertexArray[1] = new RenderVertexArray(GetSize()*m_expand, 4);
#endif
    InitVertexArray();
    InitBuffers();

    // per-vertex particle data
    m_particleData = new ParticleData[ GetSize() ];
    
    glGenBuffersARB(1, &m_particleDataBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_particleDataBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, GetSize()*4*sizeof(float), m_particleData, GL_STREAM_DRAW);
    glVertexAttribPointerARB(8, 4, GL_FLOAT, GL_FALSE, 0, 0);  // 8 = texcoord0

    Reset();

#if 0
    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0);
    glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, 32.0);
    glPointParameterfARB(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 1.0);
    GLfloat atten_params[] = { 0.0, 0.1, 0.0 };
    glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, atten_params);
#endif

    // load point sprite texture
    array2<vec4ub> img;
    read_png_rgba("pointsprite.png", img);

    glGenTextures(1, &m_sprite_tex);
    glBindTexture(GL_TEXTURE_2D, m_sprite_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    make_rgba_texture(img, true);

    // load terrain
    read_png_grey("grcanyon.png", m_terrain_img);
    glGenTextures(1, &m_terrain_tex);
    glBindTexture(GL_TEXTURE_2D, m_terrain_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (glutExtensionSupported("GL_ATI_texture_float")) {
        // use fp16 texture with filtering on NV4x
        make_scalar_texture(m_terrain_img, GL_LUMINANCE_FLOAT16_ATI, GL_LUMINANCE, true);
    } else {
        make_scalar_texture(m_terrain_img, GL_LUMINANCE, GL_LUMINANCE, true);
    }

    m_terrain_dlist = glGenLists(1);
    glNewList(m_terrain_dlist, GL_COMPILE);
    DrawTerrain();
    glEndList();

    m_terrain_scale = vec3f(8.0, 2.0, 8.0);
    m_terrain_offset = vec3f(-4.0, 0.0, -4.0);
};

ParticleSystem::
~ParticleSystem()
{
    cgDestroyProgram(m_pos_fprog);
    cgDestroyProgram(m_vel_fprog);
    cgDestroyProgram(m_passthru_fprog);
    cgDestroyProgram(m_shader_fprog);
    cgDestroyProgram(m_shader_vprog);

    cgDestroyContext(g_context);

    delete [] m_pos;
    delete [] m_vel;
    delete [] m_texCoords;
    delete m_pos_buffer[0], m_pos_buffer[1];
    delete m_vel_buffer[0], m_vel_buffer[1];
    delete m_vertexArray[0], m_vertexArray[1];
}

RenderTexture *
ParticleSystem::CreatePBuffer(int w, int h, char *mode)
{
    RenderTexture *pbuffer = new RenderTexture(mode, w, h, GL_TEXTURE_RECTANGLE_NV);
    pbuffer->Activate();

    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, w, h);

    pbuffer->Deactivate();
    return pbuffer;
}

CGprogram
ParticleSystem::loadProgram(CGcontext context, CGprofile profile, CGenum program_type, char *filename, data_path &path)
{
    std::string pathname = path.get_file(filename);
    if (pathname == "") {
        printf("Unable to load '%s', exiting...\n", filename);
        exit(-1);
    }

    CGprogram program = cgCreateProgramFromFile(context, program_type, pathname.data(),
        profile, NULL, NULL);
    cgGLLoadProgram(program);
    return program;
}

void 
ParticleSystem::InitCg()
{
    cgSetErrorCallback(cgErrorCallback);
    g_context = cgCreateContext();

    data_path path;
    path.path.push_back(".");
    path.path.push_back("../../../MEDIA/programs");
    path.path.push_back("../../../../MEDIA/programs");

    // load programs
    m_pos_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/update_pos.cg", path);
    m_pos_timestep_param  = cgGetNamedParameter(m_pos_fprog, "timestep");
    m_pos_spherePos_param = cgGetNamedParameter(m_pos_fprog, "spherePos");

    m_vel_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/update_vel.cg", path);
    m_vel_timestep_param  = cgGetNamedParameter(m_vel_fprog, "timestep");
    m_vel_damping_param   = cgGetNamedParameter(m_vel_fprog, "damping");
    m_vel_gravity_param   = cgGetNamedParameter(m_vel_fprog, "gravity");
    m_vel_spherePos_param = cgGetNamedParameter(m_vel_fprog, "spherePos");
    m_vel_sphereVel_param = cgGetNamedParameter(m_vel_fprog, "sphereVel");

    m_passthru_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/passthru.cg", path);
    m_passthru_scale_param = cgGetNamedParameter(m_passthru_fprog, "scale");
    m_passthru_bias_param  = cgGetNamedParameter(m_passthru_fprog, "bias");

    m_copy_texcoord_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/copy_texcoord.cg", path);

    m_shader_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/shader.cg", path);

    m_shader_vprog = loadProgram(g_context, CG_PROFILE_VP20, CG_SOURCE, "gpu_particles/shader_vp.cg", path);
    m_shader_modelViewProj_param  = cgGetNamedParameter(m_shader_vprog, "modelViewProj");
    m_shader_modelView_param      = cgGetNamedParameter(m_shader_vprog, "modelView");
    m_shader_time_param           = cgGetNamedParameter(m_shader_vprog, "time");

    m_shader_mb_vprog = loadProgram(g_context, CG_PROFILE_VP20, CG_SOURCE, "gpu_particles/shader_mb_vp.cg", path);
    m_shader_mb_modelViewProj_param  = cgGetNamedParameter(m_shader_mb_vprog, "modelViewProj");
    m_shader_mb_modelView_param      = cgGetNamedParameter(m_shader_mb_vprog, "modelView");
    m_shader_mb_time_param           = cgGetNamedParameter(m_shader_mb_vprog, "time");
    m_shader_mb_interp_param         = cgGetNamedParameter(m_shader_mb_vprog, "interp");

    m_expand_fprog = loadProgram(g_context, CG_PROFILE_FP30, CG_SOURCE, "gpu_particles/expand.cg", path);

#if NEW_CG
    m_posvel_fprog = loadProgram(g_context, CG_PROFILE_FP40, CG_SOURCE, "gpu_particles/update_pos_vel.cg", path);
    m_posvel_timestep_param  = cgGetNamedParameter(m_posvel_fprog, "timestep");
    m_posvel_damping_param   = cgGetNamedParameter(m_posvel_fprog, "damping");
    m_posvel_gravity_param   = cgGetNamedParameter(m_posvel_fprog, "gravity");
    m_posvel_spherePos_param = cgGetNamedParameter(m_posvel_fprog, "spherePos");
    m_posvel_sphereVel_param = cgGetNamedParameter(m_posvel_fprog, "sphereVel");
#endif
}

void 
ParticleSystem::InitBuffer(RenderTexture *pbuffer, GLenum buffer, vec3f *data)
{
    pbuffer->Activate();
    glDrawBuffer(buffer);

#if 1
    cgGLBindProgram(m_passthru_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);

    cgGLSetParameter4f(m_passthru_scale_param, 1.0, 1.0, 1.0, 1.0);
    cgGLSetParameter4f(m_passthru_bias_param, 0.0, 0.0, 0.0, 0.0);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB_NV, m_w, m_h, 0, GL_RGB, GL_FLOAT, (GLfloat *) &data[0]);
    DrawQuad(m_w, m_h, m_w, m_h);
    cgGLDisableProfile(CG_PROFILE_FP30);
#else
    glDrawPixels(m_w, m_h, GL_RGB, GL_FLOAT, (GLfloat *) &data[0]);
#endif

    glDrawBuffer(GL_FRONT);
    pbuffer->Deactivate();
}

// initialize buffers
void 
ParticleSystem::InitBuffers() {

    if (m_useMRT) {
        InitBuffer(m_pos_buffer[m_previous], mrt_buffers[0], m_pos);
        InitBuffer(m_pos_buffer[m_previous], mrt_buffers[1], m_vel);
        InitBuffer(m_pos_buffer[m_current], mrt_buffers[0], m_pos);
        InitBuffer(m_pos_buffer[m_current], mrt_buffers[1], m_vel);
    } else {
        InitBuffer(m_pos_buffer[m_previous], GL_FRONT, m_pos);
        InitBuffer(m_vel_buffer[m_previous], GL_FRONT, m_vel);
        InitBuffer(m_pos_buffer[m_current], GL_FRONT, m_pos);
        InitBuffer(m_vel_buffer[m_current], GL_FRONT, m_vel);
    }

    m_pos_buffer[0]->Activate();
    m_vertexArray[m_current]->Read(GL_FRONT, m_w, m_h);
    m_pos_buffer[0]->Deactivate();

    m_current = 0;
    m_previous = 1;
}

// initialize particle positions
void 
ParticleSystem::InitParticlesGrid(vec3f start, vec3f du, vec3f dv) {
    for(int y=0; y<m_h; y++) {
        for(int x=0; x<m_w; x++) {
            vec3f p = start + du * (float)x + dv * (float)y;
            m_pos[m_w*y + x] = p;
            m_vel[m_w*y + x] = vec3f(0.0, 0.0, 0.0);
        }
    }

    InitBuffers();
}

float frand()
{
  return rand() / (float) RAND_MAX;
}

float sfrand()
{
  return frand()*2.0f-1.0f;
}

float rand_range(float min, float max)
{
  return min + frand()*(max-min); 
}

vec3f randvec()
{
  return vec3f(sfrand(), sfrand(), sfrand());
}

void 
ParticleSystem::InitParticlesRand(vec3f scale, vec3f offset)
{
    for(int i=0; i < m_w*m_h; i++) {
        vec3f p = vec3f(frand(), frand(), frand());
        m_pos[i] = p*scale + offset;
        m_vel[i] = vec3f(0.0, 0.0, 0.0);
    }

    InitBuffers();
}

void 
ParticleSystem::InitParticlesStream(vec3f pos, vec3f dir, float pos_rand, float vel_rand)
{
    dir.normalize();
    for(int i=0; i < m_w*m_h; i++) {
        vec3f d = dir + randvec()*vel_rand;
        d.normalize();
        m_pos[i] = pos + randvec()*pos_rand;
        m_vel[i] = d;
    }

    InitBuffers();
}

void
ParticleSystem::Reset()
{
    // make all indices available
    m_freeIndices.clear();
    m_freeIndices.reserve(GetSize());
    for(int i=0; i<GetSize(); i++) {
        m_freeIndices.push_back(GetSize() - 1 - i);
    }

    m_activeParticles.clear();

    // reset per-vertex particle data
    for(int i=0; i<GetSize(); i++) {
        m_particleData[i].birth_time = 0.0f;
        m_particleData[i].death_time = 0.0f;
    }
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_particleDataBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, GetSize()*4*sizeof(float), m_particleData, GL_STREAM_DRAW);

    m_time = 0.0f;
}

void
ParticleSystem::InitNewParticles()
{
    if (m_newParticles.empty())
        return ;

    std::vector<NewParticle>::iterator i; 

    glPointSize(1.0);

    // render points for positions of new particles
    m_pos_buffer[m_previous]->Activate();
    if (m_useMRT) {
        glDrawBuffer(mrt_buffers[0]);
    } else {
        glDrawBuffer(GL_FRONT);
    }
    cgGLBindProgram(m_copy_texcoord_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);
    glBegin(GL_POINTS);
    for (i = m_newParticles.begin(); i != m_newParticles.end(); i++) {
        vec2f pos = IndexToPos(i->index);
        glTexCoord4fv(&i->pos[0]);
        glVertex2fv(&pos[0]);
    }
    glEnd();
    cgGLDisableProfile(CG_PROFILE_FP30);
    m_pos_buffer[m_previous]->Deactivate();

    // render points for velocities of new particles
    if (m_useMRT) {
        m_pos_buffer[m_previous]->Activate();
        glDrawBuffer(mrt_buffers[1]);
    } else {
        m_vel_buffer[m_previous]->Activate();
    }
    cgGLBindProgram(m_copy_texcoord_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);
    glBegin(GL_POINTS);
    for (i = m_newParticles.begin(); i != m_newParticles.end(); i++) {
        vec2f pos = IndexToPos(i->index);
        glTexCoord4fv(&i->vel[0]);
        glVertex2fv(&pos[0]);
    }
    glEnd();
    cgGLDisableProfile(CG_PROFILE_FP30);
    if (m_useMRT) {
        m_pos_buffer[m_previous]->Deactivate();
    } else {
        m_vel_buffer[m_previous]->Deactivate();
    }

    // update texture coordinate array containing other particle data
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_particleDataBuffer);
#if 1
    ParticleData *particleData_map = (ParticleData *) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    if (particleData_map) {
	    for (i = m_newParticles.begin(); i != m_newParticles.end(); i++) {
		    particleData_map[i->index] = i->data;
	    }
    }
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
#else
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, GetSize()*4*sizeof(float), m_particleData, GL_STREAM_DRAW);
#endif

    m_newParticles.clear();
}

void
ParticleSystem::FreeDeadParticles()
{
#if 0
    m_pos_buffer[m_previous]->Activate();
    cgGLBindProgram(m_copy_texcoord_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);
    glBegin(GL_POINTS);
#endif

    std::list<ActiveParticle>::iterator i; 
    for (i = m_activeParticles.begin(); i != m_activeParticles.end();) {
        if (m_time > i->death_time) {
            // particle is dead, add it to list of free indices
            m_freeIndices.push_back(i->index);
//            printf("freed: %d\n", i->index);
            i = m_activeParticles.erase(i);

#if 0
            vec2f pos = IndexToPos(i->index);
            glTexCoord4f(0.0, 0.0, 0.0, 0.0);
            glVertex2fv(&pos[0]);
#endif
        } else {
            i++;
        }
    }

#if 0
    glEnd();
    cgGLDisableProfile(CG_PROFILE_FP30);
    m_pos_buffer[m_previous]->Deactivate();
#endif
}

void
ParticleSystem::CreateParticle(vec3f pos, vec3f vel, float lifetime)
{
    if (m_freeIndices.empty())
        return ;

    int i = m_freeIndices.back();
    m_freeIndices.pop_back();
//    printf("created: %d\n", i);

    NewParticle p;
    p.index = i;
    p.pos = pos;
    p.vel = vel;
    p.data.birth_time = m_time;
    p.data.death_time = m_time + lifetime;
    m_newParticles.push_back(p);

    assert(i < GetSize());
    m_particleData[i] = p.data;

    ActiveParticle a;
    a.index = i;
    a.death_time = m_time + lifetime;
    m_activeParticles.push_back(a);
}

void
ParticleSystem::HoseEmitter(int no, vec3f pos, vec3f vel, float theta, float phi, float dir_rand, float vel_min, float vel_max, float lifetime)
{
    m_newParticles.clear();
    for(int i=0; i<no; i++) {
        vec3f v;
        // choose direction based on spherical coordinates
        v[0] = cosf(theta)*sinf(phi);
        v[1] = cosf(phi);
        v[2] = sinf(theta)*sinf(phi);
        // calculate orthogonal vectors
        vec3f up(0.0, 1.0, 0.0);
        vec3f t = v.cross(up); t.normalize();
        vec3f b = v.cross(t); b.normalize();
        // choose random point inside disc
        vec3f disc;
        do {
            disc = sfrand()*t + sfrand()*b;
        } while(disc.length() > 1.0);

        v += disc*dir_rand;
        v.normalize();
        v *= rand_range(vel_min, vel_max);

        CreateParticle(pos, vel + v, lifetime);
    }
}

void 
ParticleSystem::SetSphere(vec3f pos, vec3f vel)
{
    cgGLSetParameter4f(m_pos_spherePos_param, pos[0], pos[1], pos[2], 0.0);
    cgGLSetParameter4f(m_vel_spherePos_param, pos[0], pos[1], pos[2], 0.0);
    cgGLSetParameter4f(m_vel_sphereVel_param, vel[0], vel[1], vel[2], 0.0);
#if NEW_CG
    cgGLSetParameter4f(m_posvel_spherePos_param, pos[0], pos[1], pos[2], 0.0);
    cgGLSetParameter4f(m_posvel_sphereVel_param, vel[0], vel[1], vel[2], 0.0);
#endif
}

// step the simulation
void 
ParticleSystem::TimeStep(float dt)
{
    FreeDeadParticles();
    InitNewParticles();

    if (m_useMRT) {
        TimeStep_MRT(dt);
        return;
    }

    // update velocity
    m_vel_buffer[m_current]->Activate();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    m_pos_buffer[m_previous]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    m_vel_buffer[m_previous]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, m_terrain_tex);

    cgGLBindProgram(m_vel_fprog);
    cgGLSetParameter1f(m_vel_timestep_param, dt);
    cgGLSetParameter1f(m_vel_damping_param, m_damping);
    cgGLSetParameter3fv(m_vel_gravity_param, (float *) &m_gravity);

    cgGLEnableProfile(CG_PROFILE_FP30);
    DrawQuad(m_w, m_h, m_w, m_h);
    cgGLDisableProfile(CG_PROFILE_FP30);

    m_pos_buffer[m_previous]->Release(WGL_FRONT_LEFT_ARB);
    m_vel_buffer[m_previous]->Release(WGL_FRONT_LEFT_ARB);
    m_vel_buffer[m_current]->Deactivate();

    // update position
    m_pos_buffer[m_current]->Activate();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    m_pos_buffer[m_previous]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    m_vel_buffer[m_current]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, m_terrain_tex);

    cgGLBindProgram(m_pos_fprog);
    cgGLSetParameter1f(m_pos_timestep_param, dt);
    cgGLEnableProfile(CG_PROFILE_FP30);
    DrawQuad(m_w, m_h, m_w, m_h);
    cgGLDisableProfile(CG_PROFILE_FP30);

    m_pos_buffer[m_previous]->Release(WGL_FRONT_LEFT_ARB);
    m_vel_buffer[m_current]->Release(WGL_FRONT_LEFT_ARB);
    m_pos_buffer[m_current]->Deactivate();

    int temp = m_previous;
    m_previous = m_current;
    m_current = temp;

    m_time += dt;
//    printf("%f\n", m_time);
}

// step the simulation using multiple draw buffers in a single pass
void 
ParticleSystem::TimeStep_MRT(float dt)
{
    // write to position and velocity buffers
    m_pos_buffer[m_current]->Activate();
    glDrawBuffersATI(2, mrt_buffers);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    m_pos_buffer[m_previous]->Bind(mrt_buffers_wgl[0]);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    m_pos_buffer[m_previous]->Bind(mrt_buffers_wgl[1]);

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, m_terrain_tex);

    cgGLBindProgram(m_posvel_fprog);
    cgGLSetParameter1f(m_posvel_timestep_param, dt);
    cgGLSetParameter1f(m_posvel_damping_param, m_damping);
    cgGLSetParameter3fv(m_posvel_gravity_param, (float *) &m_gravity);

#if NEW_CG
    cgGLEnableProfile(CG_PROFILE_FP40);
#endif
    DrawQuad(m_w, m_h, m_w, m_h);
#if NEW_CG
    cgGLDisableProfile(CG_PROFILE_FP40);
#endif

    m_pos_buffer[m_previous]->Release(mrt_buffers_wgl[0]);
    m_pos_buffer[m_previous]->Release(mrt_buffers_wgl[1]);
    m_pos_buffer[m_current]->Deactivate();

    int temp = m_previous;
    m_previous = m_current;
    m_current = temp;

    m_time += dt;
}

void 
ParticleSystem::DrawQuad(int w, int h, int tw, int th)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0,         0);         glVertex2f(0,        0);
    glTexCoord2f((float)tw, 0);         glVertex2f((float)w, 0);
    glTexCoord2f((float)tw, (float)th); glVertex2f((float)w, (float) h);
    glTexCoord2f(0,         (float)th); glVertex2f(0,        (float) h);
    glEnd();
}

void 
ParticleSystem::DrawTerrain()
{
    int h = m_terrain_img.get_height();
    int w = m_terrain_img.get_width();

    for(int y=0; y<h-1; y++) {
        float v = y / (float) h;
        float v2 = (y+1) / (float) h;
        glBegin(GL_QUAD_STRIP);
        for(int x=0; x<w; x++) {
            float u = x / (float) w;
            float h = m_terrain_img(x, y) / 255.0f;
            float h2 = m_terrain_img(x, y+1) / 255.0f;

            glTexCoord2f(u, v);
			glVertex3f(u, h, v);

            glTexCoord2f(u, v2);
			glVertex3f(u, h2, v2);
        }
        glEnd();
    }
}

void
ParticleSystem::DisplayTerrain()
{
    glColor3f(0.5, 1.0, 0.5);
    glBindTexture(GL_TEXTURE_2D, m_terrain_tex);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(m_terrain_offset[0], m_terrain_offset[1], m_terrain_offset[2]);
    glScalef(m_terrain_scale[0], m_terrain_scale[1], m_terrain_scale[2]);
    glCallList(m_terrain_dlist);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

// read frame buffer to main memory
void 
ParticleSystem::ReadBack()
{
    m_pos_buffer[m_current]->Activate();
    glReadPixels(0, 0, m_w, m_h, GL_RGB, GL_FLOAT, (float *) m_pos);
    m_pos_buffer[m_current]->Deactivate();
}

// display particle system
void 
ParticleSystem::Display()
{
    // draw particles
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(1.0);
    glBegin(GL_POINTS);
    for(int i=0; i<m_w*m_h; i++) {
        glVertex3fv( (float *) &m_pos[i] );
    }
    glEnd();
}

void 
ParticleSystem::InitVertexArray()
{
    // create texture coords
    m_texCoords = new float [m_w * m_h * 3];
    float *texCoords = m_texCoords;
    for(int y=0; y<m_h; y++) {
        for(int x=0; x<m_w; x++) {
            *texCoords++ = (float)x - 0.5f;
            *texCoords++ = (float)y - 0.5f;
            *texCoords++ = 0.0f;
        }
    }

#if 0
    // use vbo for texcoords
    glGenBuffersARB(1, &m_texCoordBuffer);
    glBindBufferARB(GL_ARRAY_BUFFER, m_texCoordBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER, m_w*m_h*3*sizeof(float), m_texCoords, GL_STATIC_DRAW);
    glTexCoordPointer(3, GL_FLOAT, 0, 0);
#endif
}

void
ParticleSystem::Draw()
{
    cgGLBindProgram(m_shader_vprog);
    cgGLEnableProfile(CG_PROFILE_VP20);

    cgGLSetStateMatrixParameter(m_shader_modelView_param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(m_shader_modelViewProj_param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetParameter4f(m_shader_time_param, m_time, 0.0, 0.0, 0.0);
//    printf("%f\n", m_time);

    glColor4f(1.0, 1.0, 1.0, m_point_alpha);

    m_vertexArray[m_current]->SetPointer(0);
    glEnableVertexAttribArrayARB(0);
    glEnableVertexAttribArrayARB(8);
    glDrawArrays(GL_POINTS, 0, m_w*m_h);
    glDisableVertexAttribArrayARB(0);
    glDisableVertexAttribArrayARB(8);
    cgGLDisableProfile(CG_PROFILE_VP20);
}

// bind current and previous positions as vertex attributes, use vertex program to interpolate
void
ParticleSystem::DrawMotionBlurred(int passes)
{
    cgGLBindProgram(m_shader_mb_vprog);
    cgGLEnableProfile(CG_PROFILE_VP20);
    cgGLSetStateMatrixParameter(m_shader_mb_modelView_param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(m_shader_mb_modelViewProj_param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetParameter4f(m_shader_mb_time_param, m_time, 0.0, 0.0, 0.0);

    m_vertexArray[m_previous]->SetPointer(0);
    m_vertexArray[m_current]->SetPointer(1);
    glEnableVertexAttribArrayARB(0);
    glEnableVertexAttribArrayARB(1);
    glEnableVertexAttribArrayARB(8);

    glColor4f(1.0, 1.0, 1.0, m_point_alpha / passes);

    for(int i=0; i<passes; i++) {
        float t = i / (float) (passes-1);
        cgGLSetParameter1f(m_shader_mb_interp_param, t);
        glDrawArrays(GL_POINTS, 0, m_w*m_h);
    }

    glDisableVertexAttribArrayARB(0);
    glDisableVertexAttribArrayARB(1);
    glDisableVertexAttribArrayARB(8);
    cgGLDisableProfile(CG_PROFILE_VP20);
}

// display shaded mesh using render-to-vertex array
void 
ParticleSystem::DisplayFast(bool motion_blur, bool use_sprites)
{
    // copy geometry from framebuffer
    m_pos_buffer[m_current]->Activate();
    if (m_useMRT) {
        m_vertexArray[m_current]->Read(GL_AUX0, m_w, m_h);
    } else {
        m_vertexArray[m_current]->Read(GL_FRONT, m_w, m_h);
    }
    m_pos_buffer[m_current]->Deactivate();

    if (use_sprites) {
      // setup shader
      cgGLBindProgram(m_shader_fprog);
      cgGLEnableProfile(CG_PROFILE_FP30);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, m_sprite_tex);

      // setup point sprites
      glEnable(GL_POINT_SPRITE_ARB);
      glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
      glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_NV);
//      glPointSize(m_pointsize);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glEnable(GL_BLEND);
      glDepthMask(GL_FALSE);

      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_LESS, 0.5);

    } else {
      glPointSize(1.0);
    }

    // draw the geometry
    if (motion_blur)
        DrawMotionBlurred(m_blur_passes);
    else
        Draw();

    glDisable(GL_POINT_SPRITE_ARB);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_NV);
    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDisable(GL_ALPHA_TEST);

    cgGLDisableProfile(CG_PROFILE_FP30);
}

// geometry expansion
// read from position buffer, write to larger geometry buffer
void
ParticleSystem::ExpandGeometry()
{
    m_geom_buffer->Activate();

    cgGLBindProgram(m_expand_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    m_pos_buffer[m_current]->Bind(WGL_FRONT_LEFT_ARB);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    m_pos_buffer[m_previous]->Bind(WGL_FRONT_LEFT_ARB);

    DrawQuad(m_w*m_expand, m_h, m_w, m_h);

    m_pos_buffer[m_current]->Release(WGL_FRONT_LEFT_ARB);
    m_pos_buffer[m_previous]->Release(WGL_FRONT_LEFT_ARB);
    cgGLDisableProfile(CG_PROFILE_FP30);

    m_geom_buffer->Deactivate();
}

// display particles as lines between previous and current position
void
ParticleSystem::DisplayLines(bool smooth)
{
    ExpandGeometry();

    // copy geometry from framebuffer
    m_geom_buffer->Activate();
    m_vertexArray[m_current]->Read(GL_FRONT, m_w*m_expand, m_h);
    m_geom_buffer->Deactivate();

    cgGLBindProgram(m_shader_vprog);
    cgGLEnableProfile(CG_PROFILE_VP20);
    cgGLSetStateMatrixParameter(m_shader_modelView_param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(m_shader_modelViewProj_param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    glColor4f(1.0, 1.0, 1.0, 1.0);
    if (smooth) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
    }

    m_vertexArray[m_current]->SetPointer(0);
    glEnableVertexAttribArrayARB(0);
    glDrawArrays(GL_LINES, 0, GetSize()*m_expand);
    glDisableVertexAttribArrayARB(0);
    cgGLDisableProfile(CG_PROFILE_VP20);

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDepthMask(GL_TRUE);
}

// display float texture using texturing
void 
ParticleSystem::DisplayTexture(RenderTexture *pbuffer, GLenum buffer, int w, int h)
{
    pbuffer->Bind(buffer);

    cgGLBindProgram(m_passthru_fprog);
    cgGLEnableProfile(CG_PROFILE_FP30);
    DrawQuad(w, h, w, h);
    cgGLDisableProfile(CG_PROFILE_FP30);

    pbuffer->Release(buffer);
}

// display position and velocity textures
void 
ParticleSystem::DisplayTextures()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);

    // position
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(0.25f, 0.25f, 1.0f);
    cgGLSetParameter4f(m_passthru_scale_param, 0.25, 0.25, 0.25, 1.0);
    cgGLSetParameter4f(m_passthru_bias_param, 0.5, 0.0, 0.5, 0.0);
    if (m_useMRT) {
        DisplayTexture(m_pos_buffer[m_current], WGL_AUX0_ARB, m_w, m_h);
    } else {
        DisplayTexture(m_pos_buffer[m_current], WGL_FRONT_LEFT_ARB, m_w, m_h);
    }

    // velocity
    glTranslatef(m_w, 0.0f, 0.0f);
    cgGLSetParameter4f(m_passthru_scale_param, 0.5, 0.5, 0.5, 1.0);
    cgGLSetParameter4f(m_passthru_bias_param, 0.5, 0.5, 0.5, 0.0);
    if (m_useMRT) {
        DisplayTexture(m_pos_buffer[m_current], WGL_AUX1_ARB, m_w, m_h);
    } else {
        DisplayTexture(m_vel_buffer[m_current], WGL_FRONT_LEFT_ARB, m_w, m_h);
    }

#if 0
    // expanded geometry
    glTranslatef(m_w, 0.0f, 0.0f);
    cgGLSetParameter4f(m_passthru_scale_param, 1.0, 1.0, 1.0, 1.0);
    cgGLSetParameter4f(m_passthru_bias_param, 0.0, 0.0, 0.0, 0.0);
    DisplayTexture(m_geom_buffer, WGL_FRONT_LEFT_ARB, m_w*m_expand, m_h);
#endif

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
