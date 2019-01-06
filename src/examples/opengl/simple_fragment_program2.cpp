/*
	simple NV_fragment_program2 example
	sgreen 1/2004
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <shared/read_text_file.h>
#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor camera;
glut_simple_mouse_interactor lights;
GLuint vprog_id, fprog_id;

GLuint light_pos_tex, light_color_tex;

const int max_lights = 8;
// light positions (w==0.0 for directional lights)
const float d = 2.0;
vec4f light_pos[max_lights] = {
    vec4f(-1.0, -1.0, -1.0, 0.0),  // directional
//    vec4f(-d, -d, -d, 1.0),
	vec4f( d, -d, -d, 1.0),
    vec4f( d,  d, -d, 1.0),
	vec4f(-d,  d, -d, 1.0),
	vec4f(-d, -d,  d, 1.0),
	vec4f( d, -d,  d, 1.0),
	vec4f( d,  d,  d, 1.0),
	vec4f(-d,  d,  d, 1.0),
};
vec4f light_pos_eye[max_lights];

// light colors (w component is attenuation)
const float att = 0.2;
vec4f light_color[max_lights] = {
	vec4f(0.5, 0.5, 0.5, att),
	vec4f(1.0, 0.0, 0.0, att),
	vec4f(0.0, 1.0, 0.0, att),
	vec4f(0.0, 0.0, 1.0, att),
	vec4f(1.0, 1.0, 0.0, att),
	vec4f(1.0, 0.0, 1.0, att),
	vec4f(0.0, 1.0, 1.0, att),
	vec4f(1.0, 0.5, 0.0, att),
};

int nlights = max_lights;

GLuint load_program(GLenum program_type, const char *code);
GLuint load_texture_array(GLenum format, float *data, int n);
void query_program_limits();

void init_opengl()
{
    if (!glh_init_extensions(
		"GL_ARB_vertex_program "
		"GL_ARB_fragment_program "
		"GL_ARB_multitexture "
        "GL_NV_fragment_program2 "
		))
    {
        printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
               glh_get_unsupported_extensions());
        quitapp(-1);
    }

//	printf("%s\n", glGetString(GL_EXTENSIONS));
    query_program_limits();

	// load programs
	char *vprog_code = read_text_file("simple_fragment_program2/simple_fragment_program2.vp");
	if (!vprog_code) quitapp(-1);
	char *fprog_code = read_text_file("simple_fragment_program2/simple_fragment_program2.fp");
	if (!fprog_code) quitapp(-1);
	vprog_id = load_program(GL_VERTEX_PROGRAM_ARB, vprog_code);
    if (!vprog_id) quitapp(-1);
	fprog_id = load_program(GL_FRAGMENT_PROGRAM_ARB, fprog_code);
    if (!fprog_id) quitapp(-1);
    glEnable(GL_VERTEX_PROGRAM_ARB);

	// load textures storing light positions and colors
	light_pos_tex = load_texture_array(GL_FLOAT_RGBA32_NV, (float *) &light_pos[0], nlights);
	light_color_tex = load_texture_array(GL_RGBA, (float *) &light_color[0], nlights);

	glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, light_pos_tex);

	glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, light_color_tex);

	// create geometry
	glNewList(1, GL_COMPILE);
	glutSolidTeapot(1.0);
//	glutSolidSphere(1.0, 40, 20);
//	glutSolidTorus(0.5, 1.0, 40, 40);
	glEndList();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

void query_program_limits()
{
    struct Query {
      GLenum glenum;
      char *name;
    } limits [] = {
      #define ENUMNAME(E) E, #E
      ENUMNAME(GL_MAX_PROGRAM_INSTRUCTIONS_ARB),
      ENUMNAME(GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB),
      ENUMNAME(GL_MAX_PROGRAM_TEMPORARIES_ARB),
      ENUMNAME(GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB),
      ENUMNAME(GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV),
      ENUMNAME(GL_MAX_PROGRAM_CALL_DEPTH_NV),
      ENUMNAME(GL_MAX_PROGRAM_IF_DEPTH_NV),
      ENUMNAME(GL_MAX_PROGRAM_LOOP_DEPTH_NV),
      ENUMNAME(GL_MAX_PROGRAM_LOOP_COUNT_NV),
    };
    for(int i=0; i<sizeof(limits)/sizeof(limits[0]); i++) {
      GLint value;
      glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, limits[i].glenum, &value);
      printf("%s = %d\n", limits[i].name, value);
    }
}

GLuint load_texture_array(GLenum format, float *data, int n)
{
	const GLenum target = GL_TEXTURE_RECTANGLE_NV;
	GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(target, 0, format, n, 1, 0, GL_RGBA, GL_FLOAT, data);
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
		fprintf(stderr, "%s program error at position: %d\n%s\n", (program_type == GL_VERTEX_PROGRAM_ARB) ? "vertex" : "fragment",
			error_pos, error_string);
        return 0;
	}

    GLint is_native;
    glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &is_native);
    if (is_native != 1) {
      fprintf(stderr, "Warning - program is not within native hardware limits\n");
    }

	return program_id;
}

void draw_lights()
{
	glPushMatrix();
	lights.apply_transform();

	glPointSize(5.0);
	for(int i=0; i<nlights; i++) {
      if (light_pos[i][3] == 0.0) {
        // draw directional lights as lines
        glColor3fv(&light_color[i][0]);
	    glBegin(GL_POINTS);
        vec4f p0 = light_pos[i] * 2.0f; glVertex3fv(&p0[0]);
	    glEnd();
        glBegin(GL_LINES);
        glVertex3fv(&p0[0]);
        vec4f p1 = light_pos[i] * 1.5f; glVertex3fv(&p1[0]);
	    glEnd();
      } else {
        // draw point lights as points
	    glBegin(GL_POINTS);
        glColor3fv(&light_color[i][0]);
	    glVertex3fv(&light_pos[i][0]);
	    glEnd();
      }
    }

	glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	camera.apply_transform();

	if (b['s'])
		draw_lights();

	glPolygonMode(GL_FRONT_AND_BACK, b['w'] ? GL_LINE : GL_FILL);

	if (b['f'])
		glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, nlights, 0, 0, 0);

	// transform light positions to eye space
	matrix4f modelview = camera.get_transform() * lights.get_transform();
	for(int i=0; i<max_lights; i++) {
		modelview.mult_matrix_vec(light_pos[i], light_pos_eye[i]);
        if (light_pos_eye[i][3] == 0.0) {
          // normalize direction
          light_pos_eye[i].normalize();
        }
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, light_pos_tex);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, max_lights, 1, GL_RGBA, GL_FLOAT, (float *) &light_pos_eye[0]);

	// draw object
    glColor3f(1.0, 1.0, 1.0);
	glCallList(1);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	glutSwapBuffers();
}

void idle()
{
	if (b[' ']) {
        camera.trackball.increment_rotation();
        lights.trackball.increment_rotation();
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
		if (nlights < max_lights) nlights++;
		printf("%d lights\n", nlights);
		break;
	case '-':
		if (nlights > 0) nlights--;
		printf("%d lights\n", nlights);
		break;
	case 'l':
		if (b['l']) {
			lights.enable();
			camera.disable();
		} else {
			lights.disable();
			camera.enable();
		}
	}

	if (b['l'])
		lights.keyboard(k, x, y);
	else
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
	lights.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
	if (b['l'])
		lights.mouse(button, state, x, y);
	else
		camera.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    camera.motion(x, y);
    lights.motion(x, y);
}

void main_menu(int i)
{
  key((unsigned char) i, 0, 0);
}

void init_menu()
{
  glutCreateMenu(main_menu);
  glutAddMenuEntry("Add light [+]", '+');
  glutAddMenuEntry("Remove light [-]", '-');
  glutAddMenuEntry("Toggle move light [l]", 'l');
  glutAddMenuEntry("Quit (esc)", '\033');
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("simple_fragment_program2");

	init_opengl();
    init_menu();

    camera.configure_buttons(2);
    camera.dolly.dolly[2] = -8;
	glut_add_interactor(&camera);
	camera.enable();

    lights.configure_buttons(2);
	lights.trackball.incr = rotationf(vec3f(0.5, 0.7, 0.3), 0.1);
	glut_add_interactor(&lights);
	lights.disable();

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    b[' '] = true;
	b['f'] = true;
	b['s'] = true;

	glutMainLoop();

	return 0;
}
