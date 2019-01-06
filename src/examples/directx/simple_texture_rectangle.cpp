#ifdef UNIX
#  include <GL/glx.h>
#endif

#include <string>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_convenience.h>

#include <shared/nv_png.h>
#include <shared/array_texture.h>
#include <shared/quitapp.h>

#if !defined(GL_TEXTURE_RECTANGLE_NV) && defined(GL_EXT_texture_rectangle)
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#endif

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

tex_object_rectangle tex;
display_list quad;

bool b[256];

bool & translate_texture0 = b['t'];
bool & rotate_texture1 = b['r'];

float tx0rot = 90.f;
float tx1rot = 0.f;

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void menu(int entry) { key((unsigned char)entry, 0, 0); }
void idle();

// my functions
void init_opengl();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutCreateWindow("simple texture rectangle example");

	init_opengl();

	b[' '] = true;

	glut_helpers_initialize();

	cb.keyboard_function = key;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -2;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);


	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("simple_texture_rectangle (c) 2000 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 NVIDIA Corporation", 0);

	glutCreateMenu(menu);
	glutAddSubMenu("copyrights", copyrights);
	glutAddMenuEntry("quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

void init_opengl()
{
    if (!glh_extension_supported("GL_EXT_texture_rectangle") && 
        !glh_extension_supported("GL_NV_texture_rectangle"))
    {
	cerr << "Video card does not support texture rectangles" << endl;
        quitapp(0);
    }

   
	glEnable(GL_DEPTH_TEST);


	array2<vec3ub> img;

	tex.bind();  // get a texture name and bind it
	read_png_rgb("nvlogo_rect.png", img);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, img.get_width(), img.get_height(), 0,
				 GL_RGB, GL_UNSIGNED_BYTE, & img(0,0)[0]);
					 

	

	float aspect = img.get_width() / float(img.get_height());
	// note texture coordinates use image dimensions, not [0,1]
	quad.new_list(GL_COMPILE);
	glPushMatrix();
	if(aspect > 1)
		glScalef(1, 1/aspect, 1);
	else
		glScalef(aspect, 1, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-1, -1, 0);

	glTexCoord2f(0, img.get_height());
	glVertex3f(-1,  1, 0);

	glTexCoord2f(img.get_width(), img.get_height());
	glVertex3f( 1,  1, 0);

	glTexCoord2f(img.get_width(), 0);
	glVertex3f( 1, -1, 0);
	glEnd();

	glPopMatrix();
	quad.end_list();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);
	glutPostRedisplay();
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();
	glRotatef(60.0f, -1.0f, 1.0f, 0.0f );

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex.texture);
	glEnable(GL_TEXTURE_RECTANGLE_NV);

	quad.call_list();

	
	glPopMatrix();

	glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}
