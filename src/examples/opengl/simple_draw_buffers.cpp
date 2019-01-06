/*
	simple draw buffers example

	this example demonstrates using the ATI_draw_buffers extension
	to allow writing to multiple draw buffers from a fragment program
	in a single pass.

	this is equivalent to Direct3D's multiple render targets.

	sgreen 1/2004
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <shared/read_text_file.h>
#include <shared/RenderTexture.h>
#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor camera;
GLuint vprog_id, fprog_id;

// destination color buffers
const GLenum buffers[] = {
//	GL_FRONT_LEFT,
	GL_AUX0,
	GL_AUX1,
	GL_AUX2,
	GL_AUX3
};

const int wgl_buffers[] = {
//    WGL_FRONT_LEFT_ARB,
    WGL_AUX0_ARB,
    WGL_AUX1_ARB,
    WGL_AUX2_ARB,
    WGL_AUX3_ARB
};

// clear colors
const float cc[4][4] = {
	1.0, 0.0, 0.0, 1.0,
	0.0, 1.0, 0.0, 1.0,
	0.0, 0.0, 1.0, 1.0,
	1.0, 1.0, 0.0, 1.0,
};

int show_buffer = 0;
RenderTexture *pbuffer;
int win_w = 512, win_h = 512;

// fragment program
const char *mod_fprog_code =
"!!ARBfp1.0\n"
"TEMP tex0, tex1;\n"
"TEX tex0, fragment.texcoord, texture[0], RECT;\n"
"TEX tex1, fragment.texcoord, texture[1], RECT;\n"
"MUL result.color, tex0, tex1;\n"
//"MOV result.color, tex1;\n"
"END\n";

GLuint mod_fprog;

GLuint load_program(GLenum program_type, const char *code);
void key(unsigned char k, int x, int y);
void init_menu();
void main_menu(int i);
void resize(int w, int h);

void init_opengl()
{
    if (!glh_init_extensions(
		"GL_ARB_vertex_program "
		"GL_ARB_fragment_program "
		"GL_ARB_multitexture "
		"GL_ATI_draw_buffers "
        "WGL_ARB_pbuffer "
        "WGL_ARB_pixel_format "
        "WGL_ARB_render_texture "
		))
    {
        printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
               glh_get_unsupported_extensions());
        quitapp(-1);
    }

    pbuffer = new RenderTexture("rgba depth aux=4 textureRECT", win_w, win_h, GL_TEXTURE_RECTANGLE_NV);
    pbuffer->Activate();

    resize(win_w, win_h);

	// query buffer limits
	GLint max_buffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ATI, &max_buffers);
	printf("max draw buffers = %d\n", max_buffers);

	GLint aux_buffers;
	glGetIntegerv(GL_AUX_BUFFERS, &aux_buffers);
	printf("aux buffers = %d\n", aux_buffers);	

	// load programs
	char *vprog_code = read_text_file("simple_draw_buffers/simple_draw_buffers.vp");
	if (!vprog_code) quitapp(-1);
	char *fprog_code = read_text_file("simple_draw_buffers/simple_draw_buffers.fp");
	if (!fprog_code) quitapp(-1);
	vprog_id = load_program(GL_VERTEX_PROGRAM_ARB, vprog_code);
	fprog_id = load_program(GL_FRAGMENT_PROGRAM_ARB, fprog_code);

    mod_fprog = load_program(GL_FRAGMENT_PROGRAM_ARB, mod_fprog_code);

	// create geometry
	glNewList(1, GL_COMPILE);
	glutSolidTeapot(1.0);
	glEndList();

    glEnable(GL_DEPTH_TEST);
	glDrawBuffer(GL_FRONT);

	glutReportErrors();
}

void main_menu(int i)
{
  key((unsigned char) i, 0, 0);
}

void init_menu()
{
  glutCreateMenu(main_menu);
  glutAddMenuEntry("Display all buffers", '0');
  glutAddMenuEntry("Display buffer 1", '1');
  glutAddMenuEntry("Display buffer 2", '2');
  glutAddMenuEntry("Display buffer 3", '3');
  glutAddMenuEntry("Display buffer 4", '4');
  glutAddMenuEntry("Quit (esc)", '\033');
  glutAttachMenu(GLUT_RIGHT_BUTTON);
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
		fprintf(stderr, "%s program error at position: %d\n%s\n", (program_type == GL_VERTEX_PROGRAM_ARB) ? "vertex" : "fragment",
			error_pos, error_string);
	}
	return program_id;
}

void draw_quad()
{
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0); glVertex2f(-1, -1);
    glTexCoord2f(win_w, 0); glMultiTexCoord2fARB(GL_TEXTURE1_ARB, win_w, 0); glVertex2f(1, -1);
    glTexCoord2f(win_w, win_h); glMultiTexCoord2fARB(GL_TEXTURE1_ARB, win_w, win_h); glVertex2f(1, 1);
    glTexCoord2f(0, win_h); glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, win_h); glVertex2f(-1, 1);
    glEnd();
}

void display()
{
    // render to pbuffer
    pbuffer->Activate();

	// clear all buffers
	for(int i=0; i<4; i++) {
        glDrawBuffer(buffers[i]);
        glClearColor(cc[i][0], cc[i][1], cc[i][2], cc[i][3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	camera.apply_transform();

	glPolygonMode(GL_FRONT_AND_BACK, b['w'] ? GL_LINE : GL_FILL);

    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fprog_id);
    if (b['f'])
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	// set destination buffers
	glDrawBuffersATI(sizeof(buffers) / sizeof(GLenum), buffers);

	// draw object
    glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	glCallList(1);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_VERTEX_PROGRAM_ARB);
    glFinish();

	// render to window
    pbuffer->Deactivate();
	glDisable(GL_DEPTH_TEST);
	glDrawBuffer(GL_BACK);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

    // display aux buffers by binding as textures
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    if (show_buffer==0) {
        // draw all 4 buffers
        glScalef(0.5, 0.5, 0.5);

        glTranslatef(-1, -1, 0);
        pbuffer->Bind(wgl_buffers[0]);
        draw_quad();
        pbuffer->Release(wgl_buffers[0]);

        glTranslatef(2, 0, 0);
        pbuffer->Bind(wgl_buffers[1]);
        draw_quad();
        pbuffer->Release(wgl_buffers[1]);

        glTranslatef(0, 2, 0);
        pbuffer->Bind(wgl_buffers[2]);
        draw_quad();
        pbuffer->Release(wgl_buffers[2]);

        glTranslatef(-2, 0, 0);
        pbuffer->Bind(wgl_buffers[3]);
        draw_quad();
        pbuffer->Release(wgl_buffers[3]);

    } else if (show_buffer == 5) {
        // blend two buffers together
        glActiveTextureARB(GL_TEXTURE0_ARB);
        pbuffer->Bind(wgl_buffers[0]);

        glActiveTextureARB(GL_TEXTURE1_ARB);
        pbuffer->Bind(wgl_buffers[1]);

        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, mod_fprog);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);

        draw_quad();

        glDisable(GL_FRAGMENT_PROGRAM_ARB);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        pbuffer->Release(wgl_buffers[0]);

        glActiveTextureARB(GL_TEXTURE1_ARB);
        pbuffer->Release(wgl_buffers[1]);

    } else {
        // show one buffer
        pbuffer->Bind(wgl_buffers[show_buffer-1]);
        draw_quad();
        pbuffer->Release(wgl_buffers[show_buffer-1]);
    }
    glDisable(GL_TEXTURE_RECTANGLE_NV);

	glPopMatrix();
	glutSwapBuffers();

	glutReportErrors();
}

void idle()
{
	if (b[' ']) {
        camera.trackball.increment_rotation();
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
    case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		show_buffer = k - '0';
		printf("showing buffer %d\n", show_buffer);
		break;
	}

	camera.keyboard(k, x, y);
    
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

    camera.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
	camera.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    camera.motion(x, y);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("simple draw buffers");

	init_opengl();
    init_menu();

    camera.configure_buttons(1);
    camera.dolly.dolly[2] = -4;
	camera.trackball.incr = rotationf(vec3f(0.5, 0.7, 0.3), 0.1);
	glut_add_interactor(&camera);
	camera.enable();

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    b[' '] = true;
	b['f'] = true;

    glutMainLoop();

	return 0;
}
