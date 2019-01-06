// simple occlusion query demo
// updated to use ARB_occlusion_query extension
#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_glut_text.h>

#include <shared/quitapp.h>

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

simple_multi_line_text help;
simple_multi_line_text app_info;

GLuint oq_plane;
GLuint oq_sphere;

bool b[256];

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void special(int k, int x, int y);
void idle();

// my functions
void init_opengl();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize(512, 512);
	glutCreateWindow("ARB_occlusion_query example");

	init_opengl();

	font * f = new glut_stroke_roman();
	f->initialize();
	help.set_font(f);
	help.set_text( 
		"This is the help screen -- add helpful text here...\n"
		"  [h or F1]:     toggle help screen\n"
		"  [ ]:     toggle animation\n"
		"  [r]:      toggle rotation animation\n"
		"  [<esc>]: quit");
    app_info.set_font(f);
    app_info.set_text(
        "visible fragments\n"
        "   plane: \n"
        "  sphere: \n");
	
	glut_helpers_initialize();

	cb.keyboard_function = key;
	cb.special_function = special;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -1;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

void init_opengl()
{
    if( ! glh_init_extensions("GL_ARB_occlusion_query ") )
	{
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl << endl
			 << "Press <enter> to quit." << endl;
		quitapp(0);
	}

    GLint bitsSupported;
    glGetQueryivARB(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bitsSupported);
    cout << "Number of counter bits = " << bitsSupported << endl;

	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glGenQueriesARB(1, &oq_sphere);
    glGenQueriesARB(1, &oq_plane );
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);
	if(' ' == k)
		cb.idle_function = b[k] ? idle : 0;

	glutPostRedisplay();
}

void special(int k, int x, int y)
{
	if(GLUT_KEY_F1 == k) key('h',0,0);
	glutPostRedisplay();
}

void idle()
{
	if(b['r'])
		object.trackball.increment_rotation();
}

void display_help()
{
	float w, h;
 	help.get_dimensions(w,h);
	
	float s = (w > h)? w : h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_COLOR);

	glColor3f(.15, .15, .2);
	glBegin(GL_QUADS);
	glVertex2f(-.95, -.95);	
	glVertex2f(-.95,  .95);	
	glVertex2f( .95,  .95);	
	glVertex2f( .95, -.95);	
	glEnd();

	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glColor3f(1, 1, 1);

	glPushMatrix();
	glTranslatef(-.9, .9, 0);
	glScalef(1.8/s, 1.8/s, 1);
	glTranslatef(0, -help.get_font()->get_ascent(), 0);
	help.render();
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);

	reshaper.apply();
}

void display_info()
{
	float w, h;
 	help.get_dimensions(w,h);
	
	float s = (w > h)? w : h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);

	glColor3f(1, 1, 1);

	glPushMatrix();
	glTranslatef(-.9, .9, 0);
	glScalef(1.8/s, 1.8/s, 1);
	glTranslatef(0, -app_info.get_font()->get_ascent(), 0);
	app_info.render();
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);

	reshaper.apply();
}

void draw_objects()
{
    glColor3f(1,1,0);
    glPushMatrix();
    glTranslatef(0, -.025, 0);
    glScalef(1, .05, 1);

    // render cube, with occlusion query
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, oq_plane);
    glutSolidCube(.5);
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    glPopMatrix();

    // render sphere, with occlusion query
    glColor3f(1, 0, 0);
    glPushMatrix();
    glTranslatef(0, .25, 0);
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, oq_sphere);
    glutSolidSphere(.25, 20, 20);
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    glPopMatrix();
}

void set_app_info_string()
{
    GLuint plane_samples, sphere_samples;

    // get results of occlusion queries
    glGetQueryObjectuivARB(oq_plane, GL_QUERY_RESULT_ARB, &plane_samples);
    glGetQueryObjectuivARB(oq_sphere, GL_QUERY_RESULT_ARB, &sphere_samples);

    string s;
    char buff[80];

    s = "visible samples\n  plane: ";
    sprintf(buff, "%d", plane_samples);
    s += buff;
    if( plane_samples == 0 )
    {
        s += "  -- no samples visible";
    }
    s += "\n sphere: ";

    sprintf(buff, "%d", sphere_samples);
    s += buff;
    if( sphere_samples == 0 )
    {
        s += "  -- no samples visible";
    }
    s += "\n";

    app_info.set_text(s);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

    draw_objects(); // first time is to initialize depth buffer

    draw_objects(); // this time gets accurate visible fragment counts

    glPopMatrix();

    set_app_info_string();


	if(b['h'])
		display_help();
    else
        display_info();

	glutSwapBuffers();
}
