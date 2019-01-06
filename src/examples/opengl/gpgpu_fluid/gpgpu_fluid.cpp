#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_glut_text.h>

#include <Cg/cgGL.h>
#include <shared/quitapp.h>
#include <stopwatch.h>
#include <paramgl.h>
#include <minmax.h>

#include <shared/MovieMaker.h>

#include "flo.h"

using namespace glh;

bool b[256];

CGcontext g_cgContext; 

MovieMaker g_movie;

bool  g_bVectorVisualizationAvailable = true;

// simulation constants
int   g_iFloWidth  = 64;
int   g_iFloHeight = 64;

// The UI.
ParamListGL *paramlist;  // parameter list

// interaction stuff:
int   g_iMouseButton = 0;
bool  g_bMouseDown = false;
bool  g_bCtrlDown = false; 
bool  g_bAltDown = false; 

int   g_iMouseX = 0;
int   g_iMouseY = 0;

// The simulator
Flo  *g_pFlo = 0;

// Booleans for simulation behavior
Flo::DisplayMode g_displayMode = Flo::DISPLAY_MILK_AND_METAL;
Flo::VectorDisplayMode g_vectorDisplayMode = Flo::VECTOR_DISPLAY_NONE;
Flo::DisplayMode g_vectorField = Flo::DISPLAY_VELOCITY;

// parameters
float g_rVCScale          = 0.025f;
int   g_iNumPoissonSteps  = 50;

float g_rViscosity        = 0;
float g_rInkRGB[3]        = { 0.54f, 0.2f, 0.0f };
float g_rInkLongevity     = 0.997;
float g_rBrushRadius      = 0.1;
float g_rTimestep         = 1;
float g_rGridScale        = 1;
int   g_iPointSize        = 3;
int   g_iLineWidth        = 2;
float g_rVectorScale      = 0.05;
int   g_iPointStride      = 4;

// benchmark globals
Stopwatch g_timer;
Stopwatch g_perfTimer;
float g_rLastTime = 0;
int   g_iFrameCount = 0;
int   g_iTimeDuration = 100;
bool  g_bExitAfterBenchmark = false;
bool  g_bQuiet = false;
bool  g_bStartup = true;

string helpstring = "GPGPU Fluid Dynamics Simulation\n\n"
"[Left Mouse + Motion]: Inject ink and velocity.\n"
"[Right Mouse]: Show Menu.\n"
"[CTRL + Left Mouse]: Draw obstacles (when enabled).\n"
"[ALT + Left Mouse]: Erase obstacles (when enabled).\n\n"
"Keys:\n";

char legend[400];

void addUI(char key, string description, bool initialValue = false)
{
	b[key] = initialValue;
	char *desc = new char[description.length() + 5];
	sprintf(desc, "[%c] %s", key, description.c_str());
	glutAddMenuEntry(desc, key);
	helpstring.append(desc); helpstring.append("\n");
}

void cgErrorCallback()
{
	CGerror lastError = cgGetError();

	if(lastError)
	{
		printf("%s\n\n", cgGetErrorString(lastError));
		printf("%s\n", cgGetLastListing(g_cgContext));
		printf("Cg error, exiting...\n");
		quitapp(-2);
	}
} 


void getExtensions(void)
{
	//printf("%s\n", glGetString(GL_EXTENSIONS) );

	if (!glh_init_extensions("GL_ARB_multitexture "
		"GL_VERSION_1_4 "
		"WGL_ARB_pbuffer "
		"WGL_ARB_pixel_format "
		"WGL_ARB_render_texture "
		"GL_NV_float_buffer "
		)) 
	{
		fprintf(stderr, "Error - required extensions were not supported: %s", glh_get_unsupported_extensions());
		quitapp(-1);
	}

    if (!glh_init_extensions("GL_ARB_vertex_buffer_object "
                             "GL_NV_vertex_program3 "))
    {
        g_bVectorVisualizationAvailable = false;
    }
    
}

//----------------------------------------------------------------------------
// Function     	: InitParameters
// Description	    : 
//----------------------------------------------------------------------------
/**
* @fn InitParameters()
* @brief Initializes the UI Sliders
*/ 
void initParameters()
{
	// create a new parameter list
	paramlist = new ParamListGL("misc");
	paramlist->bar_col_outer[0] = 0.8f;
	paramlist->bar_col_outer[1] = 0.8f;
	paramlist->bar_col_outer[2] = 0.0f;
	paramlist->bar_col_inner[0] = 0.8f;
	paramlist->bar_col_inner[1] = 0.8f;
	paramlist->bar_col_inner[2] = 0.0f;

	// add some parameters to the list

	// How many iterations to run the poisson solvers.  
	paramlist->AddParam(new Param<int>("Solver Iterations", g_iNumPoissonSteps, 
		1, 500, 1, &g_iNumPoissonSteps));
	// The size of the time step taken by the simulation
	paramlist->AddParam(new Param<float>("Time step", g_rTimestep, 
		0.1f, 10, 0.1f, &g_rTimestep));
	// The Grid Cell Size
	paramlist->AddParam(new Param<float>("Grid Scale", g_rGridScale, 
		0.1f, 100, 0.1f, &g_rGridScale));
	// Scales the vorticity confinement force.
	paramlist->AddParam(new Param<float>("Vort. Conf. Scale", g_rVCScale, 
		0, .25, 0.005f, &g_rVCScale));
	// The viscosity ("thickness") of the fluid.
	paramlist->AddParam(new Param<float>("Viscosity", g_rViscosity, 
		0, 0.5f, 0.0001f, &g_rViscosity));

	// How slow or fast the Ink fades.  1 = does not fade.
	paramlist->AddParam(new Param<float>("Ink Longevity", g_rInkLongevity, 
		0.99f, 1, 0.0001, &g_rInkLongevity));
	// The size of the "brush" the user draws with
	paramlist->AddParam(new Param<float>("Brush Radius", g_rBrushRadius, 
		0.005, .25, .005, &(g_rBrushRadius)));
	// The Ink color, RGB.
	paramlist->AddParam(new Param<float>("Ink Red", g_rInkRGB[0], 
		0.0, 1.0, 0.01f, &(g_rInkRGB[0])));
	paramlist->AddParam(new Param<float>("    Green", g_rInkRGB[1], 
		0.0, 1.0, 0.01f, &(g_rInkRGB[1])));
	paramlist->AddParam(new Param<float>("    Blue", g_rInkRGB[2], 
		0.0, 1.0, 0.01f, &(g_rInkRGB[2])));

    if (g_bVectorVisualizationAvailable)
    {
        paramlist->AddParam(new Param<int>("Line Width", g_iLineWidth,
                            1, 10, 1, &(g_iLineWidth)));
        paramlist->AddParam(new Param<float>("Vector Scale", g_rVectorScale,
                            0.0f, 0.25f, 0.05f, &(g_rVectorScale)));
        paramlist->AddParam(new Param<int>("Point Size", g_iPointSize,
                            1, 10, 1, &(g_iPointSize)));
    }

	g_perfTimer.Reset();
}


void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

	getExtensions();

	// Initialize the UI
	initParameters();

	// Create the Cg context
	cgSetErrorCallback(cgErrorCallback);

	// Create cgContext.
	g_cgContext = cgCreateContext();

	// Create and initialize the Flo simulator object
	g_pFlo = new Flo(g_iFloWidth, g_iFloHeight);
	g_pFlo->Initialize(g_cgContext, g_bVectorVisualizationAvailable);
	g_pFlo->EnableTextureUpdates(b['U']);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
}

void shutdown()
{
	g_pFlo->Finalize();

	delete g_pFlo;
	g_pFlo = 0;
}

void text(int x, int y, char *buffer)
{    
    int WW = glutGet((GLenum)GLUT_WINDOW_WIDTH);
    int WH = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WW, 0, WH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int fontHeight = 13;

    if (x>=0 && y>=0)
        glRasterPos2i(x,y);

    for (char *p=buffer; *p; p++)
    {
        if (*p=='\n' && x >= 0 && y >= 0) 
        { 
            y -= fontHeight; 
            glRasterPos2i(x,y); 
        } 
        else
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void jet()
{   
	float oldColor[3];
	g_pFlo->GetInkColor(oldColor);

	if (b['1'])
	{
		float color[3] = {1, 0, 0};
		float strength[3] = {g_rGridScale * 5, 0, 0 };
		float pos[2] = {0.01f, 0.5f};
		g_pFlo->SetInkColor(color);    
		g_pFlo->AddImpulse(strength, pos, 5 * g_rBrushRadius, true);
	}
	if (b['2'])
	{
		float color[3] = {0, 1, 0};
		float strength[3] = {-g_rGridScale * 5, 0, 0 };
		float pos[2] = {0.99f, 0.5f};
		g_pFlo->SetInkColor(color);
		g_pFlo->AddImpulse(strength, pos, 5 * g_rBrushRadius, true);
	}
	if (b['3'])
	{
		float color[3] = {0, 0, 1};
		float strength[3] = {0, g_rGridScale * 5, 0};
		float pos[2] = {0.5f, 0.01f};
		g_pFlo->SetInkColor(color);
		g_pFlo->AddImpulse(strength, pos, 5 * g_rBrushRadius, true);
	}
	if (b['4'])
	{
		float color[3] = {0.5, 0.5, 0};
		float strength[3] = {0, -g_rGridScale * 5, 0};
		float pos[2] = {0.5f, 0.99f};
		g_pFlo->SetInkColor(color);
		g_pFlo->AddImpulse(strength, pos, 5 * g_rBrushRadius, true);
	}

	g_pFlo->SetInkColor(oldColor);
}


void display()
{
    if (g_perfTimer.GetTime() > 5)
        g_bStartup = false;

    g_pFlo->SetInkColor(g_rInkRGB);

	if (!b['p'] || b['.'])
	{
		b['.'] = false;

        // cycle ink color
        if (b['c'] && (g_timer.GetTime() - g_rLastTime > 0.25f))
        {
            g_rLastTime = g_timer.GetTime();
            static int sColor = 0;
            sColor = (sColor + 1) % 7;
            g_rInkRGB[0] = 0.15f * (sColor & 0x1) +
                           0.05f * ((sColor & 0x2) / 2);
            g_rInkRGB[1] = 0.15f * ((sColor & 0x2) / 2) +
                           0.05f * ((sColor & 0x4) / 4);
            g_rInkRGB[2] = 0.15f * ((sColor & 0x4) / 4) +
                           0.05f * (sColor & 0x1);
            if (sColor > 699) sColor = 0;
        }

		// set parameters that may have changed
		g_pFlo->SetViscosity(g_rViscosity);
		g_pFlo->EnablePressureClear(b['C']);
		g_pFlo->SetNumPoissonSteps(g_iNumPoissonSteps);
		g_pFlo->SetInkColor(g_rInkRGB);
		g_pFlo->SetInkLongevity(g_rInkLongevity);
		g_pFlo->SetTimeStep(g_rTimestep);
		g_pFlo->SetGridScale(g_rGridScale);
		g_pFlo->SetVorticityConfinementScale(g_rVCScale);

		if (g_displayMode == Flo::DISPLAY_VORTICITY || b['v'])
			g_pFlo->EnableVorticityComputation(true);
		else
			g_pFlo->EnableVorticityComputation(false);

		// For benchmarking...
		if (b['L'])
		{
			if (g_perfTimer.GetNumStarts() >= g_iTimeDuration)
			{
				b['L'] = false;
				g_perfTimer.Stop();

				if (!g_bQuiet)
					printf("Average iteration time: ");
				printf("%f\n", g_perfTimer.GetAvgTime());

				if (g_bExitAfterBenchmark)
					quitapp(0);
			}
			g_perfTimer.Start();
		}

		// Take a simulation timestep.
		g_pFlo->Update();
	}

	if (b['D'])
	{
		// Display the fluid.
        if (b['0'])
		    g_pFlo->Display(g_displayMode, b['b'], b['m'], b['o'], b['w']);
        else
            glClear(GL_COLOR_BUFFER_BIT);

        if (g_vectorDisplayMode != Flo::VECTOR_DISPLAY_NONE)
            g_pFlo->DisplayVectorField(g_vectorDisplayMode, g_iLineWidth,
                                       g_iPointSize, g_rVectorScale);

		// Display user interface.
		if (b['`'])
		{
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO); // invert color
            glEnable(GL_BLEND);
			paramlist->Render(0, 0);
            glDisable(GL_BLEND);
		}

        if (g_bStartup && !b['L'])
        {
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO); // invert color
            glEnable(GL_BLEND);
            text(100, glutGet(GLUT_WINDOW_HEIGHT) / 2,
                 "Use the mouse to paint fluid.\n"
                 "Press 'j' to inject ink.\n" 
                 "Press 't' to cycle display modes.\n"
                 "Press '~' to adjust parameters.\n"
                 "Right-click for menu.\n"
                 );
            glDisable(GL_BLEND);
        }

		glutSwapBuffers();

        if (b['M'])
            g_movie.Snap();
	}

	// Frame rate update
	if (b['!'])
	{
		g_iFrameCount++;

		if (g_timer.GetTime() > 0.5)
		{  
			char title[100];
			sprintf(title, "gpgpu_fluid: %f FPS", 
				g_iFrameCount / g_timer.GetTime());
			glutSetWindowTitle(title);

			g_iFrameCount = 0;
			g_timer.Reset();
            g_rLastTime = g_timer.GetTime();
		}
	}

}

void idle()
{
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
	paramlist->Special(key, x, y);
	glutPostRedisplay();
}


void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') 
	{
		shutdown();
		exit(0);
	}
	if ('?'==k)  printf(helpstring.c_str());

	switch(k)
	{
    case '!':
        if (!b['!'])
            glutSetWindowTitle("gpgpu_fluid");
        break;
    case 'M':
        if (b['M'])
            g_movie.StartCapture("gpgpu_fluid.avi");
        else
            g_movie.EndCapture();
        break;
	case 'o':
		g_pFlo->EnableArbitraryBC(b['o']);
		break;
	case 'C':
		g_pFlo->EnablePressureClear(b['C']);
		break;
	case 'U':
		g_pFlo->EnableTextureUpdates(b['U']);
		break;
	case 'r':
		g_pFlo->Reset();
		break;
	case 'R':
		g_pFlo->Reset(true);
		break;
	case 'v':
		g_pFlo->EnableVCForce(b['v']);
		break;
	case 't':
		g_displayMode = static_cast<Flo::DisplayMode>(((g_displayMode + 1) 
			% Flo::DISPLAY_COUNT));
		break;
    case 'f':
        g_vectorDisplayMode = static_cast<Flo::VectorDisplayMode>(((g_vectorDisplayMode + 1) 
            % Flo::VECTOR_DISPLAY_COUNT));
        g_pFlo->EnableVectorFieldUpdates(g_vectorDisplayMode != Flo::VECTOR_DISPLAY_NONE);
        break;
    case 'F':
        g_iPointStride *= 2;
        if (g_iPointStride > std::max<int>(g_iFloWidth, g_iFloHeight) / 4)
            g_iPointStride = 1;
        g_pFlo->InitializeVertexBuffer(g_iFloWidth / g_iPointStride, g_iFloHeight / g_iPointStride);
        break;
    case 'g':
        g_vectorField = static_cast<Flo::DisplayMode>(((g_vectorField + 1) 
            % Flo::DISPLAY_MILK_AND_METAL));
        g_pFlo->SetVectorField(g_vectorField);
        break;
	case 'L':
		g_perfTimer.Stop();
		g_perfTimer.Reset();
		break;
	case 'j':
    case 'J':
		jet();
		break;
    case 'z':
        g_pFlo->EnableZCullOpt(b['z']);
        break;
	default:
		break;
	}
    
	glutPostRedisplay();
}

void menu(int item) 
{ 
	key((unsigned char)item, 0, 0); 
}


void reshape(int w, int h)
{  
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y)
{
	// We get the y coordinate in normal window system coordinates,
	// which is upside down from the point of view of OpenGL,
	// so subtract it from the height of the window.  
	if( state == GLUT_DOWN )
	{
		g_iMouseButton = button;
		// If the left mouse button has just been pressed down,
		// set our global variables that keep track of the coordinates.
		g_iMouseX = x;
		g_iMouseY = glutGet(GLUT_WINDOW_HEIGHT) - y - 1;
		g_bMouseDown = true;
	}
	else
	{
		g_iMouseButton = 0;
		g_bMouseDown = false;
	}

	if (b['`']&& g_bMouseDown) 
	{
		// call list mouse function
		paramlist->Mouse(x, y);
	}

	g_bCtrlDown = (state == GLUT_DOWN) && (glutGetModifiers() == GLUT_ACTIVE_CTRL);
    g_bAltDown  = (state == GLUT_DOWN) && (glutGetModifiers() == GLUT_ACTIVE_ALT);
	
    if (g_bCtrlDown || g_bAltDown)
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	else
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void motion(int x, int y)
{
	const int maxRadius = 1; // in pixels

	if (b['`'] && g_bMouseDown) 
	{
		// call parameter list motion function
		paramlist->Motion(x, y);
		glutPostRedisplay();
		return;
	}

	y = glutGet(GLUT_WINDOW_HEIGHT) - y - 1;
	if(g_bMouseDown)
	{
		// Force is applied by dragging with the left mouse button
		// held down.

		float dx, dy;
		dx = x - g_iMouseX;
		dy = y - g_iMouseY;

		// clamp to some range
		float strength[3] = 
		{
			min( max(dx, -maxRadius * g_rGridScale), maxRadius * g_rGridScale),
				min( max(dy, -maxRadius * g_rGridScale), maxRadius * g_rGridScale),
				0
		};

		float pos[2] = 
		{
			x / (float) glutGet(GLUT_WINDOW_WIDTH),
				y / (float) glutGet(GLUT_WINDOW_HEIGHT)
		};

		if (g_iMouseButton == GLUT_LEFT_BUTTON)
		{
			if (b['o'] && g_bCtrlDown)
				g_pFlo->DrawBoundary(pos, 10 / (float) glutGet(GLUT_WINDOW_WIDTH));
            else if (b['o'] && g_bAltDown)
                g_pFlo->DrawBoundary(pos, 10 / (float) glutGet(GLUT_WINDOW_WIDTH), true);
			else
				g_pFlo->AddImpulse(strength, pos, g_rBrushRadius, true);
		}
		else if (g_iMouseButton == GLUT_RIGHT_BUTTON)
		{
            g_pFlo->AddImpulse(strength, pos, g_rBrushRadius, false);
		}
	}

	g_iMouseX = x;
	g_iMouseY = y;
}


void parseCommandLine(int argc, char **argv)
{
	for (int iArg = 1; iArg < argc; iArg++)
	{
		if (strstr(argv[iArg], "-b"))
		{
			if (argv[iArg][2] == 'x')
			{
				g_bExitAfterBenchmark = true;
			}
			b['L'] = true;
			g_iTimeDuration = atoi(argv[++iArg]);
		}
		else if (!strcmp(argv[iArg], "-d"))
		{
			b['D'] = (atoi(argv[++iArg]) != 0);
		}
		else if (!strcmp(argv[iArg], "-j"))
		{
			g_iNumPoissonSteps = atoi(argv[++iArg]);
		}
		else if (!strcmp(argv[iArg], "-q"))
		{
			g_bQuiet = true;
		}
		else if (!strcmp(argv[iArg], "-r"))
		{
			g_iFloWidth = g_iFloHeight = atoi(argv[++iArg]);
		}
		else if (!strcmp(argv[iArg], "-u"))
		{
			b['U'] = (atoi(argv[++iArg]) != 0);
		}
	}
}


int main(int argc, char **argv)
{
	// Window size.
	const int width = 512, height = 512;

	glutInit(&argc, argv);
	glutInitWindowSize(width, height);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_ALPHA);
	glutCreateWindow("gpgpu_fluid");

	glutCreateMenu(menu);
	addUI('!', "Toggle FPS Display", false);
    addUI('`', "Toggle Sliders", false);
    addUI('?', "Print help text");
    addUI('1', "Enable jet 1", true);
    addUI('2', "Enable jet 2", true);
    addUI('3', "Enable jet 3", true);
    addUI('4', "Enable jet 4", true);
    addUI('b', "Toggle bilinear interpolation", true);    
    addUI('c', "Toggle ink color cycling", true);    
    addUI('C', "Toggle pressure Clears", false);
    addUI('D', "Toggle display updates", true);
    addUI('j', "Fire ink jets");
    addUI('L', "Time performance", false);
    addUI('m', "Toggle 8-bit display texture (fastest)", false);
    addUI('o', "Enable painting interior obstacles", false); 
    addUI('p', "Toggle Pause", false);
    addUI('.', "Take a single step");
    addUI('r', "Reset fluid");
    addUI('R', "Reset fluid and obstacles");
    addUI('t', "Cycle display field");
    addUI('U', "Toggle texture Updates", true);
    addUI('v', "Toggle vorticity computation", true);
    addUI('w', "Toggle white background", true);
    addUI('z', "Toggle zcull optimization", false);
    addUI('f', "Toggle Vector Field Visualization", false);
    addUI('F', "Cycle Vector Field Point Stride", false);
    addUI('g', "Cycle texture to use for vector field", false);
    addUI('0', "Toggle fluid textures", true);
  	glutAttachMenu(GLUT_RIGHT_BUTTON);

    // optionally set the resolution and benchmark settings
    parseCommandLine(argc, argv);

	init_opengl();

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(reshape);
	glutSpecialFunc(special);

	if (!g_bQuiet)
	{
		system("cls");
		printf(helpstring.c_str());
	}

	// start the framerate timer.
	g_timer.Start();
    g_perfTimer.Start();

	glutMainLoop();

	return 0;
}