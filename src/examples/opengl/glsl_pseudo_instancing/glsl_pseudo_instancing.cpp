
//////////////////////////////////////////////////////////////////////////////////
//                               Important Functions
//////////////////////////////////////////////////////////////////////////////////
// renderScene()
//

#if defined(WIN32)
#  include <windows.h>
#endif
#define GLH_EXT_SINGLE_FILE

#include "ARBProgram.h"
#include "Average.h"
#include "File.h"
#include "InstanceConfiguration.h"
#include "Mesh.h"
#include "Program.h"
#include "SphereMesh.h"
#include "Timer.h"

#include <assert.h>
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_linear.h>
#include <math.h>
#include <shared/array_texture.h>
#include <nv_png.h>
#include <shared/quitapp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace glh;



// Structs
struct InstancingUniformLocations
{
  GLint mViewMatrixLocation;
  GLint mDecalMapLocation;
  GLint mLightPositionViewLocation;
};

struct NonInstancingUniformLocations
{
  GLint mTimeLocation;
  GLint mWorldMatrixLocation;
  GLint mViewMatrixLocation;
  GLint mDecalMapLocation;
};

struct Instance
{
  float mWorld[4][4];     // World matrix for instance
  float mColor[4];        // Color for instance
};


// Global Variables
bool                          gKeyState[256];
glut_simple_mouse_interactor  gMouseInteractor;
Program*                      gInstancingProgram;
Program*                      gNonInstancingProgram;
ARBProgram*                   gInstancingVertexProgram;
ARBProgram*                   gNonInstancingVertexProgram;
ARBProgram*                   gFragmentProgram;
InstancingUniformLocations    gInstancingUniformLocations;
NonInstancingUniformLocations gNonInstancingUniformLocations;
vector<Instance>              gInstances;
int                           gCurrentMeshIndex    = 0;
Mesh*                         gCurrentMesh         = NULL;
vector<Mesh*>                 gMeshes;
GLuint                        gTexture             = 0;
bool                          gUseInstancing       = true;
bool                          gUseGLSL             = true;
bool                          gDisableGLSL         = false;
bool                          gDisableText         = false;
float                         gTime                = 0.0f;
float                         gDeltaTime           = 0.0f;
float                         gTheta               = 0.0f;
Average                       gFrameDurationAverage;
float                         gFrameDuration      = 1.0f;
int                           gInstanceCount      = 0;
float                         gInstancesPerSecond = 0.0f;
InstanceConfiguration         gInstanceConfiguration;
int                           gWindowWidth         = 1;
int                           gWindowHeight        = 1;
Timer                         gTimer;


// Function declarations
void  buildInstances(void);
void  display(void);
int   glPrintf(int x, int y, const char* formatString, ...);
void  idle(void);
void  initOpenGL(void);
void  key(unsigned char k, int x, int y);
void  menu(int id);
void  motion(int x, int y);
void  mouse(int button, int state, int x, int y);
float random(float min, float max);
void  random(vec3f& v);
void  renderScene(void);
void  resize(int w, int h);


int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitWindowSize(512, 512);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  glutCreateWindow("glsl_pseudo_instancing");

  initOpenGL();

  // Build the world matrices and colors for all of the instances
  buildInstances();

  gMouseInteractor.configure_buttons(1);
  gMouseInteractor.dolly.dolly[2] = -90.0f;
  gMouseInteractor.dolly.scale = 1.0f;
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIdleFunc(idle);
  glutKeyboardFunc(key);
  glutReshapeFunc(resize);

  glutCreateMenu(menu);
  glutAddMenuEntry("Toggle pseudo-instancing [i]", 'i');
  if(gDisableGLSL == false)
    {
      glutAddMenuEntry("Toggle between GLSL shader and ARB programs [g]", 'g');
    }
  glutAddMenuEntry("Decrease mesh vertex count [1]", '1');
  glutAddMenuEntry("Increase mesh vertex count [2]", '2');
  glutAddMenuEntry("Decrease instance count [3]", '3');
  glutAddMenuEntry("Increase instance count [4]", '4');
  glutAddMenuEntry("quit [esc]", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // Flip the space bar state so that things continue spinning by default
  gKeyState[' '] = true;

  glutMainLoop();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//         Creates instance specific data for all instances
//
////////////////////////////////////////////////////////////////////////////////
void buildInstances(void)
{
  int         i, j, k, l;
  float       scale;
  vec3f       position;
  float       world[4][4];
  vec3f       orientationAxis;
  quaternionf orientationQuaternion;
  matrix4f    orientationMatrix;
  vec3f       color;
  Instance    instance;

  // Throw away all existing instances
  gInstances.resize(0);

  // Set the rand() seed so what get the same result over and over
  srand(0);

  scale = 3.0f * sqrt(1024.0f / float(gInstanceConfiguration.count()));
  for(i=0; i<gInstanceConfiguration.rows(); i++)
    {
      for(j=0; j<gInstanceConfiguration.columns(); j++)
        {
          // Compute the instance position
          random(position);
          position *= 50.0f + random(-5.0f, 5.0f);

          // Compute the orientation
          random(orientationAxis);
          orientationQuaternion = quaternionf(orientationAxis, random(0.0f, 6.28f));
          orientationQuaternion.get_value(orientationMatrix);

          // Copy the orientation matrix into the world matrix
          for(k=0; k<3; k++)
            {
              for(l=0; l<3; l++)
                {
                  world[k][l] = orientationMatrix.element(k, l);
                }
            }
          

          // Build the world matrix
          world[0][0] *= scale;        world[1][0] *= scale;        world[2][0] *= scale;        world[3][0] = 0.0f;
          world[0][1] *= scale;        world[1][1] *= scale;        world[2][1] *= scale;        world[3][1] = 0.0f;
          world[0][2] *= scale;        world[1][2] *= scale;        world[2][2] *= scale;        world[3][2] = 0.0f;
          world[0][3]  = position[0];  world[1][3]  = position[1];  world[2][3]  = position[2];  world[3][3] = 1.0f;

          // Copy the world matrix into the instance's world matrix
          assert(sizeof(world) == sizeof(instance.mWorld));
          memcpy(instance.mWorld, world, sizeof(instance.mWorld));

          // Compute the color for the instance
          color = vec3f(random(0.01f, 1.0f), random(0.01f, 1.0f), random(0.01f, 1.0f));
          color.normalize();
          instance.mColor[0] = color[0];
          instance.mColor[1] = color[1];
          instance.mColor[2] = color[2];
          
          // Store the instance data
          gInstances.push_back(instance);
        }
    }  
}


////////////////////////////////////////////////////////////////////////////////
//
//            glut callback for rendering the scene
//
////////////////////////////////////////////////////////////////////////////////
void display(void)
{
  matrix4f view;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Make sure that the camera doesn't get too crazy
  if(gMouseInteractor.dolly.dolly[2] > 0.0f)
    {
      gMouseInteractor.dolly.dolly[2] = 0.0f;
    }
  else if(gMouseInteractor.dolly.dolly[2] < -175.0f)
    {
      gMouseInteractor.dolly.dolly[2] = -175.0f;
    }

#ifdef LOAD_SHADERS_EVERY_FRAME
  gInstancingVertexProgram->load("vertexProgramInstancing.vp");
  gFragmentProgram->load("fragmentProgram.fp");
#endif


  // Do the actual rendering
  renderScene();

  // Keep track of time
  gDeltaTime = gTimer.tick();
  gTime += gDeltaTime;



  // Keep track of some statistics
  gFrameDurationAverage.addSample(gDeltaTime);
  gInstanceCount += gInstanceConfiguration.count();
  if(gFrameDurationAverage.sum() > 0.5f)
    {
      gInstancesPerSecond = float(gInstanceCount) / gFrameDurationAverage.sum();
      gInstanceCount = 0;

      gFrameDuration = gFrameDurationAverage.average();
      gFrameDurationAverage.reset();

      // If rendering to the OpenGL window is disabled, spit the text out to the console
      if(gDisableText == true)
        {
          printf("fps                = %3.1f\n", 1.0f / gFrameDuration, gFrameDuration);
          printf("instances / second = %.0f\n", gInstancesPerSecond);
          printf("instances / frame  = %d\n", gInstanceConfiguration.count());
          printf("tris / mesh        = %d\n", gCurrentMesh->triangleCount());
          printf("Mtris / second     = %.1f\n", (gInstancesPerSecond * gCurrentMesh->triangleCount()) / 1000000.0f);
          printf("\n");
        }
    }

  // Render the stats to the OpenGL window
  if(gDisableText == false)
    {
      int y;

      // Print out the stats
      glColor3f(1.0f, 1.0f, 1.0f);
      y = 0;
      glDisable(GL_DEPTH_TEST);
      glPrintf(0, y++, "fps                = %3.1f", 1.0f / gFrameDuration, gFrameDuration);
      glPrintf(0, y++, "instances / second = %.0f", gInstancesPerSecond);
      glPrintf(0, y++, "instances / frame  = %d", gInstanceConfiguration.count());
      glPrintf(0, y++, "tris / mesh        = %d", gCurrentMesh->triangleCount());
      glPrintf(0, y++, "Mtris / second     = %.1f", (gInstancesPerSecond * gCurrentMesh->triangleCount()) / 1000000.0f);

      if(gUseInstancing)
        {
          glPrintf(0, y++, "Pseudo-instancing enabled");
        }
      else
        {
          glPrintf(0, y++, "Pseudo-instancing disabled");
        }

      if(gUseGLSL)        
        {
          glPrintf(0, y++, "Using GLSL Shader");
        }
      else
        {
          glPrintf(0, y++, "Using ARB programs");
        }

      glEnable(GL_DEPTH_TEST);
    }

  glutSwapBuffers();
}


////////////////////////////////////////////////////////////////////////////////
//
//         A printf like function for printing text with OpenGL
//
////////////////////////////////////////////////////////////////////////////////
int glPrintf(int x, int y, const char* formatString, ...)
{
  int     returnValue;
  va_list argList;
  char    outputString[4096];
  int     index;
  float   raster[2];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  raster[0] = 2.0f * ((x * 9.0f) / float(gWindowWidth)) - 1.0f;
  raster[1] = 1.0f - 2.0f * (((y + 1) * 15.0f) / float(gWindowHeight));
  glRasterPos2fv(raster);

  va_start(argList, formatString);
  returnValue = vsprintf(outputString, formatString, argList);
  va_end(argList);

  index = 0;
  while(outputString[index] != NULL)
    {
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, outputString[index]);
      index++;

      assert(index < sizeof(outputString));
    }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return(returnValue);
}


////////////////////////////////////////////////////////////////////////////////
//
//           GLUT callback for twiddling fingers
//
////////////////////////////////////////////////////////////////////////////////
void idle(void)
{
  if (gKeyState[' '])
    gMouseInteractor.trackball.increment_rotation();
    
  glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
//
//              Initialize start-up OpenGL state
//
////////////////////////////////////////////////////////////////////////////////
void initOpenGL(void)
{
  array2<vec3ub> textureData;

  // Check for minimum extension support
  if(!glh_init_extensions("GL_VERSION_1_2 GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_vertex_buffer_object"))
    {
      printf("Necessary extensions unsupported: %s\n", glh_get_unsupported_extensions());
      quitapp(-1);
    }

  // Check for GLSL support
  if(!glh_init_extensions("GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader"))
    {
      printf("GLSL not supported. Using ARB_vertex_program/ARB_fragment_program\n");
      gDisableGLSL = true;
      gUseGLSL = false;
    }

  // Turn off wait on vsync on win32
#ifdef WIN32
  if(glh_init_extensions("WGL_EXT_swap_control"))
    {
      wglSwapIntervalEXT(0);
    }
#endif

  // Set up the path to load files from
  File::addPath(".");
  File::addPath("../../../MEDIA/programs/glsl_pseudo_instancing");
  File::addPath("../../../../MEDIA/programs/glsl_pseudo_instancing");
  File::addPath("../../../../../../../MEDIA/programs/glsl_pseudo_instancing");

  // Load glsl shaders
  if(gDisableGLSL == false)
    {
      gInstancingProgram = new Program("vertexShaderInstancing.glsl", "fragmentShader.glsl");
      gNonInstancingProgram = new Program("vertexShaderNonInstancing.glsl", "fragmentShader.glsl");

      // Get the uniform locations for the shaders
      gInstancingUniformLocations.mViewMatrixLocation = glGetUniformLocationARB(*gInstancingProgram, "viewMatrix");
      gInstancingUniformLocations.mDecalMapLocation = glGetUniformLocationARB(*gInstancingProgram, "decalMap");
      gInstancingUniformLocations.mLightPositionViewLocation = glGetUniformLocationARB(*gInstancingProgram, "lightPositionView");

      gNonInstancingUniformLocations.mTimeLocation = glGetUniformLocationARB(*gNonInstancingProgram, "time");
      gNonInstancingUniformLocations.mWorldMatrixLocation = glGetUniformLocationARB(*gNonInstancingProgram, "worldMatrix");
      gNonInstancingUniformLocations.mViewMatrixLocation = glGetUniformLocationARB(*gNonInstancingProgram, "viewMatrix");
      gNonInstancingUniformLocations.mDecalMapLocation = glGetUniformLocationARB(*gNonInstancingProgram, "decalMap");
    }
  else
    {
      gInstancingProgram = NULL;
      gNonInstancingProgram = NULL;
    }

  // Load ARB_vertex_program programs
  gInstancingVertexProgram = new ARBProgram(GL_VERTEX_PROGRAM_ARB, "vertexProgramInstancing.vp");
  gNonInstancingVertexProgram = new ARBProgram(GL_VERTEX_PROGRAM_ARB, "vertexProgramNonInstancing.vp");
  gFragmentProgram = new ARBProgram(GL_FRAGMENT_PROGRAM_ARB, "fragmentProgram.fp"); 

  // Create the meshes
  gMeshes.push_back(new SphereMesh(2, 2));
  gMeshes.push_back(new SphereMesh(3, 4));
  gMeshes.push_back(new SphereMesh(3, 5));
  gMeshes.push_back(new SphereMesh(4, 4));
  gMeshes.push_back(new SphereMesh(4, 5));
  gMeshes.push_back(new SphereMesh(5, 5));
  gMeshes.push_back(new SphereMesh(6, 6));
  gMeshes.push_back(new SphereMesh(7, 7));
  gMeshes.push_back(new SphereMesh(8, 8));
  gMeshes.push_back(new SphereMesh(16, 8));
  gMeshes.push_back(new SphereMesh(32, 12));
  gMeshes.push_back(new SphereMesh(64, 12));
  gMeshes.push_back(new SphereMesh(128, 12));
  gMeshes.push_back(new SphereMesh(256, 12));
  gCurrentMeshIndex = 0;
  gCurrentMesh = gMeshes[gCurrentMeshIndex];

  // Load the nvidia logo
  read_png_rgb("graydirt.png", textureData);

  glGenTextures(1, &gTexture);
  glBindTexture(GL_TEXTURE_2D, gTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  make_rgb_texture(textureData, true);

  // Setup misc state
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}



////////////////////////////////////////////////////////////////////////////////
//
//              GLUT callback for key presses
//
////////////////////////////////////////////////////////////////////////////////
void key(unsigned char k, int x, int y)
{
  gKeyState[k] = ! gKeyState[k];

  switch(k)
    {
    case 27:
    case 'q':
      {
        exit(0);
        break;
      }
    case 'i':
      {
        gUseInstancing = !gUseInstancing;
        break;
      }
    case 'g':
      {
        if(gDisableGLSL == false)
          {
            gUseGLSL = !gUseGLSL;
          }
        else
          {
            gUseGLSL = false;
          }
        break;
      }
    case '1':
      {
        assert(gMeshes.size() > 0);

        gCurrentMeshIndex--;
        if(gCurrentMeshIndex < 0)
          {
            gCurrentMeshIndex = 0;
          }
        gCurrentMesh = gMeshes[gCurrentMeshIndex];

        assert(gCurrentMesh != NULL);
        break;
      }
    case '2':
      {
        assert(gMeshes.size() > 0);

        gCurrentMeshIndex++;
        if(gCurrentMeshIndex >= (int)gMeshes.size())
          {
            gCurrentMeshIndex = (int)gMeshes.size() - 1;
          }
        gCurrentMesh = gMeshes[gCurrentMeshIndex];

        assert(gCurrentMesh != NULL);
        break;
      }
    case '3':
      {
        gInstanceConfiguration.changeState(-1);
        buildInstances();
        break;
      }
    case '4':
      {
        gInstanceConfiguration.changeState(+1);
        buildInstances();
        break;
      }
    case '6':
      {
        gDisableText = !gDisableText;
        break;
      }
    }
  

  gMouseInteractor.keyboard(k, x, y);
    
  glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
//
//              GLUT callback for menu events
//
////////////////////////////////////////////////////////////////////////////////
void menu(int id)
{
  key((unsigned char)id, 0, 0);
}


////////////////////////////////////////////////////////////////////////////////
//
//              GLUT callback for mouse drag events
//
////////////////////////////////////////////////////////////////////////////////
void motion(int x, int y)
{
  gMouseInteractor.motion(x, y);
}


////////////////////////////////////////////////////////////////////////////////
//
//              GLUT callback for mouse button press events
//
////////////////////////////////////////////////////////////////////////////////
void mouse(int button, int state, int x, int y)
{
  gMouseInteractor.mouse(button, state, x, y);
}



////////////////////////////////////////////////////////////////////////////////
//
//              Computes a random float value in a specified range
//
////////////////////////////////////////////////////////////////////////////////
float random(float min, float max)
{
  assert(min < max);  // This isn't really necessary
 
  return(min + (max - min) * (float(rand() % 1024) / 1024.0f));
}


////////////////////////////////////////////////////////////////////////////////
//
//              Computes a random unit vector
//
////////////////////////////////////////////////////////////////////////////////
void random(vec3f& v)
{
  float length;
  
  v[0] = random(-1.0f, 1.0f);
  v[1] = random(-1.0f, 1.0f);
  v[2] = random(-1.0f, 1.0f);

  length = v.normalize();
  if(length < .001f)
    {
      v = vec3f(1.0f, 0.0f, 0.0f);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//              This does the actual scene rendering
//
////////////////////////////////////////////////////////////////////////////////
void renderScene(void)
{
  assert(gCurrentMesh != NULL);

  int      i;
  int      count;
  matrix4f view;
  vec3f    light;
  vec3f    lightWorldSpace(0.0f, 100.0f, 0.0f);
  vec3f    lightPositionView;

  // Set state for rendering the mesh
  gCurrentMesh->setState();
 
  // Get the view transform from the mouse interactor
  view = gMouseInteractor.get_transform();

  // Compute the light's position in view space
  view.mult_matrix_vec(lightWorldSpace, lightPositionView);
  lightPositionView = vec3f(0.0f, 0.0f, 0.0f);


  //
  // Set up the program
  //
  if(gUseGLSL)
    {
      if(gUseInstancing)
        {
          glUseProgramObjectARB(*gInstancingProgram);
          glUniformMatrix4fvARB(gInstancingUniformLocations.mViewMatrixLocation, 1, false, &view(0,0));
          glUniform1iARB(gInstancingUniformLocations.mDecalMapLocation, 0);
          glUniform3fvARB(gInstancingUniformLocations.mLightPositionViewLocation, 1, &lightPositionView[0]);
        }
      else
        {
          glUseProgramObjectARB(*gNonInstancingProgram);
          glUniformMatrix4fvARB(gNonInstancingUniformLocations.mViewMatrixLocation, 1, false, &view(0, 0));
          glUniform1iARB(gNonInstancingUniformLocations.mDecalMapLocation, 0);
        }
    }
  else
    {
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, *gFragmentProgram);
      glEnable(GL_VERTEX_PROGRAM_ARB);

      if(gUseInstancing)
        {
          glBindProgramARB(GL_VERTEX_PROGRAM_ARB, *gInstancingVertexProgram);
        }
      else
        {
          glBindProgramARB(GL_VERTEX_PROGRAM_ARB, *gNonInstancingVertexProgram);
        }

      // GL_PROJECTION matrix gets stuck in local parameters [0..3]
              
      // Download the view matrix
      glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 4, view(0, 0), view(0, 1), view(0, 2), view(0, 3));
      glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 5, view(1, 0), view(1, 1), view(1, 2), view(1, 3));
      glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 6, view(2, 0), view(2, 1), view(2, 2), view(2, 3));
      glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 7, view(3, 0), view(3, 1), view(3, 2), view(3, 3));

      // Download the light position
      glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 11, lightPositionView[0], lightPositionView[1], lightPositionView[2], 1.0f); 
    }


  //
  // Render the instances of the mesh
  //
  count = (int)gInstances.size();
  for(i=0; i<count; i++)
    {          
      // Send down the instance's world matrix
      if(gUseInstancing)
        {
          // Use texture coordinates instead of constants. This is the crux of
          // the pseudo-instancing technique
          glMultiTexCoord4fv(GL_TEXTURE1, gInstances[i].mWorld[0]);
          glMultiTexCoord4fv(GL_TEXTURE2, gInstances[i].mWorld[1]);
          glMultiTexCoord4fv(GL_TEXTURE3, gInstances[i].mWorld[2]);
        }
      else
        {
          // Download the world matrix for the instance
          if(gUseGLSL)
            {
              glUniform4fvARB(gNonInstancingUniformLocations.mWorldMatrixLocation, 3, (float*)(gInstances[i].mWorld));
            }
          else
            {
              glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, gInstances[i].mWorld[0]);
              glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 9, gInstances[i].mWorld[1]);
              glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 10, gInstances[i].mWorld[2]);                    
            }
        }

      // Set the color for the instance
      glColor4fv(gInstances[i].mColor);

      // Render the instance  (call glDrawElements())
      gCurrentMesh->render();
    }

  // Undo state settings
  gCurrentMesh->unsetState();

  if(gUseGLSL)
    {
      glUseProgramObjectARB(0);
    }
  else
    {
      glDisable(GL_VERTEX_PROGRAM_ARB);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//              GLUT callback for window resizes
//
////////////////////////////////////////////////////////////////////////////////
void resize(int w, int h)
{
  if (h == 0) h = 1;

  gWindowWidth = w;
  gWindowHeight = h;

  glViewport(0, 0, w, h);
    
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.0, double(w) / double(h), 1.0, 250.0);
    
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gMouseInteractor.reshape(w, h);
}

