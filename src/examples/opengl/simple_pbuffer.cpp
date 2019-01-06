#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <stdio.h>
#include <iostream>

#if defined(WIN32)
#define REQUIRED_EXTENSIONS "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format "
#elif defined(UNIX)
#define REQUIRED_EXTENSIONS "GLX_SGIX_pbuffer " \
                            "GLX_SGIX_fbconfig "
#elif defined(MACOS)
#define REQUIRED_EXTENSIONS ""                            
#endif

// Convenience stuff for pbuffer object.
#define GLH_EXT_SINGLE_FILE
#include <glh/glh_glut.h>
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <shared/pbuffer.h>
#include <shared/quitapp.h>

using namespace std;

// Define a PBuffer object.
PBuffer mypbuffer("rgb depth");

GLubyte imgdata[256*256*4];

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

	char string[1024] = "This demo is a simple non-interactive demo that\n" \
                        "demonstrates how to use a p-buffer for off-screen\n" \
                        "rendering.  This demo creates and initializes a\n" \
                        "p-buffer.  For each frame, a rotating quad is\n" \
                        "rendered to the off-screen p-buffer.  The resulting\n" \
                        "image is readback into host memory from the p-buffer\n" \
                        "and then used in a subsequent glDrawPixels() call to\n" \
                        "the displayable graphics window.\n\n" \
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
    static int frame = 0;

    // Make the PBuffer's rendering context current.
    mypbuffer.Activate();

    // Clear all pixels.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.5, 0.5, 0.0 );
    glRotatef( (float)frame++, 0.0, 0.0, 1.0 );
    glTranslatef( -0.5, -0.5, 0.0 );

    // Draw a polygon with corners at
    // (0.25, 0.25, 0.0) and (0.75, 0.75, 0.0)
    glBegin( GL_POLYGON );
        glColor3f( 0, 0, 0.0 );
        glVertex3f( 0.25, 0.25, 0.0 );
        glColor3f( 1, 0, 0.0 );
        glVertex3f( 0.75, 0.25, 0.0 );
        glColor3f( 1,  1, 0.0 );
        glVertex3f( 0.75, 0.75, 0.0 );
        glColor3f( 0,  1, 0.0 );
        glVertex3f( 0.25, 0.75, 0.0 );
    glEnd();

    // Read the data from the PBuffer.
    glReadPixels( 0, 0, mypbuffer.GetWidth(), mypbuffer.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imgdata );
    int imgwidth  = mypbuffer.GetWidth();
    int imgheight = mypbuffer.GetHeight();

    // Make the glut window's rendering context current and draw to the glut window.
    mypbuffer.Deactivate();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glRasterPos2i( 0, 0 );
    glDrawPixels( imgwidth, imgheight, GL_RGBA, GL_UNSIGNED_BYTE, imgdata );
    drawtext();
    glutSwapBuffers();
}


void keyboard( unsigned char k, int x, int y )
{
    if ( k == 27 || k == 'q' ) exit( 0 );
}

void idle()
{
    glutPostRedisplay();
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

    // Initialize the PBuffer now that we have a valid context
    // that we can use during the p-buffer creation process.
    mypbuffer.Initialize(256, 256, false, false);

    // Initialize some graphics state for the PBuffer's rendering context.
    mypbuffer.Activate();

    // Clear color
    glClearColor( 0.0, 0.0, 0.0, 0.0 );

    // The viewing settings.
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );


    // Initialize some state for the GLUT window's rendering context.
    mypbuffer.Deactivate();

    // Clear color
    glClearColor( 0.1, 0.1, 0.7, 0.0 );

    // The viewing settings.
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
}


int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Simple PBuffer Example" );

    // Do some OpenGL initialization.
	init();

    // Setup the callbacks.
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutIdleFunc( idle );

    // Give control over to glut.
	glutMainLoop();
	return 0;
}
