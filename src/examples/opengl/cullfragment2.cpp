#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <glh/glh_glut_text.h>
#include <glh/glh_linear.h>
#include <nvparse.h>
#include <shared/read_text_file.h>
#include <shared/quitapp.h>

using namespace glh::ns_float;
// Declare the effect browser information

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

simple_multi_line_text help;

bool b[256];

display_list ts_cull_less;
display_list ts_cull_greater;
display_list *ts_state = &ts_cull_greater;

#define MAX_CULLERS		8

// Vertex Program ID (Similar to a texture object ID)
GLuint vpid[MAX_CULLERS];

float minx = -0.25;
float miny = -0.25;
float minz = -0.25;
float maxx = 0.25;
float maxy = 0.25;
float maxz = 0.25;

float posx[MAX_CULLERS] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
float posy[MAX_CULLERS] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
float posz[MAX_CULLERS] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

float velocityx[MAX_CULLERS] = { 0.2, 0.03, -0.2, 0.06, 0.3, 0.01,  0.3, 0.09 };
float velocityy[MAX_CULLERS] = { 0.3, 0.01,  0.3, 0.09, 0.1, 0.01,  0.01, 0.01 };
float velocityz[MAX_CULLERS] = { 0.1, 0.01,  0.01, 0.01, 0.2, 0.03, -0.2, 0.06 };

float timestep = 0.015;

int width  = 8;
int height = 8;
int depth  = 8;
float radius[4] = { 0.0625, 0.0625, 0.0625, 0.0625 };
int ncullers = 1;

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void special(int k, int x, int y);
void menu(int k);
void idle();

// my functions
void init_opengl();
void DrawMiniCubes();

void CheckParseErrors()
{
	for (char * const * errors = nvparse_get_errors(); *errors; errors++)
		fprintf(stderr, *errors);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow("Cullfragment - Complex Shape");

	init_opengl();

	font * f = new glut_stroke_mono_roman();
	f->initialize();
	help.set_font(f);
	help.set_text( 
		"This is the help screen -- add helpful text here...\n"
		"  [h or F1]:     toggle help screen\n"
		"  [ ]:     toggle animation\n"
		"  [r]:      toggle rotation animation\n"
		"  [<esc>]: quit");
	
	glut_helpers_initialize();

	cb.keyboard_function = key;
	cb.special_function = special;
	cb.idle_function = idle;
	camera.configure_buttons(1);
	camera.set_camera_mode(true);
	object.configure_buttons(1);
	object.dolly.dolly[2] = -1;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);

	b[' '] = 1;
	b['r'] = 1;
    glut_idle(1);
//	b['c'] = 1;

	glutCreateMenu(menu);
	glutAddMenuEntry("Incr. Width [u]", 'u');
	glutAddMenuEntry("Decr. Width [j]", 'j');
	glutAddMenuEntry("Incr. Height [i]", 'i');
	glutAddMenuEntry("Decr. Height [k]", 'k');
	glutAddMenuEntry("Incr. Depth [o]", 'o');
	glutAddMenuEntry("Decr. Depth [l]", 'l');
	glutAddMenuEntry("Incr. Radius [t]", 't');
	glutAddMenuEntry("Decr. Radius [g]", 'g');
	glutAddMenuEntry("Incr. Points [+]", '+');
	glutAddMenuEntry("Decr. Points [-]", '-');
	glutAddMenuEntry("Invert Cull [p]", 'p');
	glutAddMenuEntry("Toggle Cullers [c]", 'c');
	glutAddMenuEntry("Toggle Wireframe [w]", 'w');
	glutAddMenuEntry("Show Boundary [b]", 'b');
	glutAddMenuEntry("quit [esc]", 27);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

void init_lighting()
{
	GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };

	glShadeModel( GL_SMOOTH );

	glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
	glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	glEnable( GL_LIGHT0 );
	glEnable( GL_RESCALE_NORMAL );
}

void LOAD_VERTEX_PROGRAM( unsigned int _i, char *_n )
{
    char *str = read_text_file( _n );
    glBindProgramARB( GL_VERTEX_PROGRAM_ARB, _i );
    nvparse( str );
   	CheckParseErrors();
    delete [] str;
}

void init_vp()
{
	if( !glh_init_extensions( "GL_ARB_vertex_program" ) )
	    {
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp( -1 );
	    }

    // Create the vertex programs.
	// One for each possible number of cullers.
    glGenProgramsARB( MAX_CULLERS, vpid );

	LOAD_VERTEX_PROGRAM( vpid[0], "cullfragment2/cullfragment2_culldist1.vp" );
	LOAD_VERTEX_PROGRAM( vpid[1], "cullfragment2/cullfragment2_culldist2.vp" );
	LOAD_VERTEX_PROGRAM( vpid[2], "cullfragment2/cullfragment2_culldist3.vp" );
	LOAD_VERTEX_PROGRAM( vpid[3], "cullfragment2/cullfragment2_culldist4.vp" );
	LOAD_VERTEX_PROGRAM( vpid[4], "cullfragment2/cullfragment2_culldist5.vp" );
	LOAD_VERTEX_PROGRAM( vpid[5], "cullfragment2/cullfragment2_culldist6.vp" );
	LOAD_VERTEX_PROGRAM( vpid[6], "cullfragment2/cullfragment2_culldist7.vp" );
	LOAD_VERTEX_PROGRAM( vpid[7], "cullfragment2/cullfragment2_culldist8.vp" );

    // Put useful constants into register 12.
    glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 12, radius );  
    glProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 13, 0.0, 0.5, 1.0, 2.0 );  

	if ( glGetError() )
	{
		quitapp(-1);
	}
}


// Initialize texture shader state.
void init_ts()
{
	if( !glh_init_extensions( "GL_NV_texture_shader " "GL_ARB_multitexture") )
	    {
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp( -1 );
	    }

    // Setup the display lists for the texture shading operation to be Cull-Fragment
	ts_cull_greater.new_list( GL_COMPILE );
	nvparse(
		"!!TS1.0\n"
		"cull_fragment( GEQUAL_TO_ZERO, GEQUAL_TO_ZERO, GEQUAL_TO_ZERO, GEQUAL_TO_ZERO );\n"
		"nop();\n"
		"nop();\n"
		"nop();\n"
		);
	CheckParseErrors();
	ts_cull_greater.end_list();

	ts_cull_less.new_list( GL_COMPILE );
	nvparse(
		"!!TS1.0\n"
		"cull_fragment( LESS_THAN_ZERO, LESS_THAN_ZERO, LESS_THAN_ZERO, LESS_THAN_ZERO );\n"
		"nop();\n"
		"nop();\n"
		"nop();\n"
		);
	CheckParseErrors();
	ts_cull_less.end_list();
}

void init_rc()
{
	if( !glh_init_extensions( "GL_NV_register_combiners" ) )
	    {
		cerr << "Necessary extensions were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl;
		quitapp( -1 );
	    }

    nvparse(
		"!!RC1.0\n"
		"out.rgb = col0;\n"
	);
	CheckParseErrors();
    glEnable( GL_REGISTER_COMBINERS_NV );
}

void init_opengl()
{
	init_vp();
	init_ts();
    init_rc();
	init_lighting();
	glEnable(GL_DEPTH_TEST);
}

void menu(int k)
{
	key((unsigned char)k, 0, 0);
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') exit(0);

	if ( k == 'u' || k == 'U' )
		{
		if ( width < 17 )
			{
			width++;
			}
		}
	if ( k == 'j' || k == 'J' )
		{
		if ( width > 1 )
			{
			width--;
			}
		}
	if ( k == 'i' || k == 'I' )
		{
		if ( height < 17 )
			{
			height++;
			}
		}
	if ( k == 'k' || k == 'K' )
		{
		if ( height > 1 )
			{
			height--;
			}
		}
	if ( k == 'o' || k == 'O' )
		{
		if ( depth < 17 )
			{
			depth++;
			}
		}
	if ( k == 'l' || k == 'L' )
		{
		if ( depth > 1 )
			{
			depth--;
			}
		}

	if ( k == 't' || k == 'T' )
		{
		radius[0] = radius[0] + 0.002;
		if ( radius[0] > 1 ) radius[0] = 1.0;
		radius[1] = radius[2] = radius[3] = radius[0];
	    glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 12, radius );  
		}
	if ( k == 'g' || k == 'G' )
		{
		radius[0] = radius[0] - 0.002;
		if ( radius[0] < 0.00001 ) radius[0] = 0.00001;
		radius[1] = radius[2] = radius[3] = radius[0];
	    glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 12, radius );  
		}

	if ( k == 'p' || k == 'P' )
		{
		if ( ts_state == &ts_cull_greater )
			ts_state = &ts_cull_less;
		else
			ts_state = &ts_cull_greater;
		}

    if ( k == 'w' || k == 'W' )
        {
        GLint mode[2];
        glGetIntegerv( GL_POLYGON_MODE, mode );
        if ( mode[1] == GL_FILL  )
            {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
            }
        else if ( mode[1] == GL_LINE  )
            {
            glPointSize( 4 );
            glPolygonMode( GL_FRONT_AND_BACK, GL_POINT ); 
            }
        else
            {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); 
            }
        }

	if ( k == '+' || k == '=' )
		{
		ncullers++;
		if ( ncullers > MAX_CULLERS ) ncullers = MAX_CULLERS;
		}
	if ( k == '-' || k == '_' )
		{
		ncullers--;
		if ( ncullers < 1 ) ncullers = 1;
		}

	glutPostRedisplay();
}

void special(int k, int x, int y)
{
	if(GLUT_KEY_F1 == k) key('h',0,0);
	glutPostRedisplay();
}

void UpdateCullerPosition()
{
	// Compute next position.

	for ( int c = 0; c < ncullers; c++ )
	{
	float prevx = posx[c];
	float prevy = posy[c];
	float prevz = posz[c];

	float step = timestep;

	float stepx = step * velocityx[c];
	float stepy = step * velocityy[c];
	float stepz = step * velocityz[c];

	posx[c] = prevx + stepx;
	posy[c] = prevy + stepy;
	posz[c] = prevz + stepz;

	// Ensure that we've not moved outside the box.
	// If we have, compute position based on intersection point.
	int borderflag = 0;
	float hitdist;
	float astep;
	if ( posx[c] > maxx )
		{
		hitdist = maxx - prevx;
		astep = timestep * (hitdist/stepx);
		if ( astep < step ) step = astep;
		borderflag = 1;
		}
	else if ( posx[c] < minx )
		{
		hitdist = minx - prevx;
		astep = timestep * (hitdist/stepx);
		if ( astep < step ) step = astep;
		borderflag = 1;
		}

	if ( posy[c] > maxy )
		{
		hitdist = maxy - prevy;
		astep = timestep * (hitdist/stepy);
		if ( astep < step ) step = astep;
		borderflag = 2;
		}
	else if ( posy[c] < miny )
		{
		hitdist = miny - prevy;
		astep = timestep * (hitdist/stepy);
		if ( astep < step ) step = astep;
		borderflag = 2;
		}

	if ( posz[c] > maxz )
		{
		hitdist = maxz - prevz;
		astep = timestep * (hitdist/stepz);
		if ( astep < step ) step = astep;
		borderflag = 3;
		}
	else if ( posz[c] < minz )
		{
		hitdist = minz - prevz;
		astep = timestep * (hitdist/stepz);
		if ( astep < step ) step = astep;
		borderflag = 3;
		}

	stepx = step * velocityx[c];
	stepy = step * velocityy[c];
	stepz = step * velocityz[c];
	posx[c] = prevx + stepx;
	posy[c] = prevy + stepy;
	posz[c] = prevz + stepz;

	// Modify the velocity if a collision occured.
	switch( borderflag )
		{
		case 1:
			velocityx[c] = -velocityx[c];
			break;
		case 2:
			velocityy[c] = -velocityy[c];
			break;
		case 3:
			velocityz[c] = -velocityz[c];
			break;
		}
	}

}

void idle()
{
	if(b['r'])
		object.trackball.increment_rotation();

	if(b[' '])
		UpdateCullerPosition();

	glutPostRedisplay();
}

void display_help()
{
	float w, h;
 	help.get_dimensions(w,h);
	
	float s = (w > h)? w : h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_COLOR);

	glColor3f(.15, .15, .2);
	glBegin(GL_QUADS);
	glVertex2f(-.95, -.95);	
	glVertex2f(-.95,  .95);	
	glVertex2f( .95,  .95);	
	glVertex2f( .95, -.95);	
	glEnd();

	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glColor3f(1, 1, 1);

	glPushMatrix();
	glTranslatef(-.9, .9, 0);
	glScalef(1.8/s, 1.8/s, 1);
	glTranslatef(0, -help.get_font()->get_ascent(), 0);
	help.render();
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);

	reshaper.apply();
}

void DisplayCullers()
{
	for ( int c = 0; c < ncullers; c++ )
		{
		glPushMatrix();
		glTranslatef( posx[c], posy[c], posz[c] );
		glColor3f( 0.8, 0.8, 0.8 );
		glutSolidSphere(.015f, 10, 10 );
		glPopMatrix();
		}
}

void DrawCube()
{
  GLint mode;

  glGetIntegerv( GL_CULL_FACE_MODE, &mode );
  glCullFace( GL_FRONT );
  glEnable( GL_CULL_FACE );

  glBegin(GL_QUADS);

  // Red primary.
  glNormal3f(-1.0, 0.0, 0.0);
  glColor3f(0.40, 0.0, 0.0);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(-0.25, -0.25, 0.25);
  glVertex3f(-0.25, 0.25, 0.25);
  glVertex3f(-0.25, 0.25, -0.25);

  // Green primary.
  glNormal3f(1.0, 0.0, 0.0);
  glColor3f(0.00, 0.40, 0.00);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(0.25, -0.25, 0.25);
  glVertex3f(0.25, -0.25, -0.25);
  glVertex3f(0.25, 0.25, -0.25);

  // Blue primary.
  glNormal3f(0.0, -1.0, 0.0);
  glColor3f(0.00, 0.00, 0.40);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(0.25, -0.25, -0.25);
  glVertex3f(0.25, -0.25, 0.25);
  glVertex3f(-0.25, -0.25, 0.25);

  // Cyan primary.
  glNormal3f(0.0, 1.0, 0.0);
  glColor3f(0.00, 0.40, 0.40);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(0.25, 0.25, -0.25);
  glVertex3f(-0.25, 0.25, -0.25);
  glVertex3f(-0.25, 0.25, 0.25);

  // Yellow primary.
  glNormal3f(0.0, 0.0, -1.0);
  glColor3f(0.40, 0.40, 0.00);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(-0.25, 0.25, -0.25);
  glVertex3f(0.25, 0.25, -0.25);
  glVertex3f(0.25, -0.25, -0.25);

  // Purple primary.
  glNormal3f(0.0, 0.0, 1.0);
  glColor3f(0.40, 0.00, 0.40);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(-0.25, 0.25, 0.25);
  glVertex3f(-0.25, -0.25, 0.25);
  glVertex3f(0.25, -0.25, 0.25);

  glEnd();

  glDisable( GL_CULL_FACE );
  glCullFace( mode );
}

void DrawMiniCube()
{
  glBegin(GL_QUADS);

  // Red primary.
  glNormal3f(-1.0, 0.0, 0.0);
  glColor3f(0.90, 0.0, 0.0);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(-0.25, -0.25, 0.25);
  glVertex3f(-0.25, 0.25, 0.25);
  glVertex3f(-0.25, 0.25, -0.25);

  // Green primary.
  glNormal3f(1.0, 0.0, 0.0);
  glColor3f(0.00, 0.90, 0.00);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(0.25, -0.25, 0.25);
  glVertex3f(0.25, -0.25, -0.25);
  glVertex3f(0.25, 0.25, -0.25);

  // Blue primary.
  glNormal3f(0.0, -1.0, 0.0);
  glColor3f(0.00, 0.00, 0.90);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(0.25, -0.25, -0.25);
  glVertex3f(0.25, -0.25, 0.25);
  glVertex3f(-0.25, -0.25, 0.25);

  // Cyan primary.
  glNormal3f(0.0, 1.0, 0.0);
  glColor3f(0.00, 0.90, 0.90);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(0.25, 0.25, -0.25);
  glVertex3f(-0.25, 0.25, -0.25);
  glVertex3f(-0.25, 0.25, 0.25);

  // Yellow primary.
  glNormal3f(0.0, 0.0, -1.0);
  glColor3f(0.90, 0.90, 0.00);
  glVertex3f(-0.25, -0.25, -0.25);
  glVertex3f(-0.25, 0.25, -0.25);
  glVertex3f(0.25, 0.25, -0.25);
  glVertex3f(0.25, -0.25, -0.25);

  // Purple primary.
  glNormal3f(0.0, 0.0, 1.0);
  glColor3f(0.90, 0.00, 0.90);
  glVertex3f(0.25, 0.25, 0.25);
  glVertex3f(-0.25, 0.25, 0.25);
  glVertex3f(-0.25, -0.25, 0.25);
  glVertex3f(0.25, -0.25, 0.25);

  glEnd();
}

void DrawMiniCubes()
{
	glPushMatrix();

	float yscale = 0.5/height;
	float xscale = 0.5/width;
	float zscale = 0.5/depth;
	float yoffset = - 0.375*(height-1) - 0.75;
	float xoffset = - 0.375*(width-1) - 0.75;
	float zoffset = - 0.375*(depth-1) - 0.75;
	glScalef( xscale, yscale, zscale );
	glTranslatef( 0, 0, zoffset );
	for ( int k = 0; k < depth; k++ )
		{
		glTranslatef( 0, 0, 0.75 );
		glPushMatrix();
		glTranslatef( 0, yoffset, 0 );
		for ( int j = 0; j < height; j++ )
			{
			glTranslatef( 0, 0.75, 0 );
			glPushMatrix();
			glTranslatef( xoffset, 0, 0 );
			for ( int i = 0; i < width; i++ )
				{
				glTranslatef( 0.75, 0, 0 );
				DrawMiniCube();
				}
			glPopMatrix();
			}
		glPopMatrix();
		}

	glPopMatrix();
}

void display()
{
	// Store view-space position of cullers starting at constant register 14.
	const matrix4 &m = object.get_transform();
	for ( int c = 0; c < ncullers; c++ )
		{
		vec3 p( posx[c], posy[c], posz[c] );
		m.mult_matrix_vec( p );
		glProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 14+c, p[0], p[1], p[2], 1.0 );
		}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	camera.apply_inverse_transform();

	object.apply_transform();

    // Enable the vertex program.
    glBindProgramARB( GL_VERTEX_PROGRAM_ARB, vpid[ncullers-1] );
    glEnable( GL_VERTEX_PROGRAM_ARB );

	// Enable the texture shaders.
	glEnable( GL_TEXTURE_SHADER_NV );
	
	// Setup the texture shader state.
	ts_state->call_list();

	glColor3f( 1.0, 1.0, 1.0 );

	DrawMiniCubes();

	glDisable( GL_TEXTURE_SHADER_NV );
	glDisable( GL_VERTEX_PROGRAM_ARB );

	if ( b['b'] )
		DrawCube();

	if ( b['c'] )
		{
		glEnable( GL_LIGHTING );
		DisplayCullers();
		glDisable( GL_LIGHTING );
		}

	glPopMatrix();


#if 0
    int errCode = glGetError();
	if ( errCode )
	{
        const GLubyte *errString = gluErrorString( errCode );
        fprintf( stderr, "OpenGL Error: %s\n", errString );
		exit(-1);
	}
#endif

	if(b['h'])
		display_help();

	glutSwapBuffers();
}
