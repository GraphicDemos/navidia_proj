/* 
 * OpenGL visualization of Jos Stam's Stable Fluids
 * sgg 9/2001
 */
#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>

#ifdef MACOS
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/glut.h>
#include <GL/glext.h>
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define GLH_EXT_SINGLE_FILE 1
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_NV_texture_shader "

#include <glh/glh_extensions.h>
#include <glh/glh_linear.h>

#include <shared/data_path.h>
#include <shared/quitapp.h>
#include <nvparse.h>

#include "texture.h"
#include "stablefluids.h"

using namespace glh;
using namespace std;

// simple class to hold fluid simulation data
class StableFluid {
public:
  StableFluid(int size, float visc = 0.001, float dt = 0.1) {
    m_size = size;
    m_visc = visc;
    m_dt = dt;

    init_FFT(m_size);

    int n = m_size*(m_size+2);  // required for fft
    m_u = new float[n]; m_v = new float[n];
    m_u0 = new float[n]; m_v0 = new float[n];

    reset();
  }

  // reset forces and velocities
  void reset() {
    for(int i=0; i<m_size*m_size; i++) {
      m_u[i] = m_v[i] = 0.0f;
      m_u0[i] = m_v0[i] = 0.0f;
    }
  }

  void randomize(float mag = 10.0f) {
    for(int i=0; i<m_size*m_size; i++) {
      m_u[i] = m_v[i] = 0.0f;
      m_u0[i] = sfrand() * mag;
      m_v0[i] = sfrand() * mag;
    }
  }

  // execute one step of the simulation
  void simulate() {
    stable_solve(m_size, m_u, m_v, m_u0, m_v0, m_visc, m_dt);
    // seems like we need to reset forces each step - is this right?
    for(int i=0; i<m_size*m_size; i++) {
      m_u0[i] = 0.0;
      m_v0[i] = 0.0;
    }
  }

  // get/set methods
  int getSize() { return m_size; }

  void setForce(int x, int y, float u, float v) {
    m_u0[y*m_size+x] = u;
    m_v0[y*m_size+x] = v;
  }

  void getForce(int x, int y, float &u, float &v) {
    u = m_u0[y*m_size+x];
    v = m_v0[y*m_size+x];
  }

  void getVel(int x, int y, float &u, float &v) {
    u = m_u[y*m_size+x];
    v = m_v[y*m_size+x];
  }

  vec2f getVel(int x, int y) {
    vec2f v;
    x = x & (m_size-1);
    y = y & (m_size-1);
    v[0] = m_u[y*m_size+x];
    v[1] = m_v[y*m_size+x];
    return v;
  }

  // bilinearly interpolate velocity
  vec2f getVelInterp(float x, float y) {
    int ix = (int) x;
    int iy = (int) y;
    vec2f v[4];
    v[0] = getVel(ix, iy);   v[1] = getVel(ix+1, iy);
    v[2] = getVel(ix, iy+1); v[3] = getVel(ix+1, iy+1);
    float fx = x - ix;
    float fy = y - iy;
    vec2f a = lerp(v[0], v[1], 1.0-fx);
    vec2f b = lerp(v[2], v[3], 1.0-fx);
    return lerp(a, b, 1.0-fy);
  }

private:
  float sfrand()
  {
    return ((rand() / (float) RAND_MAX) * 2.0f) - 1.0f;
  }

  vec2f lerp(vec2f a, vec2f b, float t) {
    return a + t*(b-a);
  }

  int   m_size;       // grid size
  float *m_u, *m_v;   // previous velocity
  float *m_u0, *m_v0; // force
  float m_visc;       // viscosity
  float m_dt;         // time set
};

// simple particle tracer visualization
class ParticleViz {
public:
  ParticleViz(StableFluid *f, int no) {
    m_fluid = f;
    m_no = no;
    m_p = new vec2f[no];
  }

  // initialize particles in grid formation
  void init(int w, int h, float spacing) {
    int i = 0;
    for(int y=0; y<h; y++) {
      for(int x=0; x<w; x++) {
        m_p[i++] = vec2f(x*spacing, y*spacing);
      }
    }
  }

  // move the particles according to velocity field
  void step(float scale) {
    for(int i=0; i<m_no; i++) {
      vec2f p = m_p[i];
//      vec2f v = m_fluid->getVelInterp(p[0], p[1]);
      vec2f v = m_fluid->getVel((int)p[0], (int)p[1]);
      m_p[i] += v * scale;
      if (m_p[i][0] > m_fluid->getSize()-1) m_p[i][0] -= m_fluid->getSize();
      if (m_p[i][0] < 0)                    m_p[i][0] += m_fluid->getSize();
      if (m_p[i][1] > m_fluid->getSize()-1) m_p[i][1] -= m_fluid->getSize();
      if (m_p[i][1] < 0)                    m_p[i][1] += m_fluid->getSize();
    }
  }

  void render() {
    glBegin(GL_POINTS);
    for(int i=0; i<m_no; i++) {
      glVertex2f(m_p[i][0], m_p[i][1]);
    }
    glEnd();
  }

private:
  StableFluid *m_fluid;
  int m_no;
  vec2f *m_p;
};

StableFluid *fluid;
ParticleViz *viz;

int display_mode = 0;

float gridsize = 4.0;
float force = 5;

int winw = 512, winh = 512;
int texw = 512, texh = 512;
int offsetTexSize = 128;

int ox, oy;
int buttonState = 0;

float tx = 0, ty = 0;
float rotz = 0.0;
float scale = 1.0;

bool toggle[256];

GLint ntexunits = 0;

float amp = 0.025;
GLfloat offsetMatrix[] = {
	amp, 0.0,
	0.0, amp
};

float pointsize = 2.0;

GLenum textarget = GL_TEXTURE_2D;
//GLenum textarget = GL_TEXTURE_RECTANGLE_NV;

GLuint checker, imgtex, src, recursive, offset_tex;
float blend_factor = .1;
Texture *tex;
int seedmode = 1;

float offset_scale = -20.0;

void initTexShaders();
GLuint CreateOffsetTexture();
void UpdateOffsetTexture(GLuint texid, StableFluid *f);
GLuint CreateImageTexture(int w, int h, int mode);

// get extension pointers
void getExts(void)
{
	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
	   printf("Necessary extensions are not supported: %s\n", glh_get_unsupported_extensions());
       quitapp(0);
    }

#ifdef GL_ARB_multitexture
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &ntexunits);
	printf("No. of texture units = %d\n", (int)ntexunits);
#endif

}

// check for OpenGL errors
void checkErrors(char *s)
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf("%s: error - %s\n", s, (char *) gluErrorString(error));
  }
}


// Rendering functions

float frand()
{
  return rand() / (float) RAND_MAX;
}

void texCoord(float s, float t)
{
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, s, t);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, s, t);
}

void drawPlane(float w, float h)
{
  glBegin(GL_QUADS);
  texCoord(0.0, 0.0);
  glVertex2f(0.0, 0.0);

  texCoord(1.0, 0.0);
  glVertex2f(w, 0.0);

  texCoord(1.0, 1.0);
  glVertex2f(w, h);

  texCoord(0.0, 1.0);
  glVertex2f(0.0, h);
  glEnd();

}

// seed image with random coloured points
void seed(int n)
{
  glEnable(GL_POINT_SMOOTH);
  glPointSize(pointsize);
  glBegin(GL_POINTS);
  for(int i=0; i<n; i++) {
    float c = frand();
//    glColor3f(frand(), frand(), frand());
    glColor3f(c, c, c);
    glVertex2i(rand() % texw, rand() % texh);
  }
  glEnd();
}

// seed image with random grey points along bottom row
void seed_smoke(int n)
{
  glEnable(GL_POINT_SMOOTH);
  glPointSize(pointsize);
  glBegin(GL_POINTS);
  for(int i=0; i<n; i++) {
    float c = frand();
    glColor3f(c, c, c);
    glVertex2i(rand() % texw, 0);
  }
  glEnd();

}

// seed image with random points from image
void seed_img(Texture *tex, int n)
{
  glEnable(GL_POINT_SMOOTH);
  glPointSize(pointsize);
  glBegin(GL_POINTS);
  for(int i=0; i<n; i++) {
    int x = rand() % texw;
    int y = rand() % texh;
    unsigned char *ptr = tex->image + (((tex->w * (y % tex->h)) + (x % tex->w)) * 3);
    glColor3ub(*ptr, *(ptr+1), *(ptr+2));
    glVertex2i(x, texh - y);
  }
  glEnd();
}


void seed_streamers(int step)
{
  glColor3f(0.0, 0.0, 0.0);

  // clear black bars
  float b = 5;
  glBegin(GL_QUADS);
  // bottom
  glVertex2f(0, 0);
  glVertex2f(texw, 0);
  glVertex2f(texw, b);
  glVertex2f(0, b);

  // top
  glVertex2f(0, texh);
  glVertex2f(texw, texh);
  glVertex2f(texw, texh-b);
  glVertex2f(0, texh-b);  

  // left
  glVertex2f(0, 0);
  glVertex2f(b, 0);
  glVertex2f(b, texh);
  glVertex2f(0, texh);

  // right
  glVertex2f(texw, 0);
  glVertex2f(texw-b, 0);
  glVertex2f(texw-b, texh);
  glVertex2f(texw, texh);

  glEnd();

  // draw white lines
  float w = 10;
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_LINES);
  int i;
  for(i=0; i<texh; i+=step) {
    glVertex2f(0, i);
    glVertex2f(w, i);
    glVertex2f(texw, i);
    glVertex2f(texw-w, i);
  }
  for(i=0; i<texw; i+=step) {
    glVertex2f(i, 0);
    glVertex2f(i, w);
    glVertex2f(i, texh);
    glVertex2f(i, texh-w);
  }

  glEnd();
}

void set_color(float x, float y)
{
	float f = atan2(y,x) / 3.1415927 + 1;
	float r = f;
	if(r > 1)
		r = 2 - r;
	float g = f + .66667;
    if(g > 2)
		g -= 2;
	if(g > 1)
		g = 2 - g;
	float b = f + 2 * .66667;
	if(b > 2)
		b -= 2;
	if(b > 1)
		b = 2 - b;
	glColor3f(r,g,b);
}

// display velocity field as lines
void drawVectorField(StableFluid *f, float linescale)
{
    glBegin(GL_LINES);
    for(int i=0; i<f->getSize(); i++) {
      for(int j=0; j<f->getSize(); j++) {
        float u, v;
        if (toggle['F'])
          f->getForce(i, j, u, v);
        else
          f->getVel(i, j, u, v);
		set_color(u,v);
        float x = i*gridsize;
        float y = j*gridsize;
        glVertex2f(x, y);
        glVertex2f(x + u*linescale, y + v*linescale);
      }
    }
    glEnd();
}

// as above, but normalize vector and use magnitude as color
void drawVectorField2(StableFluid *f, float colorScale)
{
    glBegin(GL_LINES);
    for(int i=0; i<f->getSize(); i++) {
      for(int j=0; j<f->getSize(); j++) {
        vec2f v = f->getVel(i, j);
        float m = v.normalize();
        float x = i*gridsize;
        float y = j*gridsize;
        glColor3f(m*colorScale, m*colorScale, m*colorScale);
        glVertex2f(x, y);
        glVertex2f(x + v[0]*gridsize, y + v[1]*gridsize);
      }
    }
    glEnd();
}

// visualize using recursive offset texturing
void drawFeedback()
{
    glEnable(GL_TEXTURE_SHADER_NV);
    glActiveTextureARB( GL_TEXTURE0_ARB);
    glBindTexture(textarget, offset_tex);
    glEnable(textarget);

	if(! toggle['y'])
	{
		glActiveTextureARB( GL_TEXTURE1_ARB);
		glBindTexture(textarget, src);
		glEnable(textarget);
		
		glColor3f(blend_factor, blend_factor, blend_factor);
		
		drawPlane(texw, texh);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	if(! toggle['u'])
	{
		glActiveTextureARB( GL_TEXTURE1_ARB);
		glBindTexture(textarget, recursive);
		glEnable(textarget);
		
		glColor3f(1-blend_factor, 1-blend_factor, 1-blend_factor);
		
		drawPlane(texw, texh);
		
	}
	glDisable(GL_BLEND);

    glDisable(GL_TEXTURE_SHADER_NV);
    glActiveTextureARB( GL_TEXTURE0_ARB);
    glDisable(textarget);
    glActiveTextureARB( GL_TEXTURE1_ARB);
    glDisable(textarget);

    // seed
    if (toggle['S']) {
      switch(seedmode) {
      case 1:
        seed(100);
        break;
      case 2:
        seed_smoke(100);
        break;
      case 3:
        if (tex) seed_img(tex, 1000);
        break;
      case 4:
        seed_streamers(16);
        break;
      }
    }

    if (toggle['f']) {
      // feedback
      glActiveTextureARB(GL_TEXTURE1_ARB);
	  glBindTexture(textarget, recursive);
      glCopyTexSubImage2D(textarget, 0, 0, 0, 0, 0, min(texw, winw), min(texh, winh)); 
    }
}

// GLUT callback functions
void display(void)
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// view transform
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(tx, ty, 0.0);
    glTranslatef(winw/2.0, winh/2.0, 0.0);
	glRotatef(rotz, 0.0, 0.0, 1.0);
    glScalef(scale, scale, 1.0);
    glTranslatef(-winw/2.0, -winh/2.0, 0.0);

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_2D_MATRIX_NV, offsetMatrix);
	glActiveTextureARB( GL_TEXTURE0_ARB );

    if (toggle['s']) {
      fluid->simulate();
    }

    switch(display_mode) {
    case 0:
      if (toggle['a']) 
        UpdateOffsetTexture(offset_tex, fluid);
      drawFeedback();
      break;
    case 1:
      glColor3f(1.0, 1.0, 1.0);
      drawVectorField(fluid, 100.0);
      break;

    case 2:
      drawVectorField2(fluid, 10.0);
      break;

    case 3:
      if (toggle['a']) 
        viz->step(10.0);
      glColor3f(1.0, 1.0, 1.0);
      glPointSize(1.0);
      glPushMatrix();
      glScalef(gridsize, gridsize, 1.0);
      viz->render();
      glPopMatrix();
      break;
    }

	glPopMatrix();

	checkErrors("display");
	glutSwapBuffers();
}

void motion(int x, int y)
{
	float dx, dy;
	dx = x - ox;
	dy = y - oy;

	if (buttonState == 3) {
		// left+middle = zoom
		scale += dy / 100.0;
	} 
	else if (buttonState & 2) {
		// middle = translate
		tx += dx;
		ty -= dy;
	}
	else if (buttonState & 1) {
      // apply force
      int mx = (int)(x/gridsize);
      int my = (int)((winh-y)/gridsize);
      static int omx = mx, omy = my;

#if 0
      fluid->setForce(mx, my, (mx-omx)*force, (my-omy)*force);
#else
      if ((mx > 0) && (mx < fluid->getSize()) && (my > 0) && (my < fluid->getSize())) {
        for(int i=-1; i<=1; i++) {
          for(int j=-1; j<=1; j++) {
            fluid->setForce(mx+i, my+j, (mx-omx)*force, (my-omy)*force);
          }
        }
      }
#endif
      omx = mx; omy = my;
	}

	ox = x; oy = y;
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	int mods;

	if (state == GLUT_DOWN)
		buttonState |= 1<<button;
	else if (state == GLUT_UP)
		buttonState = 0;

	mods = glutGetModifiers();
	if (mods & GLUT_ACTIVE_SHIFT)
		buttonState |= 2;

	ox = x; oy = y;
	glutPostRedisplay();
}


void key(unsigned char key, int x, int y)
{
  toggle[key] ^= 1;

  switch (key) {
#ifndef BROWSER
	case '\033':
	case 'q':
		exit(0);
		break;
#endif
    case '=':
      amp += 0.001;
      offsetMatrix[0] = offsetMatrix[3] = amp;
      break;
    case '-':
      amp -= 0.001;
      offsetMatrix[0] = offsetMatrix[3] = amp;
      break;
    case '+':
      amp += 0.01;
      offsetMatrix[0] = offsetMatrix[3] = amp;
      break;
    case '_':
      amp -= 0.01;
      offsetMatrix[0] = offsetMatrix[3] = amp;
      break;

    case 'r':
      fluid->randomize(0.1);
      viz->init(128, 128, 1);
      tx = ty = 0.0;
      scale = 1.0;
      amp = 0.003;
      offsetMatrix[0] = offsetMatrix[3] = amp;
      break;

    case 'v':
      display_mode = (display_mode + 1) % 4;
      break;

    case 'z':
      fluid->randomize();
      break;

    case ']':
      pointsize += 1.0;
      break;
    case '[':
      if (pointsize > 1.0) pointsize -= 1.0;
      break;

    case ')':
    case '0':
		blend_factor += .01;
		break;
	case '(':
    case '9':
		blend_factor -= .01;
		break;

	if(blend_factor > 1)
		blend_factor = 1;
	if(blend_factor < 0)
		blend_factor = 0;

    //case '0':
    case '1':
    case '2':
    case '3':
    case '4':
      seedmode = key - '0';
      break;

    case 'i':
      glDeleteTextures(1, &imgtex);
      imgtex = CreateImageTexture(texw, texh, 1);
      initTexShaders();
      break;

    case ' ':
      fluid->simulate();
      viz->step(10.0);
      break;
    }

	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	winw = w;
	winh = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glOrtho(0, winw, 0, winh, -1.0, 1.0);


	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}


void seedMenu(int i)
{
  toggle['s'] = true;
  seedmode = i;
}

void mainMenu(int i)
{
	key((unsigned char) i, 0, 0);
}

void initMenus(void)
{
	glutCreateMenu(mainMenu);
	glutAddMenuEntry("Seed -> Random points [1]", '1');
	glutAddMenuEntry("Seed -> Smoke [2]", '2');
    glutAddMenuEntry("Seed -> Image [3]", '3');
    glutAddMenuEntry("Seed -> Streamers [4]", '4');
    glutAddMenuEntry("Switch visualization [v]", 'v');
	glutAddMenuEntry("Toggle simulation [s]", 's');
	glutAddMenuEntry("Step simulation [ ]", ' ');
	glutAddMenuEntry("Toggle animation [a]", 'a');
	glutAddMenuEntry("Reset simulation [r]", 'r');
	glutAddMenuEntry("Randomize velocities [z]", 'z');
	glutAddMenuEntry("Toggle seed [S]", 'S');
	glutAddMenuEntry("Toggle feedback [f]", 'f');
	glutAddMenuEntry("Initialize image texture [i]", 'i');
	glutAddMenuEntry("Increment offset [=]", '=');
	glutAddMenuEntry("Decrement offset [-]", '-');
	glutAddMenuEntry("Increment blend amount [)]", ')');
	glutAddMenuEntry("Decrement blend amount [(]", '(');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void checkShaderConsistency(GLuint unit)
{
  GLint consistent;

  glActiveTextureARB(GL_TEXTURE0_ARB + unit);
  glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, &consistent);
  if(consistent == GL_FALSE)
	  printf("Texture shader stage %d is inconsistent!\n", (int)unit);
}

void initTexShaders()
{
    char ts[200];

    sprintf(ts, "!!TS1.0\n"
                "texture_2d();\n"
                "offset_2d(tex0, %f, %f, %f, %f);\n"
                "nop();\n"
                "nop();\n",
                offsetMatrix[0], offsetMatrix[1], offsetMatrix[2], offsetMatrix[3]);

    
    nvparse(ts);

  // stage 0 -- texture
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture(textarget, offset_tex);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);

  // stage 1 -- dependent texture
  glActiveTextureARB( GL_TEXTURE1_ARB );
  glBindTexture(textarget, recursive);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  checkShaderConsistency(0);
  checkShaderConsistency(1);
  checkShaderConsistency(2);
  checkShaderConsistency(3);
}


GLuint CreateImageTexture(int w, int h, int mode)
{
  GLuint texid;
  GLubyte *img, *ptr;
  int x, y;

  glGenTextures(1, &texid);
  glBindTexture(textarget, texid);

  glTexParameteri(textarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  img = (GLubyte *) malloc(w * h * 4);
  if (!img) return 0;

  ptr = img;
  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      GLubyte b = ((x & 16) ^ (y & 16)) * 255;
      switch(mode) {
      case 0:
        // random pixels
        *ptr++ = (rand()>>4) & 255;
        *ptr++ = (rand()>>4) & 255;
        *ptr++ = (rand()>>4) & 255;
        *ptr++ = (rand()>>4) & 255;
        break;
      case 1:
        // checkerboard
        *ptr++ = b;
        *ptr++ = b;
        *ptr++ = b;
        *ptr++ = 255;
        break;
      case 2:
        // coloured checkerboard
        *ptr++ = (x * 8) & 255;
        *ptr++ = (y * 8) & 255;
        *ptr++ = 0;
        *ptr++ = 255;
        break;
      }
    }
  }

  glTexImage2D(textarget, 0, GL_RGBA,
		       w, h, 0,
		       GL_RGBA, GL_UNSIGNED_BYTE,
		       img);

  free(img);

  return texid;
}


GLuint CreateOffsetTexture()
{
  GLuint texid;

  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexParameteri(textarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(textarget, 0, GL_DSDT_NV, 128, 128, 0, GL_DSDT_NV, GL_FLOAT, NULL);
  
  return texid;
}


void UpdateOffsetTexture(GLuint texid, StableFluid *f)
{
  static GLfloat *img;

  if (!img) {
    img = (GLfloat *) malloc(f->getSize() * f->getSize() * sizeof(GLfloat) * 2);
    if (!img) return;
  }

  float min = 1e10;
  float max = -min;
  GLfloat *ptr = img;
  for(int y=0; y<f->getSize(); y++) {
    for(int x=0; x<f->getSize(); x++) {
      float u, v;
      f->getVel(x, y, u, v);
      if (u > max) max = u;
      if (u < min) min = u;
      *ptr++ = u * offset_scale;
      *ptr++ = v * offset_scale;
    }
  }

  glBindTexture(GL_TEXTURE_2D, texid);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(textarget, 0, 0, 0, f->getSize(), f->getSize(), GL_DSDT_NV, GL_FLOAT, img);
}

GLuint CreateFileTexture()
{
  GLuint texture;
  string filename;
  
  data_path media;

  media.path.push_back(".");
  media.path.push_back("../../../MEDIA/textures/2D");
  media.path.push_back("../../../../MEDIA/textures/2D");
  media.path.push_back("../../../../../../../MEDIA/textures/2D");

  filename = media.get_file("nvlogo_flip.ppm");
  if (filename != "")
    tex = LoadTexturePPM((char *)filename.c_str());


  glGenTextures(1, &texture);
  glBindTexture(textarget, texture);

  glTexParameteri(textarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(textarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if(tex)
	  glTexImage2D(textarget, 0, GL_RGBA, tex->w, tex->h, 0,
				   GL_RGB, GL_UNSIGNED_BYTE, tex->image);
  return texture;
}

void initGL()
{
    GLubyte *img;

    glDisable(GL_DITHER);

	checker = CreateImageTexture(texw, texh, 1);
	imgtex = CreateFileTexture();

    img = (GLubyte *) malloc(texw * texh * 4);
    if (!img)
	    return;

    glGenTextures(1, &recursive);
    glBindTexture(textarget, recursive);

    glTexParameteri(textarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(textarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(textarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(textarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(textarget, 0, GL_RGBA,
	             texw, texh, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE,
	             img);
    free(img);

    offset_tex = CreateOffsetTexture();

	if(tex)
		src = imgtex;
	else
		src = checker;

    initTexShaders();

    checkErrors("initGL");
}

void dumpDriverInfo(void)
{
  GLint i;
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));
  printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &i);
  printf("Max 3D texture size = %d\n", (int)i);
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(winw, winh);
	(void) glutCreateWindow("superflow");

	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);

	getExts();
	initGL();
	initMenus();
	dumpDriverInfo();

    fluid = new StableFluid(128);
    fluid->randomize();

    viz = new ParticleViz(fluid, 128*128);
    viz->init(128, 128, 1);

    toggle['s'] = true;
    toggle['a'] = true;
    toggle['f'] = true;
    
    key('r', 0, 0);

	glutMainLoop();
	return 0;
}
