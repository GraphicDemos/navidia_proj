/*
  This is a demonstration program for the DOT_PRODUCT_REFLECT_CUBE_MAP_NV
  texture shader program.  It illustrates the use of this texture shader
  for local light sources.

  Cass Everitt
  8-27-00
*/

#if defined(WIN32)
# include <windows.h>
#elif defined(UNIX)
# include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
							"GL_NV_texture_shader " \
							"GL_ARB_vertex_program " \
							"GL_SGIS_generate_mipmap "							 

#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_obs.h>
#include <glh/glh_cube_map.h>

#include <nv_png.h>
#include <shared/array_texture.h>
#include <shared/bumpmap_to_normalmap.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

#include "../nvparse/include/nvparse.h"

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object, light_object;
glut_perspective_reshaper reshaper;

// environment map
tex_object_cube_map cubemap, normcubemap;
// normal map
tex_object_2D normalmap;
tex_object_2D glossmap;

arb_vertex_program vp_reflect;
arb_vertex_program vp_gloss;
display_list torus, gloss_config, reflect_config;

int I = 40, J = 40;
float core_radius = .5f;
float meridian_radius = .25f;
float s_trans = 0, t_trans = 0;
float st_rot = 0.f;

bool b[256];
float bumpscale = 2.f;
vec3f lightpos; // in eye space
vec3f txscale(2/(3.1415927f), 1/(3.1415927f), 2/(3.1415927f));
#define PROG_BUFF_SIZE 50000
char * program_buffer;


struct single_light
{
	typedef GLfloat Type;
	int components;
	GLenum type;
	GLenum format;
	single_light(float _power) :
	components(3), type(GL_FLOAT), format(GL_RGB), power(_power) {}
	
	void operator() (const vec3f & v, Type * t)
	{
		float z = v[2] > 0 ? v[2] : 0;
		z = pow(z, power);
		t[0] = z;
		t[1] = z;
		t[2] = z;
	}
	float power;
};


// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void menu(int entry) { key((unsigned char)entry, 0, 0); }
void idle();

// my functions
void init_opengl();
void draw_torus__reflect_cube_map();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize(512, 512);
	glutCreateWindow("NV_texture_shader reflect-into-cubemap for local light");

	b['p'] = true;
	b['s'] = true;
	b['g'] = true;

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -2;
	light_object.configure_buttons(1);
	light_object.dolly.dolly = vec3f(.6, .6, -1.5);
	light_object.disable();

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);
	glut_add_interactor(&light_object);

	

	int toggles = glutCreateMenu(menu);
	glutAddMenuEntry("do glossmap pass [g]", 'g');

	int tweak = glutCreateMenu(menu);
	glutAddMenuEntry("increase bump scale [+]", '+');
	glutAddMenuEntry("decrease bump scale [-]", '-');
	glutAddMenuEntry("increase shininess [e]", 'e');
	glutAddMenuEntry("decrease shininess [E]", 'E');

	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("bump_reflect_local_light (c) 2001 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2001 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);

	glutCreateMenu(menu);
	glutAddSubMenu("toggles", toggles);
	glutAddSubMenu("tweakables", tweak);
	glutAddSubMenu("copyright info", copyrights);
	glutAddMenuEntry("move object [1]", '1');
	glutAddMenuEntry("move light [2]", '2');
	glutAddMenuEntry("quit [esc]", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutDisplayFunc(display);
    glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}


void make_normalmap_texture(const array2<vec3f> & normals)
{
	int imgsize = normals.get_width();
	GLshort * simg = new GLshort[imgsize*imgsize*2];
	GLshort * sip = simg;

	for(int j=0; j < imgsize; j++)
	{
		for(int i=0; i < imgsize; i++)
		{
			const vec3f & n = normals(i,j);

			*sip++ = (GLshort)(n[0] * 32767);
			*sip++ = (GLshort)(n[1] * 32767);

		}
	}
	normalmap.bind();

	normalmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	normalmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	normalmap.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	normalmap.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	normalmap.parameter(GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SIGNED_HILO_NV, imgsize, imgsize, 0, GL_HILO_NV,
					 GL_SHORT, simg);

	delete [] simg;
}

void
SavePPM(char *filename, int width, int height, float *data)
{
	FILE *f = fopen(filename, "wb");
	if (!f) return;
	fprintf(f, "P6\n#generated\n%d %d\n255\n", width, height);
	unsigned char *imgbyte = (unsigned char*)malloc(width*height*3);
	for (int j=height-1; j>=0; j--) {
		for (int i=0; i<width*3; i++) {
			int byte = (int) (data[i+j*width*3]*255);
			if (byte < 0) byte = 0;
			else if (byte > 255) byte=255;
			imgbyte[i+(height-1-j)*width*3] = byte;
		}
	}

	fwrite(imgbyte, 1, width*height*3, f);
	free(imgbyte);
	//fprintf(f, "\n");
	fclose(f);
}

template <class FunctionOfDirection>
		void make_cube_map_save(FunctionOfDirection & f, GLenum internal_format,
		int size, int level = 0)
	{
		typedef typename FunctionOfDirection::Type Type;
		int components = f.components;
		GLenum type = f.type;
		GLenum format = f.format;
		Type * image  = new Type[size*size*components];
		Type * ip;

		float offset = .5;
		float delta = 1;
		float halfsize = size/2.f;
		vec3f v;

		// positive x image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[2] = -(i*delta + offset - halfsize);
					v[1] = -(j*delta + offset - halfsize);
					v[0] = halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face1.ppm", size, size, image);
		}
		// negative x image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[2] = (i*delta + offset - halfsize);
					v[1] = -(j*delta + offset - halfsize);
					v[0] = -halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face2.ppm", size, size, image);
			glTexImage2D(GLH_CUBE_MAP_NEGATIVE_X,
						 level, internal_format, size, size, 0, format, type, image);
		}

		// positive y image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[0] = (i*delta + offset - halfsize);
					v[2] = (j*delta + offset - halfsize);
					v[1] = halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face3.ppm", size, size, image);
			glTexImage2D(GLH_CUBE_MAP_POSITIVE_Y,
						 level, internal_format, size, size, 0, format, type, image);
		}
		// negative y image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[0] = (i*delta + offset - halfsize);
					v[2] = -(j*delta + offset - halfsize);
					v[1] = -halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face4.ppm", size, size, image);
			glTexImage2D(GLH_CUBE_MAP_NEGATIVE_Y,
						 level, internal_format, size, size, 0, format, type, image);
		}

		// positive z image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[0] = (i*delta + offset - halfsize);
					v[1] = -(j*delta + offset - halfsize);
					v[2] = halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face5.ppm", size, size, image);
			glTexImage2D(GLH_CUBE_MAP_POSITIVE_Z,
						 level, internal_format, size, size, 0, format, type, image);
		}
		// negative z image	
		{
			ip = image;
			for(int j = 0; j < size; j++)
			{
				for(int i=0; i < size; i++)
				{
					v[0] = -(i*delta + offset - halfsize);
					v[1] = -(j*delta + offset - halfsize);
					v[2] = -halfsize;
					v.normalize();
					f(v, ip);
					ip += components;
				}
			}
			SavePPM("face6.ppm", size, size, image);
			glTexImage2D(GLH_CUBE_MAP_NEGATIVE_Z,
						 level, internal_format, size, size, 0, format, type, image);
		}
		delete [] image;
	}


void init_opengl()
{
	glEnable(GL_DEPTH_TEST);

	glClearColor(.25f, .25f, .25f, 1);

	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp(0);
	}
	
	// bump -> hilo normal map
	array2<unsigned char> img;
	read_png_grey("bump_map_torus.png", img);
	array2<vec3f> normals;
	bumpmap_to_normalmap(img, normals);
	make_normalmap_texture(normals);

	// gloss map
	array2<vec3ub> gloss;
	read_png_rgb("rgb_gloss_map_torus.png", gloss);
	glossmap.bind();
	make_rgb_texture(gloss, true);
	glossmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glossmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glossmap.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	glossmap.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	// cube map
	cubemap.bind();

	single_light f(25);

	make_cube_map(f, GL_RGB, 64, 0);
	make_cube_map(f, GL_RGB, 32, 1);
	make_cube_map(f, GL_RGB, 16, 2);
	make_cube_map(f, GL_RGB,  8, 3);
	make_cube_map(f, GL_RGB,  4, 4);
	make_cube_map(f, GL_RGB,  2, 5);
	make_cube_map(f, GL_RGB,  1, 6);

	//make_cube_map_save(f, GL_RGB,  64, 0);

	cubemap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	cubemap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cubemap.parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	cubemap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// normalization cube map
	normcubemap.bind();
	normalize_vector norm;
	make_cube_map(norm, GL_RGB, 64, 0);
	make_cube_map(norm, GL_RGB, 32, 1);
	make_cube_map(norm, GL_RGB, 16, 2);
	make_cube_map(norm, GL_RGB,  8, 3);
	make_cube_map(norm, GL_RGB,  4, 4);
	make_cube_map(norm, GL_RGB,  2, 5);
	make_cube_map(norm, GL_RGB,  1, 6);
	normcubemap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	normcubemap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	normcubemap.parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	normcubemap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	torus.new_list(GL_COMPILE);
	draw_torus__reflect_cube_map();
	torus.end_list();

	program_buffer = read_text_file("bump_reflect_local_light/bump_reflect_local_light__reflect.vp");
	vp_reflect.bind();
	nvparse(program_buffer);
	nvparse_print_errors(stderr);
    delete [] program_buffer;

	program_buffer = read_text_file("bump_reflect_local_light/bump_reflect_local_light__gloss.vp");
	vp_gloss.bind();
	nvparse(program_buffer);
	nvparse_print_errors(stderr);
    delete [] program_buffer;


	reflect_config.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                                              \n"
		"# reflection shader config                           \n"
		"texture_2d();                                        \n"
		"dot_product_reflect_cube_map_eye_from_qs_1of3(tex0); \n"
		"dot_product_reflect_cube_map_eye_from_qs_2of3(tex0); \n"
		"dot_product_reflect_cube_map_eye_from_qs_3of3(tex0); \n");
	nvparse_print_errors(stderr);
	reflect_config.end_list();

	gloss_config.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                     \n"
		"# gloss pass shader config  \n"
		"nop();                      \n"
		"nop();                      \n"
		"nop();                      \n"
		"texture_2d();               \n");
	nvparse_print_errors(stderr);
	gloss_config.end_list();

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 93, 0, 0, 1, 1);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 95, txscale[0], txscale[1], txscale[2], bumpscale);
}

float light_exponent = 32;

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);

	if('+'==k || '='==k)
		bumpscale += .1f;
	if('-'==k)
		bumpscale -= .1f;

	if('1' == k)
	{
		object.enable();
		light_object.disable();
	}

	if('2' == k)
	{
		light_object.enable();
		object.disable();
	}

	if('e' == k || 'E' == k)
	{
		if('e'==k)
			light_exponent *= 2;
		if('E'==k)
			light_exponent /= 2;
		
		// cube map
		cubemap.bind();
		single_light f(light_exponent);
		make_cube_map(f, GL_RGB, 64, 0);
		make_cube_map(f, GL_RGB, 32, 1);
		make_cube_map(f, GL_RGB, 16, 2);
		make_cube_map(f, GL_RGB,  8, 3);
		make_cube_map(f, GL_RGB,  4, 4);
		make_cube_map(f, GL_RGB,  2, 5);
		make_cube_map(f, GL_RGB,  1, 6);
	}

	if('l' == k) // reload programs
	{
		program_buffer = read_text_file("bump_reflect_local_light__reflect.vp");
        vp_reflect.load(program_buffer);
		delete [] program_buffer;
		program_buffer = read_text_file("bump_reflect_local_light__gloss.vp");
		vp_gloss.load(program_buffer);
        delete [] program_buffer;
	}

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 95, txscale[0], txscale[1], txscale[2], bumpscale);

	glutPostRedisplay();
}


vec3f get_torus_vertex(float theta, float rho)
{

	vec3f p;
	p[0] = core_radius*cos(theta) + meridian_radius*cos(theta)*cos(rho);
	p[1] = core_radius*sin(theta) + meridian_radius*sin(theta)*cos(rho);
	p[2] = meridian_radius*sin(rho);

	return p;
}

matrix4f get_torus_tangent_space_matrix(float theta, float rho)
{
	vec3f n;
	n[0] = cos(theta)*cos(rho);
	n[1] = sin(theta)*cos(rho);
	n[2] = sin(rho);

	vec3f t;
	t[0] = -sin(theta);
	t[1] = cos(theta);
	t[2] = 0;

	vec3f b;
	b[0] = cos(theta)*-sin(rho);
	b[1] = sin(theta)*-sin(rho);
	b[2] = cos(rho);

	matrix4f m;
	m.make_identity();

	m(0,0) = t[0];	m(0,1) = t[1];	m(0,2) = t[2];
	m(1,0) = b[0];	m(1,1) = b[1];	m(1,2) = b[2];
	m(2,0) = n[0];	m(2,1) = n[1]; 	m(2,2) = n[2];

	return m;
}


void vertex__reflect_cube_map(float theta, float rho)
{
	vec3f p = get_torus_vertex(theta, rho);
	matrix4f basis = get_torus_tangent_space_matrix(theta, rho);
	
	glVertexAttrib3fARB(2, basis(0,0), basis(0,1), basis(0,2));
	glVertexAttrib3fARB(3, basis(1,0), basis(1,1), basis(1,2));
	glVertexAttrib3fARB(4, basis(2,0), basis(2,1), basis(2,2));
	glVertexAttrib2fARB(8, theta, rho);
	glVertexAttrib3fARB(0, p[0], p[1], p[2]);
}

void draw_torus__reflect_cube_map()
{
	for(int i=1; i < I; i++)
	{
		float theta0 = (i-1)/(I-1.f) * 2 * 3.1415927f;
		float theta1 = (i  )/(I-1.f) * 2 * 3.1415927f;
		glBegin(GL_QUAD_STRIP);
		for(int j=0; j < J; j++)
		{
			float rho    = j/(J-1.f) * 2 * 3.1415927f;
			vertex__reflect_cube_map(theta0, rho);
			vertex__reflect_cube_map(theta1, rho);
		}
		glEnd();
	}
}


void do_cube_map_reflect_pass()
{
	vp_reflect.bind();
	glEnable(GL_VERTEX_PROGRAM_ARB);

	glEnable(GL_TEXTURE_SHADER_NV);
	
	glActiveTextureARB( GL_TEXTURE0_ARB );
	normalmap.bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);		

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);		

	glActiveTextureARB( GL_TEXTURE2_ARB );
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);		
	// stage 3 -- dot product, cube map lookup
	glActiveTextureARB( GL_TEXTURE3_ARB );
	cubemap.bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);		

	glActiveTextureARB( GL_TEXTURE0_ARB );

	reflect_config.call_list();

	glVertexAttrib3fARB(5, .5, .5, .5);

	torus.call_list();

	glDisable(GL_TEXTURE_SHADER_NV);
	glDisable(GL_VERTEX_PROGRAM_ARB);
}

void do_glossmap_pass()
{
	vp_gloss.bind();

    glEnable(GL_VERTEX_PROGRAM_ARB);

	glEnable(GL_TEXTURE_SHADER_NV);
	
	// stage 3 -- regular 2D
	glActiveTextureARB( GL_TEXTURE3_ARB );
	glossmap.bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		
	glActiveTextureARB( GL_TEXTURE0_ARB );

	gloss_config.call_list();

	torus.call_list();

	glDisable(GL_TEXTURE_SHADER_NV);
	glDisable(GL_VERTEX_PROGRAM_ARB);
}

void display()
{
	light_object.get_transform().mult_matrix_vec(vec3f(), lightpos);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 94, lightpos[0], lightpos[1], lightpos[2], 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	matrix4f orot;
	object.trackball.r.get_value(orot);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

	if(b['p'])
		do_cube_map_reflect_pass();

	if(b['g'])
	{
		glDepthFunc(GL_EQUAL);
		glDepthMask(0);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glEnable(GL_BLEND);

		do_glossmap_pass();

		glDisable(GL_BLEND);
		glDepthMask(1);
		glDepthFunc(GL_LESS);
	}


	glPopMatrix();

	glColor3f(1,1,.5f);
	glPushMatrix();
	glTranslatef(lightpos[0], lightpos[1], lightpos[2]);
	glutSolidSphere(.01f, 10, 10);
	glPopMatrix();
	glColor3f(1,1,1);

	glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}
