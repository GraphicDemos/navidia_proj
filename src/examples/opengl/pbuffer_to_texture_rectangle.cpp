#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <stdio.h>
#include <iostream>

// Convenience stuff for pbuffer object.
#define GLH_EXT_SINGLE_FILE

#if defined(WIN32)
#  define REQUIRED_EXTENSIONS "WGL_ARB_pbuffer " \
                              "WGL_ARB_pixel_format " \
                              "GL_NV_texture_rectangle "
#elif defined (UNIX)
#  define REQUIRED_EXTENSIONS "GLX_SGIX_pbuffer " \
                              "GLX_SGIX_fbconfig " \
                              "GL_NV_texture_rectangle "
#elif defined (MACOS)
#  define REQUIRED_EXTENSIONS "GL_EXT_texture_rectangle " 
#endif
							
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <shared/pbuffer.h>
#include <shared/quitapp.h>

using namespace glh;
using namespace std;

#define TEX_SIZE 600
tex_object_rectangle tex;

glut_callbacks cb;
glut_perspective_reshaper reshaper;
glut_simple_mouse_interactor object, pbuff_object;

int frame=0;

bool b[256];
rotationf parent;

// Define a PBuffer object.
PBuffer mypbuffer("rgb depth");

#if defined(MACOS)
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#endif

void drawtext()
{
    int ww = glutGet( (GLenum)GLUT_WINDOW_WIDTH );
    int wh = glutGet( (GLenum)GLUT_WINDOW_HEIGHT );

    glDisable( GL_DEPTH_TEST );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, ww-1, 0, wh-1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

	int x = 20;
	int y = wh-22;
    glRasterPos2i( x, y );

	char string[1024] = "This demonstrates how to use NV_texture_rectangle.\n"
						"It renders a glutWireTeapot() to the 600x600 pbuffer, \n"
						"then does a fast glCopyTexSubImage2D() to copy it.\n\n"

                        "Press the 'ESC' key to exit.";
                        
	char *p;

    for ( p = string; *p; p++ )
		{
        if ( *p == '\n' )
            {
            y = y - 14;
            glRasterPos2i( x, y );
            continue;
            }
        glutBitmapCharacter( GLUT_BITMAP_9_BY_15, *p );
		}

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glEnable( GL_DEPTH_TEST );
}


void display()
{
	parent = object.trackball.r.inverse();
	frame %= 360;

    // Make the PBuffer's rendering context current.
    mypbuffer.Activate();

    // Clear all pixels.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	pbuff_object.apply_transform();

	glColor3f(1,1,1);
	glRotatef(frame, 0, 1, 0);
	glutWireTeapot(.35);

	tex.bind();
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, TEX_SIZE, TEX_SIZE);

	glPopMatrix();

    // Make the glut window's rendering context current and draw to the glut window.
    mypbuffer.Deactivate();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	object.apply_transform();
	
	tex.enable();
    glBegin( GL_QUADS );
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(0, TEX_SIZE);
		glVertex2f(-1,  1);
		glTexCoord2f(TEX_SIZE, TEX_SIZE);
		glVertex2f( 1,  1);
		glTexCoord2f(TEX_SIZE, 0);
		glVertex2f( 1, -1);
    glEnd();
	tex.disable();

	glPopMatrix();
	glPushMatrix();

	drawtext();

	glPopMatrix();
    glutSwapBuffers();
}


void keyboard( unsigned char k, int x, int y )
{
	b[k] = !b[k];

    if ( k == 27 || k == 'q' ) exit( 0 );
	if ( k == '1')
	{
		object.enable();
		pbuff_object.disable();
	}
	if ( k == '2')
	{
		pbuff_object.enable();
		object.disable();
	}
	if(k == 'a')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	if(k == 'b')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(k == 'k')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
	}
	if(k == 'l')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);
	}
	if(k == 'm')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
	}
	if(k == 'n')
	{
		tex.bind();
		tex.parameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.f);
	}
	if(k == ' ')
	{
		glut_idle(b[k]);
	}
	glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
	if( b['T'] )
	{
		pbuff_object.trackball.increment_rotation();
	}
	if( b['P'] )
	{
		object.trackball.increment_rotation();
	}
		
}


void init()
{

    // Get the entry points for the extension.
	if( !glh_init_extensions(REQUIRED_EXTENSIONS) )
	    {
        cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp( -1 );
	    }

	tex.bind();
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_FLOAT, 0);
	tex.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glClearColor(.2, .2, .5, 0);

    // Initialize the PBuffer now that we have a valid context
    // that we can use during the p-buffer creation process.
    mypbuffer.Initialize(TEX_SIZE, TEX_SIZE, false, true);

    // Initialize some graphics state for the PBuffer's rendering context.
    mypbuffer.Activate();

    // Clear color
    glClearColor( 0.0, 0.0, 0.0, 0.0 );

    // The viewing settings.
    glMatrixMode( GL_PROJECTION );
	gluPerspective(60, 1, .1, 10);
	glMatrixMode(GL_MODELVIEW);

    // Initialize some state for the GLUT window's rendering context.
    mypbuffer.Deactivate();

    // Clear color
    glClearColor( 0.1, 0.1, 0.7, 0.0 );

}

void menu(int k)
{
	keyboard((unsigned char)k, 0, 0);
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Simple PBuffer Example" );


    // Do some OpenGL initialization.
	init();

	glut_helpers_initialize();

	cb.display_function = display;
	cb.keyboard_function = keyboard;
	cb.idle_function = idle;

	object.configure_buttons(1);
	object.dolly.dolly[2] = -3;
	pbuff_object.configure_buttons(1);
	pbuff_object.dolly.dolly[2] = -1;
	pbuff_object.disable();

	pbuff_object.set_parent_rotation(&parent);

	glut_add_interactor(& cb);
	glut_add_interactor(& reshaper);
	glut_add_interactor(& object);
	glut_add_interactor(& pbuff_object);

	b['T'] = true;
	b['P'] = true;
	b[' '] = true;
	glut_idle(true);

	keyboard('f', 0, 0);  // turn on trilinear
	keyboard('l', 0, 0);  // turn on anisotropic

	object.trackball.incr = rotationf(vec3f(2,1,12), to_radians(1));
	pbuff_object.trackball.incr = rotationf(vec3f(2,3,5), to_radians(1));

	int aniso = glutCreateMenu(menu);
	glutAddMenuEntry("isotropic filtering [k]", 'k');
	glutAddMenuEntry("2:1 anisotropic filtering [l]", 'l');
	glutAddMenuEntry("4:1 anisotropic filtering [m]", 'm');
	glutAddMenuEntry("8:1 anisotropic filtering [n]", 'n');


	int filtering = glutCreateMenu(menu);
	glutAddMenuEntry("nearest [a]", 'a');
	glutAddMenuEntry("linear [b]", 'b');
	glutAddSubMenu("anisotropy", aniso);
	

	glutCreateMenu(menu);
	glutAddMenuEntry("toggle animation [ ]", ' ');
	glutAddMenuEntry("  teapot animation [T]", 'T');
	glutAddMenuEntry("  textured quad animation [P]", 'P');
	glutAddMenuEntry("move textured quad [1]", '1');
	glutAddMenuEntry("move teapot in pbuffer view [2]", '2');
	glutAddSubMenu("filtering", filtering);
	glutAddMenuEntry("quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);


    // Give control over to glut.
	glutMainLoop();
	return 0;
}
