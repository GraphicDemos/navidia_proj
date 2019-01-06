#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
							"GL_NV_register_combiners " \
							"GL_ARB_texture_cube_map "							

#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_array.h>
#include <glh/glh_obs.h>
#include <glh/glh_convenience.h>

#include <shared/array_texture.h>
#include <shared/load_cubemap.h>
#include <nv_png.h>

#include <shared/quitapp.h>

#include <nvparse.h>

using namespace std;
using namespace glh;
 

glut_perspective_reshaper reshaper;
glut_callbacks cb;
glut_simple_mouse_interactor obj, alight, * manip;

vec3f scale_tweak(1,1,1);

vec4f ambient(.1f, .1f, .1f, 1);
vec3f l_pos, l_vel;
rotationf l_rot;
matrix4f alight_xf;

rotationf stupid_rotation_thing(vec3f(1,0,0), 0);

tex_object_2D exp_tex, dst2_tex, nvlogo;
tex_object_cube_map cubemap;
display_list txquad;
display_list exp_config, dst_config;
bool b[256];

void key(unsigned char k, int x, int y);
void display();
void idle();

void initialize_textures();
void init_opengl();

void menu(int entry)
{
	key(entry,0,0);
}

int main(int argc, char **argv)
{

	l_vel = vec3f(.1f, -.323f, .221f);
	l_vel *= .02f;

	b['e'] = true; // begin with exponential attenuation
	b['r'] = true; // rotate ball
	b['t'] = true; // translate ball
	b['f'] = true; // anisotropic filtering

	glutInit(&argc, argv);
	glutInitWindowSize(512,512);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("Per-Pixel Attenuation Demo");

	init_opengl();

	//alight.trackball.r.set_value(vec3f(1,1,1), .707f);
	alight.trackball.incr = rotationf(vec3f(1,1,2), .01f);
	alight.set_parent_rotation(&stupid_rotation_thing);

	cb.keyboard_function = key;
	obj.dolly.dolly[2] = -1.5;
	reshaper.fovy = 60.f;

	obj.configure_buttons(1);
	alight.configure_buttons(1);

	glut_helpers_initialize();

	manip = &obj;

	glut_add_interactor(&cb);
	glut_add_interactor(&reshaper);
	glut_add_interactor(manip);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
//	glutIdleFunc(idle);

	int manips = glutCreateMenu(menu);
	glutAddMenuEntry("camera [1]", '1');
	glutAddMenuEntry("light  [2]", '2');

	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("perpixel_attenuation (c) 2000 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);
	glutAddMenuEntry("libpng -- Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson", 0);

	int tweakables = glutCreateMenu(menu);
	glutAddMenuEntry("increase/decrease attenuation [-/+]", 0);
	glutAddMenuEntry("increase/decrease translation speed [v/V]", 0);
	glutAddMenuEntry("increase/decrease ambient light [a/A]", 0);

	int toggles = glutCreateMenu(menu);
	glutAddMenuEntry("toggle animation [space]", ' ');
	glutAddMenuEntry("   toggle light rotation [r]", 'r');
	glutAddMenuEntry("   toggle light rotation [t]", 't');
	glutAddMenuEntry("toggle anisotropic light source (projective cube map) [c]", 'c');
	glutAddMenuEntry("toggle 2:1 anisotropic texture filtering [f]", 'f');

	int atten_modes = glutCreateMenu(menu);
	glutAddMenuEntry("e^(-dst^2) [e]", 'e');
	glutAddMenuEntry("1 - dst^2 [d]", 'd');

	glutCreateMenu(menu);
	glutAddSubMenu("attenuation technique", atten_modes);
	glutAddSubMenu("mouse manipulates...", manips);
	glutAddSubMenu("toggles", toggles);
	glutAddSubMenu("tweakables", tweakables);
	glutAddSubMenu("copyright info", copyrights);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// why is this required?
	// scale_tweak = vec3f(1,.64f,1);
	scale_tweak = vec3f(1,1,1);

	key(' ', 0, 0);

	glutMainLoop();
	return 0;
}

void eye_linear_texgen()
{
	matrix4f why; 

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	set_texgen_planes(GL_EYE_PLANE, why);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	glPopMatrix();
}

void enable_texgen()
{
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
}

void disable_texgen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
}

float txf = .3f;

void apply_texture_transforms(matrix4f pre_transforms = matrix4f())
{

	glMatrixMode(GL_TEXTURE);

	
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glLoadIdentity();
	glTranslatef(.5f, .5f, .5f);
	glScalef(scale_tweak[0], scale_tweak[1], scale_tweak[2]);
	glScalef(txf, txf, txf);
	if(b['d']) 
		glScalef(2.5f, 2.5f, 2.5f); // for distance^2 attenuation
	glMultMatrixf(pre_transforms.m);


	glActiveTextureARB(GL_TEXTURE1_ARB);
	glLoadIdentity();
	glTranslatef(.5f, .5f, .5f);
	glRotatef(90, 0, 1, 0);  // rotate to put r into s for this stage
	glScalef(scale_tweak[0], scale_tweak[1], scale_tweak[2]);
	glScalef(txf, txf, txf);
	if(b['d']) 
		glScalef(2.5f, 2.5f, 2.5f); // for distance^2 attenuation
	glMultMatrixf(pre_transforms.m);
	
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_MODELVIEW);

}

void set_attenuation_texgen()
{

		glActiveTextureARB(GL_TEXTURE0_ARB);
		eye_linear_texgen();
		enable_texgen();
		
		glActiveTextureARB(GL_TEXTURE1_ARB);
		eye_linear_texgen();
		enable_texgen();
		
		glActiveTextureARB(GL_TEXTURE0_ARB);
		apply_texture_transforms();
}


void init_opengl()
{
	initialize_textures();
	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Unable to initialize the following extensions:" << endl
		     << glh_get_unsupported_extensions() << endl;
		quitapp(0);
	}
	
	txquad.new_list(GL_COMPILE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-1, -1, 1);
	glTexCoord2f(0, 1);
	glVertex3f(-1,  1, 1);
	glTexCoord2f(1, 1);
	glVertex3f( 1,  1, 1);
	glTexCoord2f(1, 0);
	glVertex3f( 1, -1, 1);
	glEnd();
	txquad.end_list();

	glLineWidth(2);

	// cube map
	cubemap.bind();
	load_png_cubemap("dots_%s.png", true);
	cubemap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	cubemap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	cubemap.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);

	array2<vec3ub> img;
	nvlogo.bind();
	read_png_rgb("nvlogo_whitebg.png", img);
	make_rgb_texture(img, true);
	nvlogo.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	nvlogo.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	nvlogo.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);


	exp_config.new_list(GL_COMPILE);
	nvparse(
		"!!RC1.0                                   \n"
		"                                          \n"
		"# simple modulation                       \n"
		"                                          \n"
		"  { alpha { spare0 = tex0.b * tex1.b; } } \n"
		"  out.rgb = spare0.a;                     \n"
		);
	nvparse_print_errors(stderr);
	exp_config.end_list();

	dst_config.new_list(GL_COMPILE);
	nvparse(
		"!!RC1.0                                   \n"
		"                                          \n"
		"# simple dst2 attenuation                 \n"
		"                                          \n"
		"  {                                       \n"
		"    alpha {                               \n"
		"      discard = tex0.b;                   \n"
		"      discard = tex1.b;                   \n"
		"      spare0 = sum();                     \n"
		"    }                                     \n"
		"  }                                       \n"
		"  out.rgb = unsigned_invert(spare0.a);    \n"
		);
	nvparse_print_errors(stderr);
	dst_config.end_list();



}

void idle()
{
	if(b['t'])
	{
		l_pos += l_vel;
		for(int i=0; i < 3; i++)
		{
			if(l_pos[i] < -.45f || l_pos[i] > .45f)
			{
				l_pos[i] -= l_vel[i];
				l_vel[i] *= -1;
			}
		}
		alight.dolly.dolly = l_pos;
	}

	if(b['r'])
		alight.trackball.increment_rotation();
	glutPostRedisplay();
}


void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27) exit(0);
	if(k=='+') txf *= .8f;
	if(k=='-') txf *= 1.35f;

	if(k=='1')
	{
		glut_remove_interactor(manip);
		manip = &obj;
		glut_add_interactor(manip);
	}
	if(k=='2')
	{
		glut_remove_interactor(manip);
		manip = &alight;
		glut_add_interactor(manip);
	}
	if(k=='v') l_vel *= 1.3f;
	if(k=='V') l_vel *= .7f;

	if(k=='x') scale_tweak[0] *= 1/.8f;
	if(k=='y') scale_tweak[1] *= 1/.8f;
	if(k=='z') scale_tweak[2] *= 1/.8f;
	if(k=='X') scale_tweak[0] *= .8f;
	if(k=='Y') scale_tweak[1] *= .8f;
	if(k=='Z') scale_tweak[2] *= .8f;

	if('a'==k) ambient /= .8f;
	if('A'==k) ambient *= .8f;


	if('e'==k)
	{
		b['e'] = true;
		b['d'] = false;
	}
	if('d'==k)
	{
		b['d'] = true;
		b['e'] = false;
	}

	if('f'==k)
	{
		if(b[k])
		{
			nvlogo.bind();
			nvlogo.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);
			cubemap.bind();
			cubemap.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);
		}
		else
		{
			nvlogo.bind();
			nvlogo.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
			cubemap.bind();
			cubemap.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
		}
	}

	if(k==' ') glutIdleFunc(b[k]? idle : 0);

	glutPostRedisplay();
}

void configure_combiners_exp_attenuation()
{
	exp_config.call_list();
	glEnable(GL_REGISTER_COMBINERS_NV);
}


void configure_combiners_dst2_attenuation()
{
	dst_config.call_list();
	glEnable(GL_REGISTER_COMBINERS_NV);
}


void draw_cube()
{
	// draw the faces
	glPushMatrix();
	glScalef(.5f, .5f, .5f);
	
	txquad.call_list();    // front
	glRotatef(90, 0, 1, 0);
	txquad.call_list();    // right
	glRotatef(90, 0, 1, 0);
	txquad.call_list();    // back
	glRotatef(90, 0, 1, 0);
	txquad.call_list();    // left
	glRotatef(90, 0, 1, 0);
	glRotatef(90, 1, 0, 0);
	txquad.call_list();    // bottom
	glRotatef(180, 1, 0, 0);
	txquad.call_list();    // top
	
	glPopMatrix();
}

void display()
{
	stupid_rotation_thing = obj.trackball.r.inverse();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	glPushMatrix();
	obj.apply_transform();

	set_attenuation_texgen();
	apply_texture_transforms(
							 alight.get_inverse_transform()
							 *
							 obj.get_inverse_transform()
							 );

	// draw cube
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glColor3f(.8f, .8f, .8f);

	matrix4f TG0, TG1;

	matrix4f M = get_matrix(GL_MODELVIEW_MATRIX);
	M = M.transpose();
	glActiveTextureARB(GL_TEXTURE0_ARB);
	matrix4f T0 = get_matrix(GL_TEXTURE_MATRIX);
	T0 = T0.transpose();
	glGetTexGenfv(GL_S, GL_EYE_PLANE, & TG0(0,0));
	glGetTexGenfv(GL_T, GL_EYE_PLANE, & TG0(1,0));
	glGetTexGenfv(GL_R, GL_EYE_PLANE, & TG0(2,0));
	glGetTexGenfv(GL_Q, GL_EYE_PLANE, & TG0(3,0));
	glActiveTextureARB(GL_TEXTURE1_ARB);
	matrix4f T1 = get_matrix(GL_TEXTURE_MATRIX);
	T1 = T1.transpose();
	glGetTexGenfv(GL_S, GL_EYE_PLANE, & TG1(0,0));
	glGetTexGenfv(GL_T, GL_EYE_PLANE, & TG1(1,0));
	glGetTexGenfv(GL_R, GL_EYE_PLANE, & TG1(2,0));
	glGetTexGenfv(GL_Q, GL_EYE_PLANE, & TG1(3,0));
	glActiveTextureARB(GL_TEXTURE0_ARB);

	if(b['e']) // exponential attenuation
	{
		glActiveTextureARB(GL_TEXTURE0_ARB);
		exp_tex.bind();
		exp_tex.enable();
		glActiveTextureARB(GL_TEXTURE1_ARB);
		exp_tex.bind();
		exp_tex.enable();			
		configure_combiners_exp_attenuation();

		draw_cube(); ///////////////////////  attenuation pass

		glDisable(GL_REGISTER_COMBINERS_NV);
		exp_tex.disable();
		glActiveTextureARB(GL_TEXTURE0_ARB);
		exp_tex.disable();
	}
	else if(b['d']) // distance^2 attenuation
	{
		glActiveTextureARB(GL_TEXTURE0_ARB);
		dst2_tex.bind();
		dst2_tex.enable();
		glActiveTextureARB(GL_TEXTURE1_ARB);
		dst2_tex.bind();
		dst2_tex.enable();
		configure_combiners_dst2_attenuation();

		draw_cube(); //////////////////////// attenuation pass

		glDisable(GL_REGISTER_COMBINERS_NV);
		dst2_tex.disable();
		glActiveTextureARB(GL_TEXTURE0_ARB);
		dst2_tex.disable();
	}

	disable_texgen();

	// now apply colors
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glEnable(GL_BLEND);
	glDepthMask(0);
	glDepthFunc(GL_EQUAL);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	nvlogo.bind();
	nvlogo.enable();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(-1,1,1);
	glMatrixMode(GL_MODELVIEW);

	if(b['c']) // anisotropic light source cubemap
	{

		glActiveTextureARB(GL_TEXTURE1_ARB);
		eye_linear_texgen();
		enable_texgen();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		alight.apply_inverse_transform();
		obj.apply_inverse_transform();
		glMatrixMode(GL_MODELVIEW);

		cubemap.bind();
		cubemap.enable();
	}

	draw_cube(); ////////////////////////// decal w/ anisotropic light

	if(b['c'])
	{
		cubemap.disable();
	}

	
	glBlendFunc(GL_ONE, GL_ONE);
	
	glColor4fv(ambient.v);
	
	draw_cube(); ///////////////////////// ambient pass
	
	
	glColor3f(1,1,1);	
	
	glActiveTextureARB(GL_TEXTURE0_ARB);
	nvlogo.disable();

	
	glDepthMask(1);
	glDepthFunc(GL_LESS);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glPopMatrix();


	// draw the light source
	glPushMatrix();
	obj.apply_transform();
	alight.apply_transform();
	if(b['c']) // anisotropic light source cubemap
	{

		glActiveTextureARB(GL_TEXTURE0_ARB);
		eye_linear_texgen();
		enable_texgen();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		alight.apply_inverse_transform();
		obj.apply_inverse_transform();
		glMatrixMode(GL_MODELVIEW);

		cubemap.bind();
		cubemap.enable();
	}

	glScalef(.3f/(txf*scale_tweak[0]),
		     .3f/(txf*scale_tweak[1]),
			 .3f/(txf*scale_tweak[2]));
	glutSolidSphere(.05, 20, 20); //////////////////////////

	if(b['c'])
	{
		cubemap.disable();
	}
	glColor3f(1,1,1);

	glPopMatrix();

	glutSwapBuffers();
}




// initialize exp(-x^2)exp(-y^2)  and dst^2 texture

void initialize_textures()
{
	int sz = 64;
	float fsz(sz);
	float range = 2.5f;
	array2<float> e(sz, sz);
	array2<float> d(sz, sz);

	{
		for(int j=0; j < sz; j++)
		{
			float fj = -1.f + (2.f*j+1.f)/fsz;
			float efj = fj * range;
			efj = exp(-(efj*efj));

			for(int i=0; i < sz; i++)
			{
				float fi = -1.f + (2.f*i+1.f)/fsz;
				float efi = fi * range;
				efi = exp(-(efi*efi));
				e(i,j) = efi*efj;
				float dst2 = fi * fi + fj * fj;
				if(dst2 > 1) dst2 = 1;
				d(i,j) = dst2;
			}
		}
	}

	exp_tex.bind();
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE8, sz, sz,
					  GL_LUMINANCE, GL_FLOAT, e.get_pointer());
	exp_tex.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	exp_tex.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	exp_tex.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	dst2_tex.bind();
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE8, sz, sz,
					  GL_LUMINANCE, GL_FLOAT, d.get_pointer());
	dst2_tex.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	dst2_tex.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	dst2_tex.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}
