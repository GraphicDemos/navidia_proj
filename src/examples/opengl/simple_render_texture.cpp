// simple_render_texture.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <iostream>

#include <GL/gl.h>
#undef GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#include <GL/glut.h>

// Convenience stuff for pbuffer object.
#define GLH_EXT_SINGLE_FILE
#define REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
							"WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format " \
                            "WGL_ARB_render_texture " \
                            "GL_SGIS_generate_mipmap "

#include <glh/glh_extensions.h>
#include <nv_math.h>
#include <nv_manip.h>
#include <shared/quitapp.h>

using namespace std;

#define MAX_ATTRIBS     256
#define MAX_PFORMATS    256
#define TEX_SIZE        256

// keyboard state
bool            b[256];

// Define a PBuffer object.
typedef struct _pbuffer
{
    HPBUFFERARB  hpbuffer;      // Handle to a pbuffer.
    HDC          hdc;           // Handle to a device context.
    HGLRC        hglrc;         // Handle to a GL rendering context.
    int          width;         // Width of the pbuffer
    int          height;        // Height of the pbuffer
} nv_pbuffer;

mat4            camera_mat;
nv_pbuffer      pbuffer;
GLuint          render_texture;
nv_scalar       max_aniso;
nv_scalar       aniso;
bool            animate;
int             frame = 0;
// camera standard manipulator
// idx 0 : camera
// idx 1 : rendered object
nv_manip *      manip[2];
unsigned int    cur_manip = 0;
// rendering setup
// near and far clip planes
nv_scalar       near_z = .25;
nv_scalar       far_z = 20.0;
// field of view
nv_scalar       fovy = 80.0;

// GLUT window HDC and HGLRC
HDC             hdc;
HGLRC           hglrc;

void display();
void init();
void keyboard( unsigned char k, int x, int y );
void reshape( int w, int h );
void mouse( int button, int state, int x, int y );
void motion( int x, int y );
void idle();
void menu( int k );
void reset();

void wglGetLastError()
{
    DWORD err = GetLastError();
    switch(err)
    {
    case ERROR_INVALID_PIXEL_FORMAT:
        cerr << "Win32 Error:  ERROR_INVALID_PIXEL_FORMAT\n";
        break;
    case ERROR_NO_SYSTEM_RESOURCES:
        cerr << "Win32 Error:  ERROR_NO_SYSTEM_RESOURCES\n";
        break;
    case ERROR_INVALID_DATA:
        cerr << "Win32 Error:  ERROR_INVALID_DATA\n";
        break;
    case ERROR_INVALID_WINDOW_HANDLE:
        cerr << "Win32 Error:  ERROR_INVALID_WINDOW_HANDLE\n";
        break;
    case ERROR_RESOURCE_TYPE_NOT_FOUND:
        cerr << "Win32 Error:  ERROR_RESOURCE_TYPE_NOT_FOUND\n";
        break;
    case ERROR_SUCCESS:
        // no error
        break;
    default:
        cerr << "Win32 Error:  " << err << endl;
        break;
    }
    SetLastError(0);
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Simple Render To Texture Example" );

    // Do some OpenGL initialization.
	init();

    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
  	glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
	glutDisplayFunc(display);

	glutCreateMenu(menu);
	glutAddMenuEntry("move textured quad [1]", '1');
	glutAddMenuEntry("move teapot in texture [2]", '2');
	glutAddMenuEntry("Toggle auto mipmap generation [m]", 'm');
	glutAddMenuEntry("Reset the scene [r]", 'r');
    glutAddMenuEntry("Toggle teapot animation [ ]", ' ');
	glutAddMenuEntry("quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Give control over to glut.
	glutMainLoop();
	return 0;
}

void shutdown_pbuffer(nv_pbuffer * pbuffer)
{
    if ( pbuffer->hpbuffer )
    {
        // Check if we are currently rendering in the pbuffer
        if (wglGetCurrentContext() == pbuffer->hglrc )
            wglMakeCurrent(0,0);
        // delete the pbuffer context
        wglDeleteContext( pbuffer->hglrc );
        wglReleasePbufferDCARB( pbuffer->hpbuffer, pbuffer->hdc );
        wglDestroyPbufferARB( pbuffer->hpbuffer );
        pbuffer->hpbuffer = 0;
    }

    // clean up the manipulators
    for ( int i = 0; i < 2; ++i)
        delete manip[i];
}

void init_pbuffer(nv_pbuffer * pbuffer, int width, int height)
{
    memset(pbuffer,0,sizeof(nv_pbuffer));

    HDC hdc = wglGetCurrentDC();
	HGLRC hglrc = wglGetCurrentContext();
    wglGetLastError();

    // Query for a suitable pixel format based on the specified mode.
    int     format;
    int     pformat[MAX_PFORMATS];
    unsigned int nformats;    int     iattributes[2*MAX_ATTRIBS];
    float   fattributes[2*MAX_ATTRIBS];
    int     nfattribs = 0;
    int     niattribs = 0;

    // Attribute arrays must be "0" terminated - for simplicity, first
    // just zero-out the array entire, then fill from left to right.
    memset(iattributes,0,sizeof(int)*2*MAX_ATTRIBS);
    memset(fattributes,0,sizeof(float)*2*MAX_ATTRIBS);
    // Since we are trying to create a pbuffer, the pixel format we
    // request (and subsequently use) must be "p-buffer capable".
    iattributes[niattribs  ] = WGL_DRAW_TO_PBUFFER_ARB;
    iattributes[++niattribs] = GL_TRUE;
    // we are asking for a pbuffer that is meant to be bound
    // as an RGBA texture - therefore we need a color plane
    iattributes[++niattribs] = WGL_BIND_TO_TEXTURE_RGBA_ARB;
    iattributes[++niattribs] = GL_TRUE;

    if ( !wglChoosePixelFormatARB( hdc, iattributes, fattributes, MAX_PFORMATS, pformat, &nformats ) )
    {
        cerr << "pbuffer creation error:  wglChoosePixelFormatARB() failed.\n";
        quitapp( -1 );
    }
    wglGetLastError();
	if ( nformats <= 0 )
    {
        cerr << "pbuffer creation error:  Couldn't find a suitable pixel format.\n";
        quitapp( -1 );
    }
    format = pformat[0];

    // Set up the pbuffer attributes
    memset(iattributes,0,sizeof(int)*2*MAX_ATTRIBS);
    niattribs = 0;
    // the render texture format is RGBA
    iattributes[niattribs] = WGL_TEXTURE_FORMAT_ARB;
    iattributes[++niattribs] = WGL_TEXTURE_RGBA_ARB;
    // the render texture target is GL_TEXTURE_2D
    iattributes[++niattribs] = WGL_TEXTURE_TARGET_ARB;
    iattributes[++niattribs] = WGL_TEXTURE_2D_ARB;
    // ask to allocate room for the mipmaps
    iattributes[++niattribs] = WGL_MIPMAP_TEXTURE_ARB;
    iattributes[++niattribs] = TRUE;
    // ask to allocate the largest pbuffer it can, if it is
    // unable to allocate for the width and height
    iattributes[++niattribs] = WGL_PBUFFER_LARGEST_ARB;
    iattributes[++niattribs] = FALSE;
    // Create the p-buffer.
    pbuffer->hpbuffer = wglCreatePbufferARB( hdc, format, width, height, iattributes );
    if ( pbuffer->hpbuffer == 0)
    {
        cerr << "pbuffer creation error:  wglCreatePbufferARB() failed\n";
        wglGetLastError();
        quitapp( -1 );
    }
    wglGetLastError();

    // Get the device context.
    pbuffer->hdc = wglGetPbufferDCARB( pbuffer->hpbuffer );
    if ( pbuffer->hdc == 0)
    {
        cerr << "pbuffer creation error:  wglGetPbufferDCARB() failed\n";
        wglGetLastError();
        quitapp( -1 );
    }
    wglGetLastError();

    // Create a gl context for the p-buffer.
    pbuffer->hglrc = wglCreateContext( pbuffer->hdc );
    if ( pbuffer->hglrc == 0)
    {
         cerr << "pbuffer creation error:  wglCreateContext() failed\n";
        wglGetLastError();
         quitapp( -1 );
    }
    wglGetLastError();

    // Determine the actual width and height we were able to create.
    wglQueryPbufferARB( pbuffer->hpbuffer, WGL_PBUFFER_WIDTH_ARB, &pbuffer->width );
    wglQueryPbufferARB( pbuffer->hpbuffer, WGL_PBUFFER_HEIGHT_ARB, &pbuffer->height );
}

void init()
{
	cerr << "Extensions: " << endl << (char *)glGetString(GL_EXTENSIONS) << endl;
    // Get the entry points for the extension.
	if( !glh_init_extensions(REQUIRED_EXTENSIONS) )
	    {
        cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp( -1 );
	    }

    // get the GLUT window HDC and HGLRC
    hdc = wglGetCurrentDC();
    hglrc = wglGetCurrentContext();

    // Initialize the PBuffer now that we have a valid context
    // that we can use during the p-buffer creation process.
    init_pbuffer(&pbuffer, TEX_SIZE, TEX_SIZE);
    wglGetLastError();

    // Initialize some graphics state for the PBuffer's rendering context.
    if (wglMakeCurrent( pbuffer.hdc, pbuffer.hglrc) == FALSE)
        wglGetLastError();

    // Clear color
    glClearColor( 0.0, 0.0, 0.0, 0.0 );

    // The viewing settings.
    glMatrixMode( GL_PROJECTION );
	gluPerspective(60, 1, .1, 10);
	glMatrixMode(GL_MODELVIEW);

    // Initialize some state for the GLUT window's rendering context.
    if (wglMakeCurrent( hdc, hglrc) == FALSE)
        wglGetLastError();

    // Clear color
    glClearColor( 0.1, 0.1, 0.7, 0.0 );

    // create our render texture object
    glGenTextures(1, &render_texture);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    // notice that we don't need to call glTexImage2D
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Generate mipmap automatically
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

    // Set anisotropic filter to the max ratio
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    aniso = max_aniso;

    // create a default mamipulator
    for ( int i = 0; i < 2; ++i)
        manip[i] = new nv_manip();

    manip[0]->set_manip_behavior( nv_manip::OBJECT | nv_manip::ALL );

    manip[1]->set_manip_behavior( nv_manip::OBJECT | nv_manip::ROTATE );

    look_at(camera_mat, vec3(0,0,2.0), vec3_neg_z, vec3_y);

    reset();
}

void reset()
{
    animate = true;
    // create a default mamipulator
    for ( int i = 0; i < 2; ++i)
    {
        manip[i]->reset();
        manip[i]->set_clip_planes(near_z, far_z);
        manip[i]->set_fov(fovy);
    }
}

void keyboard( unsigned char k, int x, int y )
{
	b[k] = !b[k];

    if ( k == 27 || k == 'q' ) 
    {
        shutdown_pbuffer(&pbuffer);
        exit( 0 );
    }
    if ( k == 'r' || k == 'R' )
        reset();
    if ( k == ' ' )
        animate = !animate;

    // camera mode
    if ( k == '1' )
        cur_manip = 0;
    // render object mode
    if ( k == '2' )
        cur_manip = 1;

    // tweaks to play with anisotropic filtering
    if ( k == '+' )
        aniso = (aniso + nv_one > max_aniso ) ? max_aniso : aniso + nv_one;
    if ( k == '-' )
        aniso = (aniso - nv_one < nv_one ) ? nv_one : aniso - nv_one;
    if ( k == '-' || k == '+')
    {
        glBindTexture(GL_TEXTURE_2D, render_texture);
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }

    // tweak to play with automatic mipmap generation
    if ( k == 'm' )
    {
        glBindTexture(GL_TEXTURE_2D, render_texture);
        if (b[k])
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
        }
        else
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        }
    }
	glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

void menu(int k)
{
	keyboard((unsigned char)k, 0, 0);
}

void motion(int x, int y)
{
    manip[cur_manip]->mouse_move(vec2(x,y),0);
}

void reshape(int w, int h)
{
    manip[0]->set_screen_size(w,h);
    manip[1]->set_screen_size(w,h);

    if (wglMakeCurrent( hdc, hglrc) == FALSE)
        wglGetLastError();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( manip[0]->get_fov(), manip[0]->get_screen_ratio(), manip[0]->get_near_z(), manip[0]->get_far_z());
    glViewport(0,0,w,h);
}

void mouse(int button, int state, int x, int y)
{
    int input_state = 0;
    // mouse keyboard state mask
    input_state |= (button == GLUT_LEFT_BUTTON) ? ((state == GLUT_DOWN) ? nv_manip::LMOUSE_DN : nv_manip::LMOUSE_UP) : 0;
    input_state |= (button == GLUT_MIDDLE_BUTTON) ? ((state == GLUT_DOWN) ? nv_manip::MMOUSE_DN : nv_manip::MMOUSE_UP) : 0;
    input_state |= (button == GLUT_RIGHT_BUTTON) ? ((state == GLUT_DOWN) ? nv_manip::RMOUSE_DN : nv_manip::RMOUSE_UP) : 0;
    // build keyboard state mask
    int key_state = glutGetModifiers();
    input_state |= (key_state & GLUT_ACTIVE_CTRL) ? nv_manip::CTRL_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_ALT) ? nv_manip::ALT_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_SHIFT) ? nv_manip::SHIFT_DN : 0;
    if (cur_manip)
        animate = false;
    // dispatch appropriately
    if (input_state & nv_manip::MOUSE_DN)
        manip[cur_manip]->mouse_down(vec2(x,y),input_state);
    if (input_state & nv_manip::MOUSE_UP)
        manip[cur_manip]->mouse_up(vec2(x,y),input_state);

    manip[0]->set_camera(camera_mat);
    manip[1]->set_camera(camera_mat);
}

void drawtext()
{
    int ww = manip[0]->get_screen_width();
    int wh = manip[0]->get_screen_height();

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

	char string[1024];
    sprintf(string, "Camera mode: %s\nAnisotropic filtering: %1.1f\nMipmap generation: %s\n",
                     cur_manip == 0 ? "Camera" : "Teapot",
                     aniso,
                     b['m'] ? "Off" : "On");
                    
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

void draw_rentex()
{
    glBindTexture(GL_TEXTURE_2D, render_texture);

	// release the pbuffer from the render texture object
    if (wglReleaseTexImageARB(pbuffer.hpbuffer, WGL_FRONT_LEFT_ARB) == FALSE)
        wglGetLastError();

    // make the pbuffer's rendering context current.
    if (wglMakeCurrent( pbuffer.hdc, pbuffer.hglrc) == FALSE)
        wglGetLastError();

    // clear all pixels.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw onto the pbuffer our spinning teapot
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef(0,0,-1);
	glColor3f(1,1,1);

    glMultMatrixf(manip[1]->get_mat().mat_array );
    glRotatef(frame % 360, 0, 1, 0);
    if (animate)
        ++frame;
	glutWireTeapot(.35);

    // make the glut window's rendering context current and draw to the glut window.
    if (wglMakeCurrent( hdc, hglrc) == FALSE)
        wglGetLastError();

    // bind the pbuffer to the render texture object
    if (wglBindTexImageARB(pbuffer.hpbuffer, WGL_FRONT_LEFT_ARB) == FALSE)
        wglGetLastError();
}

void display()
{
    draw_rentex();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_mat.mat_array);
    glMultMatrixf(manip[0]->get_mat().mat_array );
	glRotatef(60.0f, -1.0f, 2.0f, 0.0f );

    glBindTexture(GL_TEXTURE_2D, render_texture);

    // and draw textured quad with our rendered texture
    glEnable(GL_TEXTURE_2D);
    glBegin( GL_QUADS );
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(0, 1);
		glVertex2f(-1,  1);
		glTexCoord2f(1, 1);
		glVertex2f( 1,  1);
		glTexCoord2f(1, 0);
		glVertex2f( 1, -1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    drawtext();
    glutSwapBuffers();
}
