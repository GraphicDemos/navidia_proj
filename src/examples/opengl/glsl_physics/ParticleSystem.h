/*
  A particle system that represents particle positions using float textures
  It uses two float textures to represent the current and previous particle positions
  A fragment program calculates the new positions using Verlet integration,
  and writes the result to a float buffer

  sgreen 6/2002
*/

#include <glh/glh_linear.h>
#include <shared/pbuffer.h>
#include <shared/renderVertexArray.h>

using namespace glh;

class ParticleSystem {
public:
  ParticleSystem(int w, int h, int iterations);
  ~ParticleSystem();

  int GetWidth() { return m_w; }
  int GetHeight() { return m_h; }

  void SetSphere(vec3f pos);

  void InitGrid(vec3f start, vec3f du, vec3f dv);    // initialize particles in grid pattern
  void TimeStep(float dt);    // step the simulation
  void ReadBack();

  void Display();         // display particle system as points and lines
  void DisplayShaded();   // display shaded mesh
  void DisplayShaded2();  // display shaded mesh using render-to-vertex array

  void DisplayTextures(); // display position and normal textures

private:
  PBuffer *CreatePBuffer(char *mode, GLuint &tex, bool filter);
  void addShader(GLhandleARB shaderObject, const GLcharARB *shaderSource, GLenum shaderType, bool doLink);
  GLhandleARB loadProgram(char *vertFilename, char *fragFilename, data_path &path, bool doLink);
  GLhandleARB loadProgram(char *fragFilename, data_path &path, bool doLink);
  void InitShaders();

  void DrawImage();
  void CalculateNormals();
  void DrawQuad();

  // display float texture using texturing
  void DisplayTexture(PBuffer *pbuffer, GLuint tex);
  void InitVertexArray();


  int m_w, m_h;             // size of grid
  int m_iterations;         // number of constraint iterations

  vec3f *m_x;               // array of positions
  float *m_debug;

  float m_damping;
  vec3f m_gravity;

  // float pbuffers representing current and previous positions, plus destination
  PBuffer *m_pbuffer[3];
  GLuint m_pbuffer_tex[3];
  PBuffer *m_normal_pbuffer;
  GLuint m_normal_pbuffer_tex;

  int m_current, m_previous, m_dest;

  GLuint m_cloth_tex;

  // fragment programs
  GLhandleARB m_physicsProg, m_physicsProg2;
  GLhandleARB m_passthru_prog, m_normal_prog, m_shader_prog; 

  // program parameters
  GLint m_timestep_param, m_damping_param, m_gravity_param;

  GLint m_constraintDist_param;
  GLint m_spherePos_param, m_sphereRadius_param;

  GLint m_scale_param, m_bias_param;

  RenderVertexArray *m_vertexArray;
  GLuint m_texCoordBuffer;
  short *m_indices;
  float *m_texCoords;
  int m_nindices;
};
