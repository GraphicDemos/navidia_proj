/*
  This is a demonstration program that illustrates a number of NVIDIA
  extensions working together to produce a environment mapped bump
  mapped higher-order surface.

  Cass Everitt
  8-30-00
*/

#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif


#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_cube_map.h>
#include <glh/glh_mipmaps.h>
#include <glh/glh_glut_replay.h>
#include "../nvparse/include/nvparse.h"

#include <shared/array_texture.h>
#include "nv_png.h"
#include <shared/bumpmap_to_normalmap.h>
#include <shared/load_cubemap.h>
#include <shared/read_text_file.h>
#include <shared/cubemap_borders.h>
#include <shared/quitapp.h>
#include "bump_teapot.h"
#include "dynamic_normalmap.h"

#if defined(UNIX) || defined(WIN32)
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_ARB_vertex_program " \
                            "GL_EXT_blend_minmax " \
                            "GL_EXT_blend_subtract " \
                            "GL_NV_register_combiners " \
                            "GL_NV_register_combiners2 " \
                            "GL_NV_texture_shader "
#else
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
                            "GL_ARB_vertex_program " \
                            "GL_EXT_blend_minmax " \
                            "GL_EXT_blend_subtract " \
                            "GL_NV_register_combiners " \
                            "GL_NV_register_combiners2 " \
                            "GL_NV_texture_shader "
#endif

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object, cubemap_xform;
glut_perspective_reshaper reshaper;
glut_replay replay;

bool initialized = false;

bool b[256];
bool first_pass;

// for Mark Kilgard :-)
struct lazy_load_cubemap
{
public:
	lazy_load_cubemap() {}

	lazy_load_cubemap(string file_spec, bool build_mipmaps)
	{
		filespec = file_spec;
		mipmap = build_mipmaps;
		loaded = false;
	}

	void bind()
	{
		if(! loaded)
		{
		    cubemap.bind();
            cubemap.parameter(GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			cubemap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cubemap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cubemap.parameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			cubemap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			load_png_cubemap(filespec.c_str(), false);

            bordered_cubemap.bind();
            bordered_cubemap.parameter(GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
			bordered_cubemap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
			bordered_cubemap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
			bordered_cubemap.parameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			bordered_cubemap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            load_png_cubemap(filespec.c_str(), mipmap);
            cubemap_borders();
			loaded = true;
		}
        if(b['8'])
            bordered_cubemap.bind();
        else
            cubemap.bind();
	}
	string filespec;
	tex_object_cube_map cubemap, bordered_cubemap;
	bool loaded;
	bool mipmap;
};


// environment map
lazy_load_cubemap cubemap[6], * current_cubemap;
tex_object_cube_map renorm;
int num_cubemaps = 2;
int cubemap_index = 0;

float control_point_time = 0.f;
float control_point_time_incr = .01f;
float control_point_scale = 1.f;

float bumpscale_time = 0.f;
float bumpscale_time_incr = 0.05f;

float index_ratio = .8f;
vec4f refract_and_bump_scale(0,0,0,.2f);
rotationf parent;

// normal map
tex_object_2D normalmap, dynamic_normalmap, *current_normalmap;
tex_object_2D glossmap;
display_list gloss_config, reflect_config, refract_config;


arb_vertex_program vp[5];

// these are the z values of the internal
// 4 control points... the letters are chosen
// because of their layout on the keyboard
GLfloat y, u, h = 5 * 0, j;
GLfloat tess[4] = { 30, 30, 30, 30 };

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void menu(int entry) { key((unsigned char)entry, 0, 0); }

// my functions
void init_opengl();
void set_refract_and_bump_scale();
void idle();

extern "C"
bool initialize_mode(int m)
{
	b['w'] = false; // 
	b['s'] = true;  // shiny
	b['g'] = true;  // glossmap
	b['m'] = true;  // mipmap normal map
	b['P'] = true;  // animate patch rotation during idle
	b['E'] = true;  // animate environment rotation during idle
	b['C'] = true;  // do animate patch control points
	b['B'] = false; // don't animate bump scale in texel matrix
	b['r'] = true;  // show environment cube map
	b['D'] = true;  // animate dynamic normal map if it's being drawn...

	switch(m)
	{
	case 0:
		break;

		// refractive
	case 1:
		b['s'] = false;
		break;

		// dynamic normalmap (reflective)
	case 2:
		b['d'] = true;
		break;

	case 3:
		b['d'] = true;
		b['s'] = false;
		break;

	default:
		break;
	}
	initialized = true;
	return true;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowSize(512, 512);
	glutCreateWindow("fairly simple OpenGL evaluators, ARB_vertex_program, NV_texture_shader example");

	if(!initialized) initialize_mode(0);

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = key;
	cb.idle_function = idle;
	cb.display_function = display;

	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -3;
	object.trackball.incr = rotationf(vec3f(.1, .2, 1), to_radians(.01));
	cubemap_xform.configure_buttons(1);
	cubemap_xform.set_parent_rotation(& parent);
	cubemap_xform.trackball.scale *= 0.2f;
	cubemap_xform.trackball.incr = rotationf(vec3f(0, 1, 0), to_radians(.05));


	object.enable();
	camera.disable();
	cubemap_xform.disable();

	glut_add_interactor(&replay);
	glut_add_interactor(&cb);
	glut_add_interactor(&camera);
	glut_add_interactor(&object);
	glut_add_interactor(&cubemap_xform);
	glut_add_interactor(&reshaper);

	int mouse_moves = glutCreateMenu(menu);
	glutAddMenuEntry("patch [p]", 'p');
	glutAddMenuEntry("environment [e]", 'e');

	int toggles = glutCreateMenu(menu);
	glutAddMenuEntry("glossmap [g]", 'g');
	glutAddMenuEntry("mipmap normalmap [m]", 'm');
	glutAddMenuEntry("cube map borders [8]", '8');
	glutAddMenuEntry("wireframe [w]", 'w');
	glutAddMenuEntry("skybox [r]", 'r');
	glutAddMenuEntry("reflect/refract [s]", 's');
	glutAddMenuEntry("static/dynamic normalmap [d]", 'd');
	glutAddMenuEntry("** show rendered dynamic normalmap [^]", '^');
	glutAddMenuEntry("animate [ ]", ' ');
	glutAddMenuEntry("  animate control points [C]", 'C');
	glutAddMenuEntry("  animate dynamic normalmap [D]", 'D');
	glutAddMenuEntry("  animate bump scale [B]", 'B');
	glutAddMenuEntry("  animate patch rotation [P]", 'P');
	glutAddMenuEntry("  animate environment rotation [E]", 'E');


	int tweak = glutCreateMenu(menu);
	glutAddMenuEntry("use [+] to increase bump scale", '+');
	glutAddMenuEntry("use [-] to decrease bump scale", '-');
	glutAddMenuEntry("'/' and '*' to change the index of refraction", 0);
	glutAddMenuEntry("'[' and ']' to slow down or speed up bump scale animation", 0);
	glutAddMenuEntry("'{' and '}' to slow down or speed up control point animation", 0);
	glutAddMenuEntry("'(' and ')' to decrease or increase control point displacement during animation", 0);
	glutAddMenuEntry("y,u,h,j small positive z bias of inner control points",0);
	glutAddMenuEntry("Y,U,H,J small negative z bias of inner control points",0);

	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("bumpy_shiny_patch (c) 2000 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);

	glutCreateMenu(menu);
	glutAddSubMenu("mouse moves...", mouse_moves);
	glutAddSubMenu("toggles", toggles);
	glutAddSubMenu("tweakables", tweak);
	glutAddSubMenu("copyright info", copyrights);
	glutAddMenuEntry("--------------------------------", 0);
	glutAddMenuEntry("next cubemap [c]", 'c');
	glutAddMenuEntry("flatten patch [z]", 'z');
	glutAddMenuEntry("quit [esc]", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	key(' ', 0, 0); // start animating

	glutMainLoop();
	return 0;
}

void get_error()
{
    int err;

    err = glGetError();
    char *str;

    str = (char *)gluErrorString(err);
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
	build_2D_mipmaps(GL_TEXTURE_2D, GL_SIGNED_HILO_NV, imgsize, imgsize, GL_HILO_NV,
					 generic_filter<GLshort>(2, GL_SHORT), simg);
	normalmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	normalmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	normalmap.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	normalmap.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	delete [] simg;
}



void init_opengl()
{
	glEnable(GL_DEPTH_TEST);

	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
        quitapp(0);
	}

	GLfloat cc = 0.f;
	glClearColor(cc, cc, cc, 1);

	vector<string> cubes;
	cubes.push_back("nvlobby_new_%s.png");
	cubes.push_back("nvlobby_%s.png");
	cubes.push_back("cube_face_%s.png");

    // cube maps
	for(unsigned int i=0; i < cubes.size(); i++)
		cubemap[i] = lazy_load_cubemap(cubes[i], true);

	current_cubemap = & cubemap[cubemap_index];
	num_cubemaps = cubes.size();

	renorm.bind();
	normalize_vector tmp;
	make_cube_map(tmp, GL_RGB8, 256);
	renorm.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	array2<unsigned char> gimg;
	read_png_grey("bump_map.png", gimg);
	
	array2<vec3f> normals;
	bumpmap_to_normalmap(gimg, normals);

	make_normalmap_texture(normals);

	update_normalmap_bulges(dynamic_normalmap);

	if(b['d'])
		current_normalmap = & dynamic_normalmap;
	else
		current_normalmap = & normalmap;

	reflect_config.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                                                      \n"
		"# reflection shader config                                   \n"
		"texture_2d();                                                \n"
		"dot_product_reflect_cube_map_eye_from_qs_1of3(expand(tex0)); \n"
		"dot_product_reflect_cube_map_eye_from_qs_2of3(expand(tex0)); \n"
		"dot_product_reflect_cube_map_eye_from_qs_3of3(expand(tex0)); \n");
	nvparse_print_errors(stderr);
	reflect_config.end_list();

	refract_config.new_list(GL_COMPILE);
	nvparse(
		"!!TS1.0                                  \n"
		"# refraction shader config               \n"
		"texture_2d();                            \n"
		"dot_product_cube_map_1of3(expand(tex0)); \n"
		"dot_product_cube_map_2of3(expand(tex0)); \n"
		"dot_product_cube_map_3of3(expand(tex0)); \n");
	nvparse_print_errors(stderr);
	refract_config.end_list();

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

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 4, 1, 2, 0, -1);
	set_refract_and_bump_scale();

	char * txt;
	vp[0].bind();
	txt = read_text_file("bumpy_shiny_patch/bumpy_shiny_patch__reflect.vp");
	nvparse(txt);
	nvparse_print_errors(stderr);
	delete [] txt;

	vp[1].bind();
	txt = read_text_file("bumpy_shiny_patch/bumpy_shiny_patch__gloss.vp");
	nvparse(txt);
	nvparse_print_errors(stderr);
	delete [] txt;

	vp[2].bind();
	txt = read_text_file("bumpy_shiny_patch/bumpy_shiny_patch__refract.vp");
	nvparse(txt);
	nvparse_print_errors(stderr);
	delete [] txt;


	array2<vec3ub> img;
	glossmap.bind();
	read_png_rgb("rgb_gloss_map.png", img);
	make_rgb_texture(img, true);
	glossmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glossmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}


void idle(void)
{
	if(b['C'])
	{
		control_point_time += control_point_time_incr;
		
		y = control_point_scale * 2.0 * sin(control_point_time * 3.0);
		u = control_point_scale * 2.1 * sin(control_point_time * 3.2);
		h = control_point_scale * 1.3 * sin(control_point_time * 3.5);
		j = control_point_scale * 3.3 * sin(control_point_time * 2.3);
	}

	if(b['B'])
	{
		bumpscale_time += bumpscale_time_incr;
		refract_and_bump_scale[3] = sin(bumpscale_time);
		set_refract_and_bump_scale();
	}

	if(b['D'] && (b['d'] || b['^']))
		update_normalmap_bulges(dynamic_normalmap);

	if(b['E'])
		cubemap_xform.trackball.increment_rotation();
	if(b['P'])
		object.trackball.increment_rotation();


	glutPostRedisplay();
}

void set_refract_and_bump_scale()
{
	refract_and_bump_scale[0] = 1 - index_ratio;
	refract_and_bump_scale[1] = 1 - index_ratio;
	refract_and_bump_scale[2] = 1;

	vec4f v = refract_and_bump_scale;

	vp[0].bind();
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 5, v[0], v[1], v[2], v[3]);
}

void key(unsigned char k, int mouse_x, int mouse_y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q')
	{
		vp[0].del();
		exit(0);
	}

	if('+' == k || '=' == k)
	{
		refract_and_bump_scale[3] += .05f;
		set_refract_and_bump_scale();
	}
	if('-' == k)
	{
		refract_and_bump_scale[3] -= .05f;
		set_refract_and_bump_scale();
	}

	if('/' == k)
	{
		index_ratio -= .0025f;
		if(index_ratio < 0) index_ratio = 0;
		set_refract_and_bump_scale();
	}

	if('*' == k)
	{
		index_ratio += .0025f;
		if(index_ratio > 1) index_ratio = 1;
		set_refract_and_bump_scale();
	}

	if('d' == k)
	{
		if(b[k])
			current_normalmap = & dynamic_normalmap;
		else
			current_normalmap = & normalmap;
	}

	// slow down or speed up bump scale animation
	if('[' == k)
		bumpscale_time_incr *= .85f;
	if(']' == k)
		bumpscale_time_incr /= .85f;

	// slow down or speed up control point animation
	if('{' == k)
		control_point_time_incr *= .85f;
	if('}' == k)
		control_point_time_incr /= .85f;

	// decrease/increase control point scale during animation
	if('(' == k)
		control_point_scale *= .85f;
	if(')' == k)
		control_point_scale /= .85f;


	if('c' == k)
	{
		cubemap_index++;
		cubemap_index %= num_cubemaps;
		current_cubemap = & cubemap[cubemap_index];
	}


	if('m'==k)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB );
		if(b[k])
			current_normalmap->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			current_normalmap->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	}

	if('y'==k)
		y += .1f;
	if('Y'==k)
		y -= .1f;
	if('u'==k)
		u += .1f;
	if('U'==k)
		u -= .1f;
	if('h'==k)
		h += .1f;
	if('H'==k)
		h -= .1f;
	if('j'==k)
		j += .1f;
	if('J'==k)
		j -= .1f;


	if('1'==k)
		tess[0] += .1f;
	if('!'==k)
		tess[0] -= .1f;
	if('2'==k)
		tess[1] += .1f;
	if('@'==k)
		tess[1] -= .1f;
	if('3'==k)
		tess[2] += .1f;
	if('#'==k)
		tess[2] -= .1f;
	if('4'==k)
		tess[3] += .1f;
	if('$'==k)
		tess[3] -= .1f;

	if('p' == k)
	{
		object.enable();
		camera.disable();
		cubemap_xform.disable();
	}
	if('e' == k)
	{
		object.disable();
		camera.disable();
		cubemap_xform.enable();
	}

	if('E' == k)
	{
		if(b[k])
			cubemap_xform.trackball.scale /= 10;
		else
			cubemap_xform.trackball.scale *= 10;
	}

	if('z' == k)
	{
		h = j = y = u = 0;
		control_point_time = 0;
		bumpscale_time = 0;
	}

	if(' '==k)
		glut_idle(b[k]);		

	if('n' == k)
		glutSetWindowTitle("The Whole Enchilada");
        
	glutPostRedisplay();
}

// OpenGL evaluators stuff 
void eval_maps()
{
    glMapGrid2d(20, 0.0, 1.0, 20, 0.0, 1.0);
    glEvalMesh2(GL_FILL, 0, 20, 0, 20);
}

struct patch_indexer
{
	patch_indexer(int uorder, int vorder, GLfloat *ptr=0) :
	 u_order(uorder), v_order(vorder), p(ptr) {}
	GLfloat & operator () (int i, int j, int component)
	{ return p[4*u_order*j + 4*i + component]; }
	void set(int i, int j, vec4f v)
	{
		int index = 4*u_order*j + 4*i;
		p[index++] = v[0];
		p[index++] = v[1];
		p[index++] = v[2];
		p[index++] = v[3];
	}
	vec4f get(int i, int j)
	{ return vec4f(p + 4*u_order*j + 4*i); }

	int u_order, v_order;
	GLfloat *p;
};


void build_maps()
{
	// position map
	GLfloat patch_position[64];
	patch_indexer p(4, 4, patch_position);

	{
		for(int i=0; i <4; i++)
			for(int j=0; j < 4; j++)
				p.set(i,j, vec4f(i - 1.5f, j - 1.5f, 0, 1));
	}

	p(1,2,2) = y;   p(2,2,2) = u;
	p(1,1,2) = h;   p(2,1,2) = j;

    glMap2f(GL_MAP2_VERTEX_4, 0, 1, 4, 4, 0, 1, 16, 4, patch_position);

	// dp/du map
	GLfloat patch_dpdu[48];
	patch_indexer du(3, 4, patch_dpdu);

	{
		for(int i=0; i < 3; i++)
			for(int j=0; j < 4; j++)
			{
				du.set( i, j, p.get(i+1,j) - p.get(i,j));
			}
	}

    glMap2f(GL_MAP2_TEXTURE_COORD_4, 0, 1, 4, 3, 0, 1, 12, 4, patch_dpdu);

	// dp/dv map
	GLfloat patch_dpdv[48];
	patch_indexer dv(4, 3, patch_dpdv);

	{
		for(int i=0; i < 4; i++)
			for(int j=0; j < 3; j++)
			{
				dv.set( i, j, p.get(i,j+1) - p.get(i,j));
			}
	}

    glMap2f(GL_MAP2_NORMAL, 0, 1, 4, 4, 0, 1, 16, 3, patch_dpdv);

	// simple texture coordinate map
	GLfloat patch_uv[16];
	patch_indexer uv(2,2, patch_uv);
	uv.set(0,0,vec4f(0,0,0,1));
	uv.set(0,1,vec4f(0,1,0,1));
	uv.set(1,0,vec4f(1,0,0,1));
	uv.set(1,1,vec4f(1,1,0,1));

    glMap2f(GL_MAP2_COLOR_4, 0, 1, 4, 2, 0, 1, 8, 2, patch_uv);
    
    get_error();
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

void draw_shiny_patch()
{
	vp[0].bind();

    // set up the program parameters for the cubemap xform
	matrix4f mvc =   cubemap_xform.trackball.get_transform()
		           * camera.trackball.get_inverse_transform()
				   * object.trackball.get_transform();
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 20, mvc(0,0), mvc(0,1), mvc(0,2), 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 21, mvc(1,0), mvc(1,1), mvc(1,2), 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 22, mvc(2,0), mvc(2,1), mvc(2,2), 0);

	matrix4f invertscale;
	invertscale *= -1;
	
	matrix4f mvc4 =   cubemap_xform.trackball.get_transform()
					* invertscale
		            * camera.get_inverse_transform()
				    * object.get_transform();

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 43, mvc4(0,0), mvc4(0,1), mvc4(0,2), mvc4(0,3));
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 44, mvc4(1,0), mvc4(1,1), mvc4(1,2), mvc4(1,3));
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 45, mvc4(2,0), mvc4(2,1), mvc4(2,2), mvc4(2,3));

	glEnable(GL_VERTEX_PROGRAM_ARB);

	glEnable(GL_TEXTURE_SHADER_NV);
	
	glActiveTextureARB( GL_TEXTURE0_ARB );
	current_normalmap->bind();

	// stage 3 -- dot product, cube map lookup
	glActiveTextureARB( GL_TEXTURE3_ARB );
	current_cubemap->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		

	glActiveTextureARB( GL_TEXTURE0_ARB );

	reflect_config.call_list();



	verify_shader_config();
	if (b['T'])
	    render_teapot();
	else
        eval_maps();

    get_error();

	glDisable(GL_TEXTURE_SHADER_NV);

	glDisable(GL_VERTEX_PROGRAM_ARB);

	first_pass = false;
}

void draw_refractive_patch()
{
	vp[2].bind();
	
	// set up the program parameters for the cubemap xform
	matrix4f cmat =   cubemap_xform.trackball.get_transform();
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 23, cmat(0,0), cmat(0,1), cmat(0,2), 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 24, cmat(1,0), cmat(1,1), cmat(1,2), 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 25, cmat(2,0), cmat(2,1), cmat(2,2), 0);

	glEnable(GL_VERTEX_PROGRAM_ARB);

	glEnable(GL_TEXTURE_SHADER_NV);
	
	// stage 0 -- normal map
	glActiveTextureARB( GL_TEXTURE0_ARB );
	current_normalmap->bind();

	// stage 3 -- dot product, cube map lookup
	glActiveTextureARB( GL_TEXTURE3_ARB );
	current_cubemap->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		

	glActiveTextureARB( GL_TEXTURE0_ARB );

	refract_config.call_list();

	verify_shader_config();
	if (b['T'])
	    render_teapot();
	else
        eval_maps();

	glDisable(GL_TEXTURE_SHADER_NV);

	glDisable(GL_VERTEX_PROGRAM_ARB);

	first_pass = false;
}

void draw_glossmapped_patch()
{
	vp[1].bind();
	glEnable(GL_VERTEX_PROGRAM_ARB);

	
	glEnable(GL_TEXTURE_SHADER_NV);
	
	glActiveTextureARB( GL_TEXTURE3_ARB );
	glossmap.bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		
	glActiveTextureARB( GL_TEXTURE0_ARB );

	gloss_config.call_list();

	if(! first_pass)
	{
		glDepthFunc(GL_EQUAL);
		glDepthMask(0);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glEnable(GL_BLEND);
	}

	if(b['T'])
	    render_teapot();
	else
        eval_maps();

	if(! first_pass)
	{
		glDepthFunc(GL_LESS);
		glDepthMask(1);
		glDisable(GL_BLEND);
	}

	glDisable(GL_TEXTURE_SHADER_NV);

	glDisable(GL_VERTEX_PROGRAM_ARB);

	first_pass = false;
}


void draw_environment_cube_map()
{

	glPushMatrix();
	glLoadIdentity();
	matrix4f ident;
	set_texgen_planes(GL_EYE_PLANE, ident);
	glPopMatrix();
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	cubemap_xform.trackball.apply_transform();

	current_cubemap->bind();
	current_cubemap->cubemap.enable();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	//glutSolidSphere(10.f, 40, 40);
	glutSolidCube(10);

	current_cubemap->cubemap.disable();

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);

}


void display()
{

	// do playback if playing...
	if(replay.playing())
		replay.dispatch_accumulated_events();

	parent = cubemap_xform.trackball.r * object.trackball.r;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

	build_maps();

	first_pass = true;

	if(b['w'])
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_MAP2_VERTEX_4);
    glEnable(GL_MAP2_COLOR_4);
    glEnable(GL_MAP2_NORMAL);
    glEnable(GL_MAP2_TEXTURE_COORD_4);

	if(b['s'])
		draw_shiny_patch();
	else
		draw_refractive_patch();

	if(b['g'])
		draw_glossmapped_patch();

    glDisable(GL_MAP2_VERTEX_4);
    glDisable(GL_MAP2_COLOR_4);
    glDisable(GL_MAP2_NORMAL);
    glDisable(GL_MAP2_TEXTURE_COORD_4);

    glPopMatrix();

	if(b['r'])
		draw_environment_cube_map();

	if(b['d'] && b['^'])
		update_normalmap_bulges(dynamic_normalmap);

    get_error();

	glutSwapBuffers();
}
