
/*

  issues

	-  2dh frustum not correct
	-  2dh view looks weird when the "real" camera translates (is that correct?)

*/

#if defined(WIN32)
#  include <windows.h>
#endif

#include <string>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_obs.h>
#include <glh/glh_glut2.h>

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, scenecam, object;
glut_perspective_reshaper reshaper;

string title_l = "scene";
string title_r = "eye view";

bool b[256];


int current_vertex = 2;
float w[3] = {1,1,1};

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void menu(int item) { key((unsigned char)item, 0, 0); }

// my functions
void init_opengl();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize( 1024, 512 );
	glutCreateWindow("Visualizing homogeneous triangles...");

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	scenecam.configure_buttons(1);
	scenecam.set_camera_mode(true);
	scenecam.trackball.r = rotationf(vec3f(0,1,0), to_radians(45)) * rotationf(vec3f(1,0,0), to_radians(-10));
	scenecam.translator.t = vec3f(2, .3, 1);
	object.configure_buttons(1);
	object.translator.t[2] = -1;
	reshaper.aspect_factor = .5f;

	camera.disable();
	scenecam.disable();
	
	camera.set_parent_rotation( & camera.trackball.r);
	object.set_parent_rotation( & camera.trackball.r);
	scenecam.set_parent_rotation( & scenecam.trackball.r);


	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&camera);
	glut_add_interactor(&scenecam);
	glut_add_interactor(&reshaper);

	glutCreateMenu(menu);
	glutAddMenuEntry("manipulate \"external view\" camera [1]", '1');
	glutAddMenuEntry("manipulate regular camera [2]", '2');
	glutAddMenuEntry("manipulate triangle [3]", '3');
	glutAddMenuEntry("increase w [w]", 'w');
	glutAddMenuEntry("decrease w [W]", 'W');
	glutAddMenuEntry("flip red vertex (make external) [r]", 'r');
	glutAddMenuEntry("flip green vertex (make external) [g]", 'g');
	glutAddMenuEntry("flip blue vertex (make external) [b]", 'b');
	glutAddMenuEntry("toggle depth clamp [~]", '~');
	glutAddMenuEntry("switch between 3D and 2DH external view [h]", 'h');
	glutAddMenuEntry("quit [esc]", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

void init_opengl()
{
	glEnable(GL_DEPTH_TEST);
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);

	if('~' == k)
	{
		if(! b[k])
			glDisable(GL_DEPTH_CLAMP_NV);
		else
			glEnable(GL_DEPTH_CLAMP_NV);
	}

	if('1' == k)
	{
		scenecam.enable();
		camera.disable();
		object.disable();
	}

	if('2' == k)
	{
		scenecam.disable();
		camera.enable();
		object.disable();
	}

	if('3' == k)
	{
		scenecam.disable();
		camera.disable();
		object.enable();
	}

	if('r' == k)
		current_vertex = 0;
	if('g' == k)
		current_vertex = 1;
	if('b' == k)
		current_vertex = 2;
    
	if('w' == k)
		w[current_vertex] += .125f;

	if('W' == k)
		w[current_vertex] -= .125f;

	char buff[128];
	char *color[3] = { "red", "green", "blue" };
	sprintf(buff, "eye_view    %s vertex w == %.3f", color[current_vertex], w[current_vertex] * (b[color[current_vertex][0]] ? -1 : 1));
	title_r = string(buff);

	glutPostRedisplay();
}

void draw_external_triangle()
{
	glBegin(GL_TRIANGLES);

	float R = b['r'] ? -1 : 1;
	float G = b['g'] ? -1 : 1;
	float B = b['b'] ? -1 : 1;

	glColor3f(1,0,0);

	glVertex4f(-.5 * R, -.5 * R, 0, w[0] * R );

	glColor3f(0,1,0);
	glVertex4f( .5 * G, -.5 * G, 0, w[1] * G );

	glColor3f(0,0,1);
	glVertex4f(  0 * B,  .5 * B, 0, w[2] * B );

	glEnd();
}


// left
void display_scene()
{
	title_l = "3D (x/w, y/w, z/w)";
	
	// push far plane out for external cam rendering...
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float zfar = reshaper.zFar;
	reshaper.zFar = 5000;
	reshaper.apply_projection();
	reshaper.zFar = zfar;
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	scenecam.apply_inverse_transform();

	glPushMatrix();
	object.apply_transform();
	draw_external_triangle();
	glPopMatrix();

	glPushMatrix();
	camera.apply_transform();
	glColor3f(1,1,1);
	glBegin(GL_POINTS);
	glVertex3f(0,0,0);  // draw the camera's location
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-1, -1, 0);
	glVertex3f( 1, -1, 0);
	glVertex3f( 1,  1, 0);
	glVertex3f(-1,  1, 0);
	glVertex3f(-1, -1, 0);
	glEnd();            // draw the camera's z=0 plane
	glColor4f(0,1,1,.2f);
	glDepthMask(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	glVertex3f(-1, -1, 0);
	glVertex3f( 1, -1, 0);
	glVertex3f( 1,  1, 0);
	glVertex3f(-1,  1, 0);
	glEnd();
	glDepthMask(1);
	glDisable(GL_BLEND);

	reshaper.apply_projection_inverse(); 
	glutWireCube(2);  // draw the camera's frustum
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(0);
	glColor4f(0,1,1,.2f);
	glutSolidCube(2);
	glDisable(GL_BLEND);
	glColor3f(1,1,1);
	glDepthMask(1);
	glPopMatrix();

	glPopMatrix();

	// put far plane back ...
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	reshaper.apply_projection();
	glMatrixMode(GL_MODELVIEW);


}

void funny_vertex(matrix4f & m, matrix4f & m2, vec4f v)
{
	m2.mult_matrix_vec(v);
	v[0] /= v[3];
	v[1] /= v[3];
	v[2] /= v[3];
	v[3] = 1;
	m.mult_matrix_vec(v);
	glVertex3f(v[0], v[1], -v[3]);
}


void draw_w_cube_wireframe()
{
	matrix4f m = reshaper.get_projection();
	matrix4f m2 = reshaper.get_projection_inverse();

	int i;

	for(i=-1; i < 2; i+=2)
	{
		glBegin(GL_LINE_LOOP);
		funny_vertex(m, m2, vec4f(-1,-1,i,1));
		funny_vertex(m, m2, vec4f( 1,-1,i,1));
		funny_vertex(m, m2, vec4f( 1, 1,i,1));
		funny_vertex(m, m2, vec4f(-1, 1,i,1));
		glEnd();
	}
	glBegin(GL_LINES);
	{
		funny_vertex(m, m2, vec4f(-1,-1,-1,1));
		funny_vertex(m, m2, vec4f(-1,-1, 1,1));
		funny_vertex(m, m2, vec4f( 1,-1,-1,1));
		funny_vertex(m, m2, vec4f( 1,-1, 1,1));
		funny_vertex(m, m2, vec4f( 1, 1,-1,1));
		funny_vertex(m, m2, vec4f( 1, 1, 1,1));
		funny_vertex(m, m2, vec4f(-1, 1,-1,1));
		funny_vertex(m, m2, vec4f(-1, 1, 1,1));
	}
	glEnd();
		
}

void draw_w_cube_solid()
{
	matrix4f m = reshaper.get_projection();
	matrix4f m2 = reshaper.get_projection_inverse();

	int i;

	for(i=-1; i < 2; i+=2)
	{
		glBegin(GL_QUADS);
		funny_vertex(m, m2, vec4f( i,-1,-1,1));
		funny_vertex(m, m2, vec4f( i,-1, 1,1));
		funny_vertex(m, m2, vec4f( i, 1, 1,1));
		funny_vertex(m, m2, vec4f( i, 1,-1,1));

		funny_vertex(m, m2, vec4f(-1, i, 1, 1));
		funny_vertex(m, m2, vec4f( 1, i, 1, 1));
		funny_vertex(m, m2, vec4f( 1, i,-1, 1));
		funny_vertex(m, m2, vec4f(-1, i,-1, 1));

		funny_vertex(m, m2, vec4f(-1,-1,i,1));
		funny_vertex(m, m2, vec4f( 1,-1,i,1));
		funny_vertex(m, m2, vec4f( 1, 1,i,1));
		funny_vertex(m, m2, vec4f(-1, 1,i,1));
		glEnd();
	}
}

void draw_w_0_plane(bool solid = false)
{
	matrix4f m = reshaper.get_projection();
	matrix4f m2;

	if(! solid)
	{
		glColor3f(1,1,1);
		glPointSize(3);
		glBegin(GL_POINTS);
		funny_vertex(m, m2, vec4f(0,0,0,1));  // draw the camera's location
		glEnd();
		glPointSize(1);
	}

	if(solid)
		glBegin(GL_QUADS);
	else
		glBegin(GL_LINE_LOOP);
	funny_vertex(m, m2, vec4f(-1,-1,0,1));
	funny_vertex(m, m2, vec4f( 1,-1,0,1));
	funny_vertex(m, m2, vec4f( 1, 1,0,1));
	funny_vertex(m, m2, vec4f(-1, 1,0,1));
	glEnd();



}


void funny_vertex2(matrix4f & m, vec4f v)
{
	m.mult_matrix_vec(v);
	glVertex3f(v[0], v[1], -v[3]);
}

void draw_2DH_triangle()
{
	matrix4f m = reshaper.get_projection() * camera.get_inverse_transform() * object.get_transform();

	float R = b['r'] ? -1 : 1;
	float G = b['g'] ? -1 : 1;
	float B = b['b'] ? -1 : 1;

	glBegin(GL_TRIANGLES);

	glColor3f(1,0,0);
	funny_vertex2(m, vec4f(-.5 * R, -.5 * R, 0, w[0] * R));

	glColor3f(0,1,0);
	funny_vertex2(m, vec4f( .5 * G, -.5 * G, 0, w[1] * G));

	glColor3f(0,0,1);
	funny_vertex2(m, vec4f( 0 * B, .5 * B, 0, w[2] * B));

	glEnd();
}



// left
void display_2DH()
{
	title_l = "2DH (x, y, w)";

	// push far plane out for external cam rendering...
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float zfar = reshaper.zFar;
	reshaper.zFar = 5000;
	reshaper.apply_projection();
	reshaper.zFar = zfar;
	glMatrixMode(GL_MODELVIEW);


	glPushMatrix();
	scenecam.apply_inverse_transform();
	camera.apply_transform();

	draw_2DH_triangle();


	//camera.apply_transform();
    draw_w_0_plane(false);  // draw the camera's z=0 plane
	glColor4f(1,1,0,.2f);
	glDepthMask(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_w_0_plane(true);
	glDepthMask(1);
	glDisable(GL_BLEND);

	draw_w_cube_wireframe();  // draw the camera's frustum
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(0);
	glColor4f(1,1,0,.2f);
	draw_w_cube_solid();
	glDisable(GL_BLEND);
	glColor3f(1,1,1);
	glDepthMask(1);

	glPopMatrix();

	// put far plane back ...
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	reshaper.apply_projection();
	glMatrixMode(GL_MODELVIEW);

}

// right
void display_r()
{
	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();
	draw_external_triangle();

	glPopMatrix();
}


void render_string(const char * str)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-.9f, .8f, 0);
	glScalef(.0006f, .0006f, 1);
	while(*str)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *str);
		//glTranslatef(glutStrokeWidth(GLUT_STROKE_ROMAN, *str), 0, 0);
		str++;
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
}



void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	GLint w = vp[2]/2;
	GLint h = vp[3];

	// left
	glViewport(0,0, w, h);
	if(b['h'])
		display_2DH();
	else
		display_scene();
	glColor3f(1,1,1);
	render_string(title_l.c_str());

	// right
	glViewport(w,0, w, h);
	display_r();
	glColor3f(1,1,1);
	render_string(title_r.c_str());

	glViewport(vp[0], vp[1], vp[2], vp[3]);

	glutSwapBuffers();
}
