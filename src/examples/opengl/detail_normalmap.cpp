#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
							"GL_EXT_secondary_color " \
							"GL_NV_register_combiners " \
							"GL_NV_register_combiners2 " \
							"GL_SGIS_generate_mipmap "
							 
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_glut_callfunc.h>

#include <shared/array_texture.h>
#include <shared/data_path.h>
#include <shared/bumpmap_to_normalmap.h>
#include "../png/nv_png.h"
#include <shared/quitapp.h>

#include "../nvparse/include/nvparse.h"

using namespace glh;

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void idle();

// my functions
void init_opengl();
void set_up_combiners();
void build_combiner_dlist();
void menu(int entry) { key((unsigned char)entry, 0, 0); }

// globals
glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

tex_object_2D base_normalmap, detail_normalmap, decal;
display_list quad;
display_list combiner_config;

library_handle lib;

float light_rotation = 0;

float diffuse_coefficient = .7f;
float specular_coefficient = .5f;

float lerp_factor = .8;
float detail_scale = 5;
float detail_translate = 0;

bool b[256];


int main(int argc, char **argv)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowSize(512, 512);
	glutInit(&argc, argv);
	glutCreateWindow("Detail Normalmaps");

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	cb.display_function = display;

	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -2;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);

	int tweakables = glutCreateMenu(menu);
	glutAddMenuEntry("increase detail scale [s]", 's');
	glutAddMenuEntry("decrease detail scale [S]", 'S');
	glutAddMenuEntry("increase detail translate [t]", 't');
	glutAddMenuEntry("decrease detail translate [T]", 'T');
	glutAddMenuEntry("increase normal lerp factor [l]", 'l');
	glutAddMenuEntry("decrease normal lerp factor [L]", 'L');



	glutCreateMenu(menu);
	glutAddMenuEntry("toggle wireframe [w]", 'w');
	glutAddMenuEntry("toggle animation [ ]", ' ');
	glutAddSubMenu("tweakables", tweakables);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
#ifdef _WIN32
	lib.init("detail_normalmap");
#endif
	key(' ', 0, 0); // animate

	glutMainLoop();
	return 0;
}


void build_detail_normalmap_mipmaps(array2<vec4ub> & in_img, int level)
{
	int w = in_img.get_width() / 2;
	int h = in_img.get_height() / 2;
	array2<vec4ub> img(w, h);
	unsigned char alpha = (unsigned char)(255 * pow(1.5, -level));


	for(int i=0; i < w; i++)
		for(int j=0; j < h; j++)
		{
			int I = i*2;
			int II = I+1;
			int J = j*2;
			int JJ = J+1;
			float f;
			for(int k=0; k < 3; k++)
			{
			  f = (  float(in_img(  I,  J)[k])
			       + float(in_img( II,  J)[k])		
			       + float(in_img(  I, JJ)[k])		
			       + float(in_img( II, JJ)[k])) / 4;
			  img(i,j)[k] = (unsigned char)f;
			}
			img(i,j)[3] = alpha;
		}
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.get_pointer());
	if(w > 1 || h > 1)
		build_detail_normalmap_mipmaps(img, level+1);
}


void init_opengl()
{
	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Unable to initialize because the following extensions were not supported:" << endl
			<< glh_get_unsupported_extensions() << endl;
		quitapp(0);
	}

	glEnable(GL_DEPTH_TEST);

	array2<vec3ub> img;
	array2<unsigned char> bump_img;

	// initialize the normalmaps
	read_png_grey("add_normals__base.png", bump_img);
	bumpmap_to_normalmap(bump_img, img, vec3f(1,1,.05));
	base_normalmap.bind();
	base_normalmap.parameter(GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	make_rgb_texture(img, false);
	base_normalmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	base_normalmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	read_png_grey("add_normals__detail.png", bump_img);
	bumpmap_to_normalmap(bump_img, img, vec3f(1,1,.05));
	detail_normalmap.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.get_width(), img.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.get_pointer());
	array2<vec4ub> img4(img.get_width(), img.get_height());
	{
	  for(int i=0; i < img.get_width(); i++)
		for(int j=0; j < img.get_height(); j++)
		{
			vec3ub & v3 = img(i,j);
			vec4ub v;
			v[0] = v3[0];
			v[1] = v3[1];
			v[2] = v3[2];
			v[3] = 1;
			img4(i,j) = v;
		}
	}
	build_detail_normalmap_mipmaps(img4, 1);
	detail_normalmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	detail_normalmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	quad.new_list(GL_COMPILE);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
	glVertex2f(-1, -1);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 1);
	glVertex2f(-1,  1);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 1);
	glVertex2f( 1,  1);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 0);
	glVertex2f( 1, -1);
	glEnd();
	quad.end_list();

	set_up_combiners();
}

GLH_FUNC void key__t(int x, int y)
{
	detail_translate += .001f;
}
GLH_FUNC void key__T(int x, int y)
{
	detail_translate -= .001f;
}
GLH_FUNC void key__s(int x, int y)
{
	detail_scale += .1f;
}
GLH_FUNC void key__S(int x, int y)
{
	detail_scale -= .1f;
}
GLH_FUNC void key__l(int x, int y)
{
	lerp_factor += .01f;
}
GLH_FUNC void key__L(int x, int y)
{
	lerp_factor -= .01f;
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);

	if('w' == k)
	{
		if(b[k])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if(' '==k)
	{
		glutIdleFunc(b[k] ? idle : 0);
	}
#ifdef _WIN32
	lib.call_func(k,x,y);
#else 
	switch (k) {
	case 't':
	    key__t (x,y);
	    break;
	case 'T':
	    key__T (x,y);
	    break;
	case 's':
	    key__s (x,y);
	    break;
	case 'S':
	    key__S (x,y);
	    break;
	case 'l':
	    key__l (x,y);
	    break;
	case 'L':
	    key__L (x,y);
	    break;
	}

#endif
	glutPostRedisplay();
}


void idle()
{
	light_rotation += to_radians(1);
	glutPostRedisplay();
}

void set_up_combiners()
{
	glActiveTextureARB(GL_TEXTURE0_ARB);
	base_normalmap.bind();
	base_normalmap.enable();
	glActiveTextureARB(GL_TEXTURE1_ARB);
	detail_normalmap.bind();
	detail_normalmap.enable();
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glEnable(GL_REGISTER_COMBINERS_NV);

	combiner_config.new_list(GL_COMPILE);
	build_combiner_dlist();
	combiner_config.end_list();

	combiner_config.call_list();

}

void build_combiner_dlist()
{
	nvparse(
		"!!RC1.0                                           \n"
		"{ // set up lerp factor                           \n"
		"	alpha {                                        \n"
		"		discard = unsigned_invert(tex1);           \n"
		"		discard = col0 * tex1;                     \n"
		"		col0 = sum();                              \n"
		"	}                                              \n"
		"}                                                 \n"
		"                                                  \n"
		"{ // lerp between base and detail                 \n"
		"	rgb {                                          \n"
		"		discard = tex0 * col0.a;                   \n"
		"		discard = tex1 * unsigned_invert(col0.a);  \n"
		"		tex1 = sum();                              \n"
		"	}                                              \n"
		"}                                                 \n"
		"                                                  \n"
		"# Normalize l and h in the combiners!             \n"
		"                                                  \n"
		"{                                                 \n"
		"	rgb { // normalize n and h  (step 1)           \n"
		"		spare0 = expand(tex1) . expand(tex1);      \n"
		"		spare1 = expand(col1) . expand(col1);      \n"
		"	}                                              \n"
		"}                                                 \n"
		"{                                                 \n"
		"	rgb { // normalize n (step 2)                  \n"
		"		discard = expand(tex1);                    \n"
		"		discard = half_bias(tex1) *                \n"
 		"                 unsigned_invert(spare0);         \n"
		"		tex1 = sum();                              \n"
		"	}                                              \n"
		"}                                                 \n"
		"{                                                 \n"
		"	rgb { // normalize h (step 2)                  \n"
		"		discard = expand(col1);                    \n"
		"		discard = half_bias(col1) *                \n"
 		"                 unsigned_invert(spare1);         \n"
		"		col1 = sum();                              \n"
		"	}                                              \n"
		"}                                                 \n"
		"{                                                 \n"
		"	rgb {                                          \n"
		"		spare0 = expand(col0) . tex1;              \n"
		"		spare1 = col1 . tex1;                      \n"
		"	}                                              \n"
		"}                                                 \n"
		"{                                                 \n"
		"	rgb {                                          \n"
		"		spare1 = unsigned(spare1) *                \n"
		"				 unsigned(spare1);                 \n"
		"	}                                              \n"
		"}                                                 \n"
		"{                                                 \n"
		"	rgb {                                          \n"
		"		spare1 = spare1 * spare1;                  \n"
		"	}                                              \n"
		"}                                                 \n"
		"final_product = spare1 * spare1;                  \n"
		"out.rgb = const0 * spare0 + final_product;        \n"
		"out.a = unsigned_invert(zero);                    \n"	);
#ifdef _WIN32
	nvparse_print_errors(stderr);
#endif
}


void display()
{
    vec3f l(3,3,3);

	l.normalize();
	quaternionf q(vec3f(0,0,1), light_rotation);
	q.mult_vec(l); 
	object.trackball.r.inverse().mult_vec(l); // rotate the light into the quad's object space
	vec3f eye(0,0,1);
	object.trackball.r.inverse().mult_vec(eye); // rotate eye vector into quad's object space
	vec3f h = l + eye;
	h.normalize();

	l *= .5f;
	l += .5f;
	glColor4f(l[0], l[1], l[2], lerp_factor);  // put the light vector into the primary color

	h *= .5f;
	h += .5f;
	glSecondaryColor3fvEXT(&h[0]); // put the h vector into the secondary color

	GLfloat diffuse[] = { .5, 1, 0, 1 };
	glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, diffuse);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glLoadIdentity();
	glScalef(detail_scale, detail_scale, 1);
	glTranslatef(detail_translate, detail_translate, 0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_MODELVIEW);

	quad.call_list();

	glPopMatrix();

	glutSwapBuffers();
}
