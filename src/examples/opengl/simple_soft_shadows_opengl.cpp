/**
 * simple_soft_shadows.cpp
 *
 * This sample demonstrates how one can use dynamic branching in fragment programs
 * to accelerate soft shadow rendering. Branching allows us to selectively oversample shadowmap, taking
 * high number of samples only where necessary (in penumbra) and provides significant performance boost
 * in comparison to oversampling across entire shadowmap.
 * Additionaly, jittered sampling is exemplified, to get rid of banding artifacts.
 *
 * Written by Yury Uralsky (yuralsky@nvidia.com)
 * Copyright (c) 2004 by NVIDIA Corporation. All rights reserved.
 */

#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif

#include <iostream>

#if defined(UNIX)
#define GL_GLEXT_PROTOTYPES
#endif

#define GLH_EXT_SINGLE_FILE

#if defined(WIN32)
#define REQUIRED_EXTENSIONS "GL_VERSION_1_2 " \
							"GL_ARB_multitexture " \
							"GL_ARB_fragment_program " \
							"GL_ARB_vertex_program " \
							"GL_ARB_depth_texture " \
							"GL_ARB_shadow " \
							"WGL_ARB_pbuffer " \
							"WGL_ARB_pixel_format "
#elif defined(UNIX)
#define REQUIRED_EXTENSIONS "GL_VERSION_1_2 " \
							"GL_ARB_multitexture " \
							"GL_ARB_fragment_program " \
							"GL_ARB_vertex_program " \
							"GL_ARB_depth_texture " \
							"GL_ARB_shadow " \
							"GLX_SGIX_pbuffer " \
							"GLX_SGIX_fbconfig "
#endif

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <shared/pbuffer.h>
#include <shared/array_texture.h>
#include <nv_png.h>
#include <shared/read_text_file.h>

using namespace std;
using namespace glh;

#define	FP_64TAPS_NV4X				1
#define	FP_32TAPS_NV4X				2
#define	FP_64TAPS_NV3X				3
#define	FP_32TAPS_NV3X				4

#define FP_64TAPS_SHOW_PENUMBRA		5
#define	FP_32TAPS_SHOW_PENUMBRA		6

#define	VP_NAME						7

#define TEX_SIZE			1024
#define	JITTER_SIZE			16

bool nv4x_supported = false;
int mode_nv3x = 1;
int mode_32 = 0;
int mode_show_penumbra = 0;

int jitter = 1;

int frames = 0;

float fps, softness = 8.0f;

glut_callbacks cb;
glut_perspective_reshaper reshaper, lightshaper;
glut_simple_mouse_interactor camera, object, spotlight, *current_interactor;

tex_object_2D light_image, decal, light_view_depth;
tex_object_3D jitter_lookup_64, jitter_lookup_32;

display_list quad, geometry;

// Create pbuffer for shadowmap
PBuffer pbuffer("rgb depth=24");

GLenum depth_format;

void key(unsigned char k, int x, int y)
{
	if (k == 27) exit(0);
	if (k == '1' || k == '2' || k == '3')
    {
		object.disable();
		camera.disable();
		spotlight.disable();

		if (k == '1') camera.enable();
		else if(k == '2') object.enable();
		else spotlight.enable();
	}
	if (k == 'b' && nv4x_supported) {
		mode_nv3x ^= 1;
	}
	if (k == 's') {
		mode_32 ^= 1;
	}
	if (k == 'p' && nv4x_supported) {
		mode_show_penumbra ^= 1;
		mode_nv3x = 0;
	}
	if (k == '+' && softness < 24.0f) {
		softness += 1.0f;
	}
	if (k == '-' && softness > 0) {
		softness -= 1.0f;
	}
	if (k == 'j') {
		jitter ^= 1;
	}
	glutPostRedisplay();
}

void menu(int entry)
{
	key(entry, 0, 0);
}

void draw_text()
{
	int ww = glutGet( (GLenum)GLUT_WINDOW_WIDTH );
	int wh = glutGet( (GLenum)GLUT_WINDOW_HEIGHT );

	glDisable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, ww - 1, 0, wh - 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1, 1, 1);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDisable(GL_VERTEX_PROGRAM_ARB);

	glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2_ARB);
	glDisable(GL_TEXTURE_3D);
	glActiveTexture(GL_TEXTURE3_ARB);
	glDisable(GL_TEXTURE_2D);

	int x = 20;
	int y = wh-22;
	glRasterPos2i(x, y);

	char string[] = "fps = %.1f mode = %s\nsamples = %d/%d (estimation/total)\njitter = %s fwidth = %.1f";
	char tmp[1024];

	sprintf(tmp, string, fps, mode_nv3x ? "NV3X (no branching)" : "NV4X (branching)", mode_32 ? 4 : 8, mode_32 ? 32 : 64, jitter == 1 ? "yes" : "no", softness);
	                        
	char *p;

	for (p = tmp; *p; p++) {
		if (*p == '\n') {
			y = y - 14;
			glRasterPos2i( x, y );
			continue;
		}
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
	}

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}

// Create 3D texture for per-pixel jittered offset lookup
void create_jitter_lookup(tex_object_3D& jitter_lookup, int size, int samples_u, int samples_v)
{
	jitter_lookup.bind();
	jitter_lookup.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	jitter_lookup.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	jitter_lookup.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	jitter_lookup.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	jitter_lookup.parameter(GL_TEXTURE_WRAP_R, GL_REPEAT);

	signed char * data = new signed char[size * size * samples_u * samples_v * 4 / 2];

	for (int i = 0; i<size; i++) {
		for (int j = 0; j<size; j++) {
			for (int k = 0; k<samples_u*samples_v/2; k++) {

				int x, y;
				vec4f v;

				x = k % (samples_u / 2);
				y = (samples_v - 1) - k / (samples_u / 2);

				// generate points on a regular samples_u x samples_v rectangular grid
				v[0] = (float)(x * 2 + 0.5f) / samples_u;
				v[1] = (float)(y + 0.5f) / samples_v;
				v[2] = (float)(x * 2 + 1 + 0.5f) / samples_u;
				v[3] = v[1];
				
				// jitter position
				v[0] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_u);
				v[1] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_v);
				v[2] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_u);
				v[3] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_v);

				// warp to disk
				vec4f d;
				d[0] = sqrtf(v[1]) * cosf(2 * 3.1415926f * v[0]);
				d[1] = sqrtf(v[1]) * sinf(2 * 3.1415926f * v[0]);
				d[2] = sqrtf(v[3]) * cosf(2 * 3.1415926f * v[2]);
				d[3] = sqrtf(v[3]) * sinf(2 * 3.1415926f * v[2]);

				data[(k * size * size + j * size + i) * 4 + 0] = (signed char)(d[0] * 127);
				data[(k * size * size + j * size + i) * 4 + 1] = (signed char)(d[1] * 127);
				data[(k * size * size + j * size + i) * 4 + 2] = (signed char)(d[2] * 127);
				data[(k * size * size + j * size + i) * 4 + 3] = (signed char)(d[3] * 127);
			}
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_SIGNED_RGBA_NV, size, size, samples_u * samples_v / 2, 0, GL_RGBA, GL_BYTE, data);

	delete [] data;

}

void init_opengl()
{
	if(! glh_init_extensions(REQUIRED_EXTENSIONS))
	{
		cerr << "Necessary extensions were not supported:" << endl
			<< glh_get_unsupported_extensions() << endl << endl
			<< "Press <enter> to quit." << endl << endl;
		cerr << "Extensions: " << glGetString(GL_EXTENSIONS) << endl << endl;
		cerr << "Renderer: " << glGetString(GL_RENDERER) << endl << endl;
		char buff[10];
		cin.getline(buff, 10);
		exit(0);
	}

	glClearColor(.5f, .5f, .5f, .5f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	array2<vec3ub> img;

	// Load decal image
	decal.bind();
	read_png_rgb("decal_image.png", img);
	make_rgb_texture(img, true);
	decal.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	decal.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load spotlight image
	light_image.bind();
	read_png_rgb("nvlogo_spot.png", img);
	make_rgb_texture(img, true);
	light_image.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	light_image.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	light_image.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	light_image.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create per-pixel jitter lookup textures
	create_jitter_lookup(jitter_lookup_64, JITTER_SIZE, 8, 8);	// 8 'estimation' samples, 64 total samples
	create_jitter_lookup(jitter_lookup_32, JITTER_SIZE, 4, 8);	// 4 'estimation' samples, 32 total samples

	// Create some sample geometry ...
	quad.new_list(GL_COMPILE);
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glScalef(4, 4, 4);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(0, 8);
	glVertex2f(-1,  1);
	glTexCoord2f(8, 8);
	glVertex2f(1,  1);
	glTexCoord2f(8, 0);
	glVertex2f(1, -1);
	glEnd();
	glPopMatrix();
	quad.end_list();

	geometry.new_list(GL_COMPILE);
	glPushMatrix();
	glTranslatef(0, .4f, 0);
	glutSolidTeapot(.5f);
	glPopMatrix();
	geometry.end_list();

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	// Load programs
	char * txt = 0;
	
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VP_NAME);
	txt = read_text_file("simple_soft_shadows/soft_shadow.vp");
	if (!txt) exit(0);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
	if (glGetError() != GL_NO_ERROR) {
		cerr << "Error loading vertex program" << endl <<
		glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
		exit(0);
	}

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_64TAPS_NV3X);
	txt = read_text_file("simple_soft_shadows/soft_shadow64_nv3x.fp");
	if (!txt) exit(0);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
	if (glGetError() != GL_NO_ERROR) {
		cerr << "Error loading 64-samples NV3X fragment program" << endl <<
		glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
		exit(0);
	}

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_32TAPS_NV3X);
	txt = read_text_file("simple_soft_shadows/soft_shadow32_nv3x.fp");
	if (!txt) exit(0);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
	if (glGetError() != GL_NO_ERROR) {
		cerr << "Error loading 32-samples NV3X fragment program" << endl <<
		glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
		exit(0);
	}

	if (glh_extension_supported("GL_NV_fragment_program2")) {
		nv4x_supported = true;
		mode_nv3x = 0;
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_64TAPS_NV4X);

		txt = read_text_file("simple_soft_shadows/soft_shadow64_nv4x.fp");
		if (!txt) exit(0);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
		if (glGetError() != GL_NO_ERROR) {
			cerr << "Error loading 64-samples NV4X fragment program" << endl <<
			glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
			exit(0);
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_32TAPS_NV4X);
		txt = read_text_file("simple_soft_shadows/soft_shadow32_nv4x.fp");
		if (!txt) exit(0);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
		if (glGetError() != GL_NO_ERROR) {
			cerr << "Error loading 32-samples NV4X fragment program" << endl <<
			glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
			exit(0);
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_64TAPS_SHOW_PENUMBRA);
		txt = read_text_file("simple_soft_shadows/show_penumbra8_nv4x.fp");
		if (!txt) exit(0);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
		if (glGetError() != GL_NO_ERROR) {
			cerr << "Error loading 64-samples NV4X fragment program for penumbra visualization" << endl <<
			glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
			exit(0);
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP_32TAPS_SHOW_PENUMBRA);
		txt = read_text_file("simple_soft_shadows/show_penumbra4_nv4x.fp");
		if (!txt) exit(0);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(txt), txt);
		if (glGetError() != GL_NO_ERROR) {
			cerr << "Error loading 64-samples NV4X fragment program for penumbra visualization" << endl <<
			glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
			exit(0);
		}

	}

	// init pbuffer
	pbuffer.Initialize(TEX_SIZE, TEX_SIZE, false, true);
	pbuffer.Activate();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glFrontFace(GL_CCW);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	{
		GLint depth_bits;
		glGetIntegerv(GL_DEPTH_BITS, & depth_bits);
        
		if(depth_bits == 16)  depth_format = GL_DEPTH_COMPONENT16_ARB;
		else                  depth_format = GL_DEPTH_COMPONENT24_ARB;
	}
	pbuffer.Deactivate();

	light_view_depth.bind();
	light_view_depth.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	light_view_depth.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	light_view_depth.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	light_view_depth.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	light_view_depth.parameter(GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	light_view_depth.parameter(GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, depth_format, TEX_SIZE, TEX_SIZE, 0, 
		GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
}

void render_scene(glut_simple_mouse_interactor & view)
{
	glPushMatrix();
	view.apply_inverse_transform();
	object.apply_transform();

	quad.call_list();
	geometry.call_list();

	glScalef(0.5f, 0.5f, 0.5f);
	glTranslatef(0, 1.6f, 0);

	glPopMatrix();
}


void update_light_view()
{
	pbuffer.Activate();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

    glPolygonOffset(2.5f, 16.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);

	lightshaper.apply();

	// Render shadowmap ...
	render_scene(spotlight);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// ... And copy to texture
    light_view_depth.bind();
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,  lightshaper.width, lightshaper.height);

    pbuffer.Deactivate();
}

void display()
{
	// Prepare shadowmap
	update_light_view();

    // place light
    glPushMatrix();
	glLoadIdentity();
	camera.apply_inverse_transform();
	spotlight.apply_transform();
	glLightfv(GL_LIGHT0, GL_POSITION, & vec4f(0,0,0,1)[0]);
	glPopMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MATRIX0_ARB);
	glLoadIdentity();
	glTranslatef(.5f, .5f, .5f);
	glScalef(.5f, .5f, .5f);
	
	gluPerspective(lightshaper.fovy, 1, lightshaper.zNear, lightshaper.zFar);
	spotlight.apply_inverse_transform();
	camera.apply_transform();

	glMatrixMode(GL_MODELVIEW);

	// texture unit 0 - decal image
	glActiveTextureARB(GL_TEXTURE0_ARB);
	decal.bind();
	decal.enable();

	// texture unit 1 - shadowmap
	glActiveTextureARB(GL_TEXTURE1_ARB);
	light_view_depth.bind();
	light_view_depth.enable();

	// texture unit 2 - jitter lookup
	glActiveTextureARB(GL_TEXTURE2_ARB);
	if (mode_32) {
		jitter_lookup_32.bind();
		jitter_lookup_32.enable();
	} else {
		jitter_lookup_64.bind();
		jitter_lookup_64.enable();
	}

	// texture unit 3 - spot light image
	glActiveTextureARB(GL_TEXTURE3_ARB);
	light_image.bind();
	light_image.enable();

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VP_NAME);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, mode_show_penumbra * 4 + mode_nv3x * 2 + mode_32 + 1);

	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 0, 0, 0, softness / TEX_SIZE);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, (float)jitter / JITTER_SIZE, (float)jitter / JITTER_SIZE, 0, 0);

	reshaper.apply();

	// Render scene with shadows
    render_scene(camera);

	// Draw some stat info
	draw_text();
	
	glutSwapBuffers();

	++frames;

	static int last_time = 0;
	int this_time = glutGet(GLUT_ELAPSED_TIME);
	if (this_time - last_time > 500) {
		fps = (float)frames * 1000 / (this_time - last_time);
		frames = 0;
		last_time = this_time;
	}
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGB);
	glutCreateWindow("Optimizing soft shadows with dynamic branching");

	init_opengl();
    
	glut_helpers_initialize();
	cb.keyboard_function = key;
	cb.display_function = display;

	camera.configure_buttons(1);
	object.configure_buttons(1);
	spotlight.configure_buttons(1);

	object.dolly.dolly[2] = -3; // push plane forward
	object.trackball.r = rotationf(vec3f(1,0,0), to_radians(30));

	spotlight.pan.pan[0] = -.5;
	spotlight.pan.pan[1] = 6.0;
	spotlight.dolly.dolly[2] = -2.; // push spotlight forward
	spotlight.trackball.r =  rotationf(vec3f(1.f, 0.f, 0.f), to_radians(-90));

	camera.set_camera_mode(true);
	spotlight.set_camera_mode(false);
	spotlight.trackball.invert_increment = true;

	camera.set_parent_rotation( & camera.trackball.r);
	object.set_parent_rotation( & camera.trackball.r);
	spotlight.set_parent_rotation( & camera.trackball.r);

	lightshaper.fovy  = 60.f;
	lightshaper.zNear = .5f;
	lightshaper.zFar  = 10.f;
	lightshaper.width = TEX_SIZE;
	lightshaper.height = TEX_SIZE;

	camera.disable();
	spotlight.disable();
	lightshaper.disable();
    // attach interactors to the event multiplexor
	glut_add_interactor(& cb);
	glut_add_interactor(& reshaper);
	glut_add_interactor(& object);
	glut_add_interactor(& camera);
	glut_add_interactor(& spotlight);

    
	int copyrights = glutCreateMenu(menu);
	glutAddMenuEntry("simple_soft_shadows (c) 2004 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2001 NVIDIA Corporation", 0);
	glutAddMenuEntry("GLH -- Copyright (c) 2000 Cass Everitt", 0);
	glutAddMenuEntry("libpng -- Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson", 0);

	glutCreateMenu(menu);

	glutAddMenuEntry("move camera [1]", '1');
	glutAddMenuEntry("move object [2]", '2');
	glutAddMenuEntry("move light  [3]", '3');
	glutAddMenuEntry("increase shadow softness [+]", '+');
	glutAddMenuEntry("decrease shadow softness [-]", '-');
	glutAddMenuEntry("toggle perpixel jitter [j]", 'j');
	glutAddMenuEntry("toggle 64/32 samples [s]", 's');
	if (nv4x_supported) glutAddMenuEntry("toggle dynamic branching (NV4X/NV3X mode) [b]", 'b');
	if (nv4x_supported) glutAddMenuEntry("toggle showing penumbra (oversample region) [p]", 'p');
	glutAddSubMenu("copyright info", copyrights);
	glutAddMenuEntry("quit [q]", 'q');

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutIdleFunc(idle);

    glutMainLoop();

	return 0;
}

