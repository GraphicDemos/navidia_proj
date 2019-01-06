/*

  This demo is similar to "DX6 EMBM" in that it treats
  a (ds,dt) result from a previous texture stage as a
  signed offset for the current stage's (s,t).  In the
  current stage, the (ds,dt) vector is transformed by a
  2x2 matrix to yield (ds',dt') = M (ds, dt), and the
  current stage's lookup is lookup_2D(s + ds', t + dt').

  This approach for EMBM implies that the environment is
  captured in a single 2D texture and that the transformed
  (ds, dt) represent a consistent normal perturbation. It
  is a dubious implication, but there are cases where
  it is "good enough".

  In this case, I'm using the x and y of a per-vertex normal
  or reflection vector as the (s,t) for a 2D texture.
  The final lookup is lookup_2D(s' + ds', t' + dt').

  One problem of this "DX6 EMBM" is that the 2x2 matrix is
  constant state.  For tangent-space bump mapping, we need
  this matrix to vary per-fragment through interpolated
  per-vertex values.  We can achieve this using the
  DOT_PRODUCT_TEXTURE_2D texture shader by passing in a
  "normal" vector that is (ds, dt, 1), and the texture
  coordinates of the dot product stages are:
  (s1, t1, r1)  =  (M00, M01, s)
  (s2, t2, r2)  =  (M10, M11, t)

  This way, we have a 2x2 matrix that is interpolated per-vertex,
  but at the expense of an additional texture shader stage.

  This demo illustrates the problem of "DX6 EMBM" using
  tangent-space normal maps, and how DOT_PRODUCT_TEXTURE_2D
  can improve the results.


  Cass Everitt
  10-31-00

*/

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
							"GL_ARB_texture_cube_map " \
							"GL_NV_register_combiners " \
							"GL_NV_texture_shader " \
							"GL_ARB_vertex_program " \
							"GL_SGIS_generate_mipmap "						 

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <shared/array_texture.h>
#include <shared/data_path.h>
#include <shared/bumpmap_to_normalmap.h>
#include <nv_png.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

#include <nvparse.h>

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object, environment, *current_interactor;
glut_perspective_reshaper reshaper;

tex_object_2D normalmap_dsdt, normalmap_rgb, decal, cubeface;
arb_vertex_program vp_dsdt, vp_dotproduct;
display_list ts_dsdt, ts_dsdt_notex, ts_dotproduct, ts_dotproduct_notex;
display_list rc_tex, rc_notex;

float light_rotation = 0;
float bump_scale = .12f;
float env_scale = .75f;
float env_trans = 0.f;

bool b[256];

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void idle();

// my functions
void init_opengl();
void set_up_shaders();

void menu(int entry) { key((unsigned char)entry, 0, 0); }

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize(512, 512);
	glutCreateWindow("tangent-space offset bump mapping");

	b['t'] = true;  // decal texture on
	b['s'] = true;  // specular
	b['o'] = true;  // use OFFSET_TEXTURE_2D
	b['r'] = true;  // animate rotation (when animation enabled)

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	cb.display_function = display;
	cb.idle_function = idle;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.trackball.incr = rotationf(vec3f(0,1,0), to_radians(.2));
	object.dolly.dolly[2] = -2.2;
	environment.configure_buttons(1);
	environment.disable();

	current_interactor = & object;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&environment);
	glut_add_interactor(&reshaper);

	int toggles = glutCreateMenu(menu);
	glutAddMenuEntry("animate [ ]", ' ');
	glutAddMenuEntry("wireframe [w]", 'w');
	glutAddMenuEntry("toggle per-vertex 2x2 matrix [o]", 'o');

	int tweakables = glutCreateMenu(menu);
	glutAddMenuEntry("increase (ds,dt) scale [+]", '+');
	glutAddMenuEntry("decrease (ds,dt) scale [-]", '-');
	glutAddMenuEntry("increase (s,t) scale [0]", '0');
	glutAddMenuEntry("decrease (s,t) scale [9]", '9');

	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("offset_bump_mapping (c) 2001 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2001 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);
	glutAddMenuEntry("libpng -- Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson", 0);

	glutCreateMenu(menu);

	glutAddSubMenu("toggles", toggles);
	glutAddSubMenu("tweakables", tweakables);
	glutAddSubMenu("copyright info", copyrights);
	glutAddMenuEntry("quit [q]", 'q');

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	key(' ', 0, 0); // turn on animation

	glutMainLoop();
	return 0;
}


void init_opengl()
{
	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Unable to initialize because the following extensions were not supported:" << endl
			<< glh_get_unsupported_extensions() << endl;
		quitapp(0);
	}

#if 1 // really bizarre hack
	vec3f n;
	glMultiTexCoord3fvARB(GL_TEXTURE2_ARB, &n[0]);
	glMultiTexCoord3fvARB(GL_TEXTURE3_ARB, &n[0]);
#endif

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	array2<vec3ub> img;
	array2<vec3f> nimg;

	// initialize the decal
	read_png_rgb("earth.png", img);
	decal.bind();
	make_rgb_texture(img, true);
	decal.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	decal.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// initialize the single cube face environment map
	read_png_rgb("spots_embm.png", img);
	cubeface.bind();
	make_rgb_texture(img, true);
	cubeface.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	cubeface.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	cubeface.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
	cubeface.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP);

	
	// initialize the normalmap
	array2<unsigned char> bump_img;
	read_png_grey("earth_bump.png", bump_img);
	bumpmap_to_normalmap(bump_img, nimg, vec3f(1,1,.05));
	for(int i=0; i < nimg.get_width(); i++)
		for( int j=0; j < nimg.get_height(); j++)
			nimg(i,j)[2] = 1;
	normalmap_dsdt.bind();
	normalmap_dsdt.parameter(GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DSDT_MAG_NV, nimg.get_width(), nimg.get_height(), 0,
				 GL_DSDT_MAG_NV, GL_FLOAT, (const void *)nimg.get_pointer());
	normalmap_dsdt.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	normalmap_dsdt.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	normalmap_rgb.bind();
	normalmap_rgb.parameter(GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SIGNED_RGB_NV, nimg.get_width(), nimg.get_height(), 0,
				 GL_RGB, GL_FLOAT, (const void *)nimg.get_pointer());
	normalmap_rgb.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	normalmap_rgb.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ts_dsdt.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                         \n"
		"texture_2d();                   \n"
		"texture_2d();                   \n"
		"nop();                          \n"
		"offset_2d(tex0, 1, 0, 0, 1);    \n"
		);
	nvparse_print_errors(stderr);
	ts_dsdt.end_list();

	ts_dsdt_notex.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                         \n"
		"texture_2d();                   \n"
		"nop();                          \n"
		"nop();                          \n"
		"offset_2d(tex0, 1, 0, 0, 1);    \n"
		);
	nvparse_print_errors(stderr);
	ts_dsdt_notex.end_list();

	ts_dotproduct.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                            \n"
		"texture_2d();                      \n"
		"texture_2d();                      \n"
		"dot_product_2d_1of2(tex0);         \n"
		"dot_product_2d_2of2(tex0);         \n"
		);
	nvparse_print_errors(stderr);
	ts_dotproduct.end_list();

	ts_dotproduct_notex.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                            \n"
		"texture_2d();                      \n"
		"nop();                             \n"
		"dot_product_2d_1of2(tex0);         \n"
		"dot_product_2d_2of2(tex0);         \n"
		);
	nvparse_print_errors(stderr);
	ts_dotproduct_notex.end_list();

	rc_tex.new_list(GL_COMPILE);
	nvparse(
		"!!RC1.0                            \n"
		"out.rgb = col0 * tex1 + tex3;      \n"
		);
	nvparse_print_errors(stderr);
	rc_tex.end_list();

	rc_notex.new_list(GL_COMPILE);
	nvparse(
		"!!RC1.0                            \n"
		"out.rgb = tex3;                    \n"
		);
	nvparse_print_errors(stderr);
	rc_notex.end_list();

	glEnable(GL_REGISTER_COMBINERS_NV);

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 95, 1, 2, 0, -1);

	char * txt;
	vp_dsdt.bind();
	txt = read_text_file("offset_bump_mapping/offset_bump_mapping__dsdt.vp");
	nvparse(txt);
	nvparse_print_errors(stderr);
	delete [] txt;

	vp_dotproduct.bind();
	txt = read_text_file("offset_bump_mapping/offset_bump_mapping__dotproduct.vp");
	nvparse(txt);
	nvparse_print_errors(stderr);
	delete [] txt;

	key('-', 0, 0); // initialize offset texture 2d matrix

}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);

	if('1' == k)
	{
		current_interactor->disable();
		current_interactor = & object;
		current_interactor->enable();
	}
	if('2' == k)
	{
		current_interactor->disable();
		current_interactor = & environment;
		current_interactor->enable();
	}

	if('w' == k)
	{
		if(b[k])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	if('9' == k)
		env_scale *= 1.2f;
	if('0' == k)
		env_scale /= 1.2f;
	if('7' == k)
		env_trans += .1f;
	if('8' == k)
		env_trans -= .1f;

	if('.' == k) // invert bump scale
	{
		bump_scale *= -1;
	}
	if('+' == k) // increase bump scale
	{
		bump_scale *= 1.2;
	}

	if('-' == k) // decrease bump scale
	{
		bump_scale /= 1.2;
	}

	// toggle combiners features
	if('d'==k || 's'==k || 't'==k || 'R'==k)
		set_up_shaders();

	if(' '==k)
	{
		//glutIdleFunc(b[k] ? idle : 0);
		glut_idle(b[k]);
	}

	glutPostRedisplay();
}

void idle()
{
	if(b['r'])
		object.trackball.increment_rotation();
	glutPostRedisplay();
}


void verify_shader_config()
{
	GLint consistent;

	glActiveTextureARB( GL_TEXTURE0_ARB );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, & consistent);
	if(consistent == GL_FALSE)
		cerr << "Shader stage 0 is inconsistent!" << endl;

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, & consistent);
	if(consistent == GL_FALSE)
		cerr << "Shader stage 1 is inconsistent!" << endl;

	glActiveTextureARB( GL_TEXTURE2_ARB );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, & consistent);
	if(consistent == GL_FALSE)
		cerr << "Shader stage 2 is inconsistent!" << endl;

	glActiveTextureARB( GL_TEXTURE3_ARB );
	glGetTexEnviv(GL_TEXTURE_SHADER_NV, GL_SHADER_CONSISTENT_NV, & consistent);
	if(consistent == GL_FALSE)
		cerr << "Shader stage 3 is inconsistent!" << endl;

	glActiveTextureARB( GL_TEXTURE0_ARB );
}


void set_up_shaders__offset_2D()
{
	
	vp_dsdt.bind();
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 94, bump_scale, 0, 0, 0);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_TEXTURE_SHADER_NV);
	

	if(b['t'])
	{
		ts_dsdt.call_list();
		rc_tex.call_list();
	}
	else
	{
		ts_dsdt_notex.call_list();
		rc_notex.call_list();
	}

	// stage 0 -- normal map
	glActiveTextureARB( GL_TEXTURE0_ARB );
	normalmap_dsdt.bind();

	// stage 1 -- decal
	glActiveTextureARB( GL_TEXTURE1_ARB );
	decal.bind();

	// stage 3 -- offset
	glActiveTextureARB( GL_TEXTURE3_ARB );
	cubeface.bind();
	float m[4] = {0, 0, 0, 0};
	m[0] = bump_scale;
	m[3] = bump_scale;
	glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_2D_MATRIX_NV, m);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	verify_shader_config();

	glActiveTextureARB( GL_TEXTURE0_ARB );

}


void set_up_shaders__dot_product_2D()
{
	
	vp_dotproduct.bind();
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 94, bump_scale, 0, 0, 0);
	glEnable(GL_TEXTURE_SHADER_NV);
	
	// stage 0 -- normal map
	glActiveTextureARB( GL_TEXTURE0_ARB );
	normalmap_rgb.bind();

	// stage 1 -- decal
	glActiveTextureARB( GL_TEXTURE1_ARB );
	decal.bind();

	// stage 3 -- offset
	glActiveTextureARB( GL_TEXTURE3_ARB );
	cubeface.bind();

	if(b['t'])
	{
		ts_dotproduct.call_list();
		rc_tex.call_list();
	}
	else
	{
		ts_dotproduct_notex.call_list();
		rc_notex.call_list();
	}

	verify_shader_config();

	glActiveTextureARB( GL_TEXTURE0_ARB );

}

void set_up_shaders()
{
	if(b['o'])
		set_up_shaders__offset_2D();
	else
		set_up_shaders__dot_product_2D();
}


vec3f sphere_position(float theta, float phi)
{
	vec3f p;
	p[0] =   cos(phi) * cos(theta);
	p[1] =   sin(phi);
	p[2] = - cos(phi) * sin(theta);
	return p;
}

vec3f sphere_tangent(float theta, float phi)
{
	vec3f t;
	t[0] = - sin(theta);
	t[1] =   0;
	t[2] = - cos(theta);
	return t;
}

vec3f sphere_binormal(float theta, float phi)
{
	vec3f b;
	b[0] = - sin(phi) * cos(theta);
	b[1] =   cos(phi);
	b[2] =   sin(phi) * sin(theta);
	return b;
}

vec3f sphere_normal(float theta, float phi)
{
	return sphere_position(theta, phi);
}

vec3f rotate_light_vector(vec3f l, float theta, float phi)
{
	vec3f l2;
	l2[0] = l.dot(sphere_tangent(theta, phi));
	l2[1] = l.dot(sphere_binormal(theta, phi));
	l2[2] = l.dot(sphere_normal(theta, phi));
	return l2;
}


// This is done in immediate mode for the sake of simplicity -- not performance.
void draw_sphere(vec3f light_direction)
{

	glVertexAttrib3fARB(1,.3f,.3f,.3f);

	
	int stacks = 30;
	int slices = 60;
	float pi = 3.1415927f;
	for(int i=0; i < stacks-1; i++)
	{
		float t = i/(stacks-1.f);
		float t2 = (i+1)/(stacks-1.f);
		float phi = pi*t - pi/2;
		float phi2 = pi*t2 - pi/2;

		glBegin(GL_QUAD_STRIP);
		for(int j=0; j < 60; j++)
		{
			float s = j/(slices-1.f);
			float theta = 2*pi*s;
	
			glVertexAttrib2fARB(8, s, t);
			glVertexAttrib3fvARB(3, sphere_tangent (theta, phi).v);
			glVertexAttrib3fvARB(4, sphere_binormal(theta, phi).v);
			glVertexAttrib3fvARB(5, sphere_normal  (theta, phi).v);
			glVertexAttrib3fvARB(0, sphere_position(theta, phi).v);

			glVertexAttrib2fARB(8, s, t2);
			glVertexAttrib3fvARB(3, sphere_tangent (theta, phi2).v);
			glVertexAttrib3fvARB(4, sphere_binormal(theta, phi2).v);
			glVertexAttrib3fvARB(5, sphere_normal  (theta, phi2).v);
			glVertexAttrib3fvARB(0, sphere_position(theta, phi2).v);

		}
		glEnd();
	}
}

void display()
{
	vec3f l(1,1,3);

	glEnable(GL_NORMALIZE);

	l.normalize();
	quaternionf q(vec3f(0,0,1), light_rotation);
	q.mult_vec(l);

	object.trackball.r.inverse().mult_vec(l); // rotate the light into the quad's object space

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MATRIX0_ARB);
	glLoadIdentity();
	glTranslatef(.5f, .5f, .5f);
	glTranslatef(env_trans, env_trans, env_trans);
	glScalef(.5f, .5f, .5f);
	glScalef(env_scale, env_scale, env_scale);
	environment.trackball.apply_inverse_transform();
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

	set_up_shaders();
	draw_sphere(l);

	glPopMatrix();

	glutSwapBuffers();
}
