/* 
 * Image space motion blur demo
 * http://developer.nvidia.com/docs/IO/8230/GDC2003_OpenGLShaderTricks.pdf
 *
 * (c) NVidia Corporation 2003
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Cg/cgGL.h>
#include <iostream>

#if defined(MACOS)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>

#include <shared/array_texture.h>
#include <nv_png.h>
#include <glh/glh_linear.h>

#include <shared/NvIOModel.h>
#include <shared/quitapp.h>

#if !defined(GL_TEXTURE_RECTANGLE_NV) && defined(GL_EXT_texture_rectangle)
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#endif

using namespace std;
using namespace glh;

int win_w = 1024, win_h = 768;

int ox, oy;
int buttonState = 0;

float tx = 0, ty = 0, tz = -3;
float rx = 30, ry = 0, rz = 0;

int keydown[256];
bool toggle[256];

NvIOModel model;
bool loaded_model = false;

GLuint color_map;
GLuint sceneTex = 0;

CGcontext context;
CGprogram mblur_fprog, mblur_vprog;
CGprofile vertexProfile, fragmentProfile;

CGparameter modelViewProj_param, prevModelViewProj_param;
CGparameter modelView_param, prevModelView_param;
CGparameter halfWindowSize_param, blurScale_param;
CGparameter sceneTex_param, fpBlurScale_param;
float blurScale = 1.0;

matrix4f projection, modelView, prevModelView;

void cleanExit(int);

void winCoordsBegin(void)
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);

  glMatrixMode(GL_MODELVIEW);
}

void winCoordsEnd(void)
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void drawTexturedQuad()
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex2i(0, 0);
  glTexCoord2f(1.0, 0.0); glVertex2i(win_w, 0);
  glTexCoord2f(1.0, 1.0); glVertex2i(win_w, win_h);
  glTexCoord2f(0.0, 1.0); glVertex2i(0, win_h);
  glEnd();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

void drawScene()
{
  glEnable(GL_DEPTH_TEST);

  if (toggle['w'])
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  if (loaded_model) {
    model.Render();
  } else {
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glutSolidTorus(0.5, 1.0, 30, 60);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void copyWindowToTexture(GLenum target, GLuint *tex)
{  
  if (*tex) {
    // copy to texture
    glBindTexture(target, *tex);
    glCopyTexSubImage2D(target, 0, 0, 0, 0, 0, win_w, win_h);
  } else {
    // create texture
    glGenTextures(1, tex);
    glBindTexture(target, *tex);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCopyTexImage2D(target, 0, GL_RGB, 0, 0, win_w, win_h, 0);
  }
}

// render scene to texture
void renderScene()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture(GL_TEXTURE_2D, color_map);
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  drawScene();
  glDisable(GL_TEXTURE_2D);

  copyWindowToTexture(GL_TEXTURE_RECTANGLE_NV, &sceneTex);
}

void renderBlurredScene()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  cgGLBindProgram(mblur_vprog);
  cgGLEnableProfile(vertexProfile);

  if (!toggle[' ']) {
    // update matrices
    cgGLSetStateMatrixParameter(modelView_param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetMatrixParameterfc(prevModelView_param, prevModelView.get_value() );

    cgGLSetStateMatrixParameter(modelViewProj_param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    matrix4f prevModelViewProjection = projection * prevModelView;
    cgGLSetMatrixParameterfc(prevModelViewProj_param, prevModelViewProjection.get_value() );
  }
 
  if (toggle['b']) {
    cgGLBindProgram(mblur_fprog);
    cgGLEnableProfile(fragmentProfile);
    cgGLSetTextureParameter(sceneTex_param, sceneTex);
    cgGLEnableTextureParameter(sceneTex_param);
    cgGLSetParameter1f(fpBlurScale_param, blurScale);
  }
  
  drawScene();

  cgGLDisableProfile(vertexProfile);
  cgGLDisableProfile(fragmentProfile);
}

void display()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(tx, ty, tz);
  glRotatef(rx, 1.0, 0.0, 0.0);
  glRotatef(ry, 0.0, 1.0, 0.0);
  glRotatef(rz, 0.0, 0.0, 1.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, (float *) modelView.get_value());

  if (!toggle[' ']) {
    renderScene();
  }

  renderBlurredScene();

  glutReportErrors();
  glutSwapBuffers();

  prevModelView = modelView;
}

void motion(int x, int y)
{
	float dx, dy;
	dx = x - ox;
	dy = y - oy;

	if (buttonState == 3) {
		// left+middle = zoom
		tz += (dx+dy) / 100.0;
	} 
	else if (buttonState == 2) {
		// middle = translate
		tx += dx / 100.0;
		ty -= dy / 100.0;
	}
	else if (buttonState == 1) {
		// left = rotate
		rx += dy / 5.0;
		ry += dx / 5.0;
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
		buttonState = 2;
	if (mods & GLUT_ACTIVE_CTRL)
		buttonState = 3;

	ox = x; oy = y;
	glutPostRedisplay();
}


void idle(void)
{
  if (toggle['a'] && !toggle[' ']) {
//    ry += 10.0;
    static double last_time;
    double time = glutGet(GLUT_ELAPSED_TIME);
    double dt = (time - last_time) / 1000.0;
    last_time = time;
    ry += dt * 200.0;
  }
  glutPostRedisplay();
}

void key(unsigned char key, int x, int y)
{
  toggle[key] ^= 1;

  switch (key) {
  case '\033':
  case 'q':
    cleanExit(0);
    break;
  case 'r':
    tx = 0.0; ty = 0.0; tz = -3.0;
    rx = ry = rz = 0.0;
    break;

  case '=':
  case '+':
    if (blurScale < 1.0)
      blurScale += 0.1;
    break;
  case '-':
  case '_':
    if (blurScale > 0.0)
      blurScale -= 0.1;
    break;

  default:
	keydown[key] = 1;
	break;
  }

  printf("%f\n", blurScale);
  glutPostRedisplay();
}


void reshape(int w, int h)
{
  win_w = w;
  win_h = h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (float) w / (float) h, 0.1, 10.0);

  glGetFloatv(GL_PROJECTION_MATRIX, (float *) projection.get_value());

  glMatrixMode(GL_MODELVIEW);
  glViewport(0, 0, w, h);

  cgGLSetParameter2f(halfWindowSize_param, win_w/2.0, win_h/2.0);
  sceneTex = 0;
}

void mainMenu(int i)
{
  key((unsigned char) i, 0, 0);
}

void initMenus(void)
{
  glutCreateMenu(mainMenu);
  glutAddMenuEntry("Toggle animation [a]", 'a');
  glutAddMenuEntry("Toggle pause [ ]", ' ');
  glutAddMenuEntry("Toggle blur [b]", 'b');
  glutAddMenuEntry("Toggle wireframe [w]", 'w');
  glutAddMenuEntry("Increase blur scale [+]", '+');
  glutAddMenuEntry("Decrease blur scale [-]", '-');
  glutAddMenuEntry("Quit (esc)", '\033');
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void cleanExit(int exitval)
{
  if (mblur_vprog) cgDestroyProgram(mblur_vprog);
  if (mblur_fprog) cgDestroyProgram(mblur_fprog);
  if (context) cgDestroyContext(context);

  if(exitval == 0) { exit(0); }
  else { quitapp(exitval); }
}

void cgErrorCallback(void)
{
  CGerror LastError = cgGetError();

  if(LastError) {
    const char *Listing = cgGetLastListing(context);
    printf("\n---------------------------------------------------\n");
    printf("%s\n\n", cgGetErrorString(LastError));
    printf("%s\n", Listing);
    printf("---------------------------------------------------\n");
    printf("Cg error, exiting...\n");
    cleanExit(-1);
  }
}

CGprogram loadCgProgram(char *filename, CGprofile profile, data_path &path)
{
  std::string pathname = path.get_file(filename);
  if (pathname == "") {
    printf("Unable to load '%s', exiting...\n", filename);
    quitapp(-1);
  }

  CGprogram program = cgCreateProgramFromFile(context, CG_SOURCE, pathname.data(),
                                              profile, NULL, NULL);
  cgGLLoadProgram(program);
  return program;
}

void initCg()
{
  if (cgGLIsProfileSupported(CG_PROFILE_ARBVP1))
    vertexProfile = CG_PROFILE_ARBVP1;
  else if (cgGLIsProfileSupported(CG_PROFILE_VP30))
    vertexProfile = CG_PROFILE_VP30;
  else
  {
    printf("Vertex programming extensions (GL_ARB_vertex_program or "
           "GL_NV_vertex_program2) not supported, exiting...\n");
    quitapp(-1);
  }

  if (cgGLIsProfileSupported(CG_PROFILE_FP30))
    fragmentProfile = CG_PROFILE_FP30;
  else if (cgGLIsProfileSupported(CG_PROFILE_ARBFP1))
    fragmentProfile = CG_PROFILE_ARBFP1;
  else
  {
    printf("Fragment programming extensions (GL_ARB_fragment_program or "
           "GL_NV_fragment_program) not supported, exiting...\n");
    quitapp(-1);
  }
	
  cgSetErrorCallback(cgErrorCallback);
  context = cgCreateContext();

  data_path path;
  path.path.push_back(".");
  path.path.push_back("../../../MEDIA/programs");
  path.path.push_back("../../../../MEDIA/programs");
  path.path.push_back("../../../../../../../MEDIA/programs");

  mblur_vprog = loadCgProgram("motion_blur/mblur_vp.cg", vertexProfile, path);
  cgGLLoadProgram(mblur_vprog);

  modelView_param         = cgGetNamedParameter(mblur_vprog, "modelView");
  modelViewProj_param     = cgGetNamedParameter(mblur_vprog, "modelViewProj");
  prevModelView_param     = cgGetNamedParameter(mblur_vprog, "prevModelView");
  prevModelViewProj_param = cgGetNamedParameter(mblur_vprog, "prevModelViewProj");

  halfWindowSize_param    = cgGetNamedParameter(mblur_vprog, "halfWindowSize");
  cgGLSetParameter2f(halfWindowSize_param, win_w/2.0, win_h/2.0);
  blurScale_param         = cgGetNamedParameter(mblur_vprog, "blurScale");
  cgGLSetParameter1f(blurScale_param, 1.0);

  mblur_fprog = loadCgProgram("motion_blur/mblur.cg", fragmentProfile, path);
  cgGLLoadProgram(mblur_fprog);

  sceneTex_param          = cgGetNamedParameter(mblur_fprog, "sceneTex"); 
  fpBlurScale_param       = cgGetNamedParameter(mblur_fprog, "blurScale"); 
  cgGLSetParameter1f(fpBlurScale_param, 1.0);

}

// get extension pointers
void getExts(void)
{
  if(! glh_init_extensions("GL_ARB_multitexture "))
  {
	 printf("Necessary extensions are not supported: %s\n", glh_get_unsupported_extensions());
     quitapp(0);
  }
}

void initGL()
{
  // initialize OpenGL lighting
  GLfloat matAmb[4] =    {1.0, 1.0, 1.0, 1.0};
  GLfloat matDiff[4] =   {1.0, 1.0, 1.0, 1.0};
  GLfloat matSpec[4] =   {1.0, 1.0, 1.0, 1.0};
  GLfloat matShine =     80.0;

  GLfloat lightPos[] =   {1.0, 1.0, 1.0, 0.0};
  GLfloat lightAmb[4] =	 {0.1, 0.1, 0.1, 1.0};
  GLfloat lightDiff[4] = {0.9, 0.9, 0.9, 1.0};
  GLfloat lightSpec[4] = {0.5, 0.5, 0.5, 1.0};

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, matShine);

  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
  GLfloat black[] =   {0.0, 0.0, 0.0, 1.0};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_DITHER);

  initCg();

  // load texture
  array2<vec3ub> img;
  glGenTextures(1, &color_map);
  glBindTexture(GL_TEXTURE_2D, color_map);
  data_path path;
  path.path.push_back(".");
  path.path.push_back("../../../MEDIA/textures/2D");
  path.path.push_back("../../../../MEDIA/textures/2D");
  path.path.push_back("../../../../../../../MEDIA/textures/2D");
  set_png_path(path);
  read_png_rgb("graydirt.png", img );
  make_rgb_texture(img, true);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  toggle['s'] = toggle['v'] = toggle['a'] = toggle['b'] = true;

  glutReportErrors();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(win_w, win_h);
  (void) glutCreateWindow("image space motion blur");

  glutDisplayFunc(display);
  glutKeyboardFunc(key);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIdleFunc(idle);

  if (argc > 1) {
    // load obj model
    loaded_model = model.ReadOBJ(argv[1]);
    model.Rescale();
  }

  getExts();
  initGL();
  initMenus();

  glutMainLoop();
  return 0;
}
