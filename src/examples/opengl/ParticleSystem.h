/*
  A particle system that represents particle positions using float textures
  It uses two float textures to represent the current and previous particle positions
  A fragment program calculates the new positions using Verlet integration,
  and writes the result to a float buffer

  sgreen 6/2002
*/

#include <Cg/cgGL.h>
#include <glh/glh_linear.h>
#include "shared/RenderTexture.h"
#include "shared/renderVertexArray.h"
#include <list>

using namespace glh;

struct ParticleData {
    float birth_time;
    float death_time;
    float pad[2];
};

struct NewParticle {
  int index;
  vec3f pos;
  vec3f vel;
  ParticleData data;
};

struct ActiveParticle
{
	float death_time;
	int index;
};

const GLenum mrt_buffers[] = {
    GL_AUX0,  // pos
    GL_AUX1,  // vel
};

const GLenum mrt_buffers_wgl[] = {
    WGL_AUX0_ARB,  // pos
    WGL_AUX1_ARB,  // vel
};

class ParticleSystem {
public:
  ParticleSystem(int w, int h, int iterations, bool useMRT);
  ~ParticleSystem();

  int GetWidth() { return m_w; }
  int GetHeight() { return m_h; }
  int GetSize() { return m_w * m_h; }

  int GetNoActiveParticles() { return (int) m_activeParticles.size(); }

  void SetSphere(vec3f pos, vec3f vel);

  void InitParticlesGrid(vec3f start, vec3f du, vec3f dv);
  void InitParticlesRand(vec3f scale, vec3f offset);
  void InitParticlesStream(vec3f pos, vec3f dir, float pos_rand, float vel_rand);

  void Reset();
  void CreateParticle(vec3f pos, vec3f vel, float lifetime);
  void HoseEmitter(int no, vec3f pos, vec3f vel, float theta, float phi, float dir_rand, float vel_min, float vel_max, float lifetime);

  void TimeStep(float dt);    // step the simulation
  void TimeStep_MRT(float dt); // step the simulation using multiple draw buffers
  void ReadBack();

  void DisplayTerrain();

  void Display();
  void DisplayFast(bool motion_blur, bool use_sprites);
  void DisplayLines(bool smooth);

  void DisplayTextures(); // display textures

private:
  vec2f IndexToPos(int i) { return vec2f(i % m_w, (i / m_w) + 1); } // convert index to position in texture

  RenderTexture *CreatePBuffer(int w, int h, char *mode);
  CGprogram loadProgram(CGcontext context, CGprofile profile, CGenum program_type, char *filename, data_path &path);
  void InitCg();

  void InitBuffer(RenderTexture *pbuffer, GLenum buffer, vec3f *data);
  void DrawQuad(int w, int h, int tw, int th);

  void InitBuffers();

  void InitNewParticles();
  void FreeDeadParticles();

  void DrawTerrain();
  void Draw();
  void DrawMotionBlurred(int passes);
  void ExpandGeometry();

  // display float texture using texturing
  void DisplayTexture(RenderTexture *pbuffer, GLenum buffer, int w, int h);
  void InitVertexArray();

  int m_w, m_h;             // size of grid
  int m_iterations;         // number of constraint iterations

  vec3f *m_pos, *m_vel;     // array of positions

  float m_damping;
  vec3f m_gravity;
  float m_time;

  // float pbuffers representing current and previous positions and velocities
  RenderTexture *m_pos_buffer[2], *m_vel_buffer[2];
  RenderTexture *m_geom_buffer;
  int m_expand;
  int m_current, m_previous;

  // vertex arrays
  RenderVertexArray *m_vertexArray[2];
  GLuint m_texCoordBuffer;
  float *m_texCoords;

  ParticleData *m_particleData;
  GLuint m_particleDataBuffer;

  // allocation
  std::vector<int> m_freeIndices;
  std::list<ActiveParticle> m_activeParticles;
  std::vector<NewParticle> m_newParticles;

  // textures
  GLuint m_sprite_tex, m_terrain_tex;
  array2<unsigned char> m_terrain_img;

  GLuint m_terrain_dlist;
  vec3f m_terrain_scale, m_terrain_offset;

  // fragment programs and parameters
  CGprogram m_pos_fprog;
  CGparameter m_pos_timestep_param, m_pos_spherePos_param;

  CGprogram m_vel_fprog;
  CGparameter m_vel_timestep_param, m_vel_damping_param, m_vel_gravity_param;
  CGparameter m_vel_spherePos_param, m_vel_sphereVel_param, m_vel_sphereRadius_param;

  CGprogram m_passthru_fprog;
  CGparameter m_passthru_scale_param, m_passthru_bias_param;

  CGprogram m_copy_texcoord_fprog;

  CGprogram m_shader_fprog;
  CGprogram m_shader_vprog;
  CGparameter m_shader_modelView_param, m_shader_modelViewProj_param;
  CGparameter m_shader_time_param;

  CGprogram m_shader_mb_vprog; 
  CGparameter m_shader_mb_modelView_param, m_shader_mb_modelViewProj_param, m_shader_mb_time_param, m_shader_mb_interp_param;

  CGprogram m_expand_fprog;

  CGprogram m_posvel_fprog;
  CGparameter m_posvel_timestep_param;
  CGparameter m_posvel_damping_param;  
  CGparameter m_posvel_gravity_param;  
  CGparameter m_posvel_spherePos_param;
  CGparameter m_posvel_sphereVel_param;

  int m_blur_passes;
  float m_pointsize;
  float m_point_alpha;

  bool m_useMRT;
};
