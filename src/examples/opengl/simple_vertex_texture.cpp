/*
	simple vertex texture demo
	sgreen 1/2004

	this example demonstrates how to perform texture lookups
	in the vertex shader using NV_vertex_program3
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <GL/glu.h>

#include <shared/array_texture.h>
#include <shared/data_path.h>
#include <nv_png.h>
#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor object;
GLuint vprog_id, vprog_bilinear_id, fprog_id;
GLuint color_tex, displace_tex;
int tex_width, tex_height;

float scale = 0.8;
float rot = 0.0;
float displace = 0.3;
int lod = 0;

int model = 0;
int tesselation = 500;

// vertex program for simple displacement mapping
const char *vprog_code = 
"!!ARBvp1.0\n"
"OPTION NV_vertex_program3;\n"
"PARAM mvp[4] = { state.matrix.mvp };\n"
"PARAM texmat[4] = { state.matrix.texture };\n"
"PARAM scale = program.local[0];\n"
"PARAM lod = program.local[1];\n"
"TEMP pos, displace, texcoord;\n"
// transform texcoord by texture matrix
"DP4 texcoord.x, texmat[0], vertex.texcoord;\n"
"DP4 texcoord.y, texmat[1], vertex.texcoord;\n"
// ***************************************************
// vertex texture lookup
"MOV texcoord.w, lod.x;\n"
"TXL displace, texcoord, texture[1], 2D;\n"		    
// ***************************************************
// try and do as much work here as possible that isn't
// dependent on the texture lookup, to cover latency
"MOV result.texcoord[0], texcoord;\n"
"MAD result.color, displace.x, 0.5, 0.5;\n"
"MOV pos.w, 1.0;\n"
// ***************************************************
// scale vertex along normal
"MUL displace.x, displace.x, scale;\n"
"MAD pos.xyz, vertex.normal, displace.x, vertex.position;\n"	
// transform position to clip space
"DP4 result.position.x, mvp[0], pos;\n"
"DP4 result.position.y, mvp[1], pos;\n"
"DP4 result.position.z, mvp[2], pos;\n"
"DP4 result.position.w, mvp[3], pos;\n"
"END\n";

// version with bilinear filtering
const char *vprog_bilinear_code = 
"!!ARBvp1.0\n"
"OPTION NV_vertex_program3;\n"
"PARAM mvp[4] = { state.matrix.mvp };\n"
"PARAM texmat[4] = { state.matrix.texture };\n"
"PARAM scale = program.local[0];\n"
"PARAM lod = program.local[1];\n"
"PARAM texturesize = program.local[2];\n"
"PARAM texelsize = program.local[3];\n"
"TEMP pos, texcoord, texcoord2;\n"
"TEMP frac, tex0, tex1, tex2;\n"
// transform texcoord by texture matrix
"DP4 texcoord.x, texmat[0], vertex.texcoord;\n"
"DP4 texcoord.y, texmat[1], vertex.texcoord;\n"
"MOV result.texcoord[0], texcoord;\n"
// ***************************************************
// vertex texture lookup with bilinear filtering
"MUL texcoord2.xy, texcoord, texturesize;\n"            // scale up to integer coords
"FRC frac.xy, texcoord2;\n"                             // get fractional position within texel
"ADD texcoord.z, texcoord.x, texelsize.x;\n"
"MOV texcoord.w, lod.x;\n"
"TXL tex0, texcoord.xyyw, texture[1], 2D;\n"
"TXL tex1, texcoord.zyyw, texture[1], 2D;\n"
"ADD tex1, tex1, -tex0;\n"                              // lerp(frac.x, tex0, tex1) - note no LRP instruction in VP!
"MAD tex2, tex1, frac.x, tex0;\n"
"ADD texcoord.y, texcoord.y, texelsize.y;\n"
"TXL tex0, texcoord.xyyw, texture[1], 2D;\n"
"TXL tex1, texcoord.zyyw, texture[1], 2D;\n"
"ADD tex1, tex1, -tex0;\n"                              // lerp(frac.x, tex0, tex1)
"MAD tex0, tex1, frac.x, tex0;\n"
"ADD tex0, tex0, -tex2;\n"                              // lerp (frac.y, tex2, tex0)
"MAD tex0, tex0, frac.y, tex2;\n"
// ***************************************************
"MAD result.color, tex0.x, 0.5, 0.5;\n"
"MOV pos.w, 1.0;\n"
// ***************************************************
// scale vertex along normal
"MUL tex0.x, tex0.x, scale;\n"
"MAD pos.xyz, vertex.normal, tex0.x, vertex.position;\n"	
// transform position to clip space
"DP4 result.position.x, mvp[0], pos;\n"
"DP4 result.position.y, mvp[1], pos;\n"
"DP4 result.position.z, mvp[2], pos;\n"
"DP4 result.position.w, mvp[3], pos;\n"
"END\n";


// fragment program
const char *fprog_code =
"!!ARBfp1.0\n"
"TEMP tex0;\n"
// modulate texture by fragment color
"TEX tex0, fragment.texcoord, texture[0], 2D;\n"
"MUL result.color, tex0, fragment.color;\n"
"END\n";


GLuint load_program(GLenum program_type, const char *code);
GLuint load_rgb_texture(char *filename, GLenum target, bool mipmap, int &width, int &height);
GLuint load_greyscale_texture(char *filename, GLenum target, GLenum format, bool mipmap, int &width, int &height);
void draw_grid(int rows, int cols,
               float sx, float sy, float sz,
               float ux, float uy, float uz,
               float vx, float vy, float vz);

void init_opengl()
{
    if (!glh_init_extensions(
        "GL_ARB_vertex_program "
        "GL_ARB_multitexture "
        "GL_NV_vertex_program3"
		))
    {
        printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
                glh_get_unsupported_extensions());
        quitapp(-1);
    }

	printf("%s\n", glGetString(GL_EXTENSIONS));
	GLint vtex_units;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &vtex_units);
	printf("Vertex texture units: %d\n", vtex_units);

	// load programs
	vprog_id = load_program(GL_VERTEX_PROGRAM_ARB, vprog_code);
	vprog_bilinear_id = load_program(GL_VERTEX_PROGRAM_ARB, vprog_bilinear_code);
	fprog_id = load_program(GL_FRAGMENT_PROGRAM_ARB, fprog_code);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// load textures
	GLenum target = GL_TEXTURE_2D;
	color_tex = load_rgb_texture("nveye.png", target, true, tex_width, tex_height);
//	displace_tex = load_greyscale_texture("nveye_displace.png", target, GL_LUMINANCE_FLOAT32_ATI, true, tex_width, tex_height);
	displace_tex = load_greyscale_texture("nveye_displace.png", target, GL_RGBA_FLOAT32_ATI, true, tex_width, tex_height);

    // set texture size parameters for bilinear filtering
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 2, tex_width, tex_height, 0, 0);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 3, 1.0 / tex_width, 1.0 / tex_height, 0, 0);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(target, color_tex);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(target, displace_tex);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // create geometry
    glNewList(1, GL_COMPILE);
    glNormal3f(0.0, 0.0, 1.0);
    draw_grid(tesselation, tesselation, -1.0, -1.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    glEndList();

    glNewList(2, GL_COMPILE);
    GLUquadricObj *qobj = gluNewQuadric();
    // note to mjk - why doesn't glutSolidSphere have texture coordinates?!
    gluQuadricTexture(qobj, GL_TRUE);
    gluQuadricNormals(qobj, GLU_SMOOTH);
    gluSphere(qobj, 0.5, tesselation*2, tesselation);
    gluDeleteQuadric(qobj);
    glEndList();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    glutReportErrors();
}

// load textures from PNG files
GLuint load_rgb_texture(char *filename, GLenum target, bool mipmap, int &width, int &height)
{
    GLuint id;
    array2<vec3ub> img;
    read_png_rgb(filename, img);
    glGenTextures(1, &id);
    glBindTexture(target, id);
    make_rgb_texture(img, mipmap);
    width = img.get_width();
    height = img.get_height();
    return id;
}

GLuint load_greyscale_texture(char *filename, GLenum target, GLenum format, bool mipmap, int &width, int &height)
{
    GLuint id;
    array2<unsigned char> img;
    read_png_grey(filename, img);
    glGenTextures(1, &id);
    glBindTexture(target, id);
    make_scalar_texture(img, format, GL_LUMINANCE, mipmap);
    width = img.get_width();
    height = img.get_height();
    return id;
}

// load a vertex or fragment program from a string
GLuint load_program(GLenum program_type, const char *code)
{
	GLuint program_id;
	glGenProgramsARB(1, &program_id);
	glBindProgramARB(program_type, program_id);
	glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);

	GLint error_pos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
	if (error_pos != -1) {
		const GLubyte *error_string;
		error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		fprintf(stderr, "Program error at position: %d\n%s\n", error_pos, error_string);
	}
	return program_id;
}

// draw a subdivided quad mesh
void draw_grid(int rows, int cols,
               float sx, float sy, float sz,
               float ux, float uy, float uz,
               float vx, float vy, float vz)
{    
  for(int y=0; y<rows; y++) {
    glBegin(GL_TRIANGLE_STRIP);
    for(int x=0; x<=cols; x++) {
      float u = x / (float) cols;
      float v = y / (float) rows;
      float v2 = (y + 1) / (float) rows;
      glTexCoord2f(u, v);
      glVertex3f(sx + (u*ux) + (v*vx), sy + (u*uy) + (v*vy), sz + (u*uz) + (v*vz));
      glTexCoord2f(u, v2);
      glVertex3f(sx + (u*ux) + (v2*vx), sy + (u*uy) + (v2*vy), sz + (u*uz) + (v2*vz));
    }
    glEnd();
  }    
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.apply_transform();

	// rotate texture around center
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0.5, 0.5, 0.0);
	glScalef(scale, scale, scale);
	glRotatef(rot, 0.0, 0.0, 1.0);
	glTranslatef(-0.5, -0.5, 0.0);
	glMatrixMode(GL_MODELVIEW);

    if (b['b'])
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vprog_bilinear_id);
    else
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vprog_id);

	if (b['v'])
		glEnable(GL_VERTEX_PROGRAM_ARB);
	else
		glDisable(GL_VERTEX_PROGRAM_ARB);

    glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, displace, 0, 0, 0);
    glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, lod, 0, 0, 0);

    // set texture size parameters for bilinear filtering
    int mip_width = tex_width;
    int mip_height = tex_height;
    for(int i=0; i<lod; i++) {
        mip_width >>= 1;
        mip_height >>= 1;
    }
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 2, mip_width, mip_height, 0, 0);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 3, 1.0/mip_width, 1.0/mip_height, 0, 0);

	glPolygonMode(GL_FRONT_AND_BACK, b['w'] ? GL_LINE : GL_FILL);

    // draw object
    glColor3f(1.0, 1.0, 1.0);
	glCallList(1 + model);

	glutSwapBuffers();
    glutReportErrors();
}

void idle()
{
	if (b[' ']) {
        object.trackball.increment_rotation();
		rot += 1.0;
	}
    
  glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	switch(k) {
	case 27:
	case 'q':
		exit(0);
		break;
	case '=':
	case '+':
		displace += 0.1;
		break;
	case '-':
		displace -= 0.1;
		break;
	case ']':
		scale += 0.1;
		break;
	case '[':
		scale -= 0.1;
		break;
    case '.':
        lod ++;
        printf("lod: %d\n", lod);
        break;
    case ',':
        if (lod > 0) lod--;
        printf("lod: %d\n", lod);
        break;
	case 'o':
		model = (model + 1) % 2;
		break;
	}

    object.keyboard(k, x, y);
    
	glutPostRedisplay();
}

void resize(int w, int h)
{
  if (h == 0) h = 1;

  glViewport(0, 0, w, h);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  object.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
  object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
  object.motion(x, y);
}

void main_menu(int i)
{
  key((unsigned char) i, 0, 0);
}

void init_menus()
{
  glutCreateMenu(main_menu);
  glutAddMenuEntry("Toggle animation [ ]", ' ');
  glutAddMenuEntry("Toggle vertex program [v]", 'v');
  glutAddMenuEntry("Toggle bilinear filtering [b]", 'b');
  glutAddMenuEntry("Switch object [o]", 'o');
  glutAddMenuEntry("Increase displacement [+]", '+');
  glutAddMenuEntry("Decrease displacement [-]", '-');
  glutAddMenuEntry("Increase scale []]", ']');
  glutAddMenuEntry("Decrease scale [[]", '[');
  glutAddMenuEntry("Increase lod [.]", '.');
  glutAddMenuEntry("Decrease lod [,]", ',');
  glutAddMenuEntry("Quit (esc)", '\033');
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("simple_vertex_texture");

    if (argc > 1) {
        tesselation = atoi(argv[1]);
    }

    init_opengl();
    init_menus();

    object.configure_buttons(1);
    object.dolly.dolly[2] = -2;
    object.trackball.r = rotationf(vec3f(1.0, 0.0, 0.0), -GLH_PI/4.0);

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    b[' '] = true;
    b['v'] = true;

    glutMainLoop();

    return 0;
}
