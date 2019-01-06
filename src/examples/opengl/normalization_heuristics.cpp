#if defined(WIN32)
#  include <windows.h>
#endif

#include <glh/glh_glut.h>
#include <glh/glh_glut_text.h>

#include <string>

#include "head.h"
#include "stopwatch.h"

#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor object;

CGcontext cgContext; 
Head      head;
Stopwatch timer;

// settings
int   iFrameCount = 0;
int   iMaxMipLevel = 8;

bool  bBenchMark = false;
unsigned int iBMarkState = 0;

string helpstring = "Normalization Heuristics\n\n"
                    "[Mouse]: Rotate.\n"
                    "[CTRL + Mouse]: Zoom.\n"
                    "[SHIFT + Mouse]: translate.\n\n"
                    "Keys:\n";
char legend[400];

void addUI(char key, string description, bool initialValue = false)
{
    b[key] = initialValue;
    char *desc = new char[description.length() + 5];
    sprintf(desc, "%s [%c]", description.c_str(), key);
    glutAddMenuEntry(desc, key);
    helpstring.append(desc); helpstring.append("\n");
}

void cgErrorCallback()
{
    CGerror lastError = cgGetError();

    if(lastError)
    {
        printf("%s\n\n", cgGetErrorString(lastError));
        printf("%s\n", cgGetLastListing(cgContext));
        printf("Cg error, exiting...\n");
        quitapp(-2);
    }
} 

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.10, 0.30, 0.40, 1);

    // Create cgContext.
    cgContext = cgCreateContext();
    cgSetErrorCallback(cgErrorCallback);

    if (!head.Initialize(cgContext))
        quitapp(-1);

    object.dolly.dolly += vec3f(0, 0, -1.0f);
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


void updateLegend()
{
    sprintf(legend, 
        "Cubemaps: %c %c %c %c\n"
        "Shader Precision: %df\n"
        "Cubemap Precision: %2d-bit\n"
        "Dot Optimization %3s\n"
        "Mipmaps %s, max level %d\n"
        "Disable legend [t] for max fps.",
        b['v'] ? 'V' : ' ', b['l'] ? 'L' : ' ', 
        b['h'] ? 'H' : ' ', b['n'] ? 'N' : ' ',
        b['f'] ? 32 : 16,
        b['8'] ? 8 : 16,
        b['o'] ? "on" : "off",
        b['m'] ? "on" : "off", 
        iMaxMipLevel);
}

void setBenchmark()
{
    iMaxMipLevel = 3;
    head.SetMaxMipLevel(iMaxMipLevel);

    b['v'] = (iBMarkState & 0x1)   != 0; // V cubemap
    b['l'] = (iBMarkState & 0x2)   != 0; // L cubemap
    b['h'] = (iBMarkState & 0x4)   != 0; // H cubemap
    b['n'] = (iBMarkState & 0x8)   != 0; // N cubemap
    b['m'] = (iBMarkState & 0x10)  != 0; // mipmaps
    b['f'] = (iBMarkState & 0x20)  != 0; // 32-bit floats
    b['8'] = (iBMarkState & 0x40)  == 0; // 16-bit HILO cubemaps
    head.ResetFragmentProgram(b['v'], b['l'], b['h'], b['n'], !b['8'], b['o']);
    updateLegend();
}

void display()
{
    if (!bBenchMark)
        timer.Reset();
    else
    {
        timer.Start();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, b['w'] ? GL_LINE : GL_FILL);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -0.5f);
    object.apply_transform();
   
    head.Display(b['8'] ? Head::HEAD_DISPLAY_STD : Head::HEAD_DISPLAY_HILO);

    if (b['t'])
        text(5, 70, legend);

    glutSwapBuffers();

    iFrameCount++;

    if (bBenchMark)
    {
        if (iFrameCount == 100)
        {
            timer.Stop();
            float fps = 1.0f / timer.GetAvgTime();
            if (iBMarkState == 0x00 ||
                iBMarkState == 0x10 || iBMarkState == 0x20 || iBMarkState == 0x30 ||
                iBMarkState == 0x40 || iBMarkState == 0x50 || iBMarkState == 0x60 ||
                iBMarkState == 0x70)
                printf("%c,%d,%d\n", b['m']?'m':' ', b['f']?32:16, b['8']?8:16);
            iBMarkState++;
            setBenchmark();
            iFrameCount = 0;
            timer.Reset();
            printf("%f,\n", fps);
        }
        if (iBMarkState >= 128)
        {
            iBMarkState = 0;
            bBenchMark = false;
            timer.Start();
        }
    }
    else if (0 == iFrameCount % 50)
    {
        char title[100];
        sprintf(title, "Normalization | %f FPS", 1.0f / timer.GetTime());
        glutSetWindowTitle(title);
    }
}

void idle()
{
    if (b[' '])
        object.trackball.increment_rotation();
    
    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if (27==k || 'q'==k) exit(0);
    if ('?'==k)  printf(helpstring.c_str());

    switch(k)
    {
    case 'f':
        head.ToggleHalfPrecision(); 
        // intentional fallthrough...
    case 'v':
    case 'l':
    case 'h':    
    case 'n':
    case 'o':
    case '8':
        head.ResetFragmentProgram(b['v'], b['l'], b['h'], b['n'], !b['8'], b['o']);
        break;
    case 'm':
        head.ToggleNormalizationMipMap();
        break;
    case '-':
        iMaxMipLevel--;
        if (iMaxMipLevel < 0) iMaxMipLevel = 0;
        head.SetMaxMipLevel(iMaxMipLevel);
        break;
    case '+':
    case '=':
        iMaxMipLevel++;
        if (iMaxMipLevel > 8) iMaxMipLevel = 8;
        head.SetMaxMipLevel(iMaxMipLevel);
        break;
    case 'p':
        head.PrintCurrentFragmentProgram(b['8'] ? Head::HEAD_DISPLAY_STD : Head::HEAD_DISPLAY_HILO);
        break;
    case 'B':
        timer.Stop();
        timer.Reset();
        bBenchMark = true;
        iBMarkState = 0;
        setBenchmark();
        iFrameCount = 0;
        break;
    default:
        break;
    }

    object.keyboard(k, x, y);

    updateLegend();

    glutPostRedisplay();
}

void menu(int item) 
{ 
    key((unsigned char)item, 0, 0); 
}

void resize(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
    object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    object.motion(x, y);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("Normalization");

	init_opengl();

    object.configure_buttons(1);
    object.dolly.dolly[2] = -2;

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    glutCreateMenu(menu);
    addUI('w', "Toggle wireframe", false);
    addUI(' ', "Toggle animation", true);
    addUI('v', "Toggle V cubemap", false);
    addUI('l', "Toggle L cubemap", false);
    addUI('h', "Toggle H cubemap", false);
    addUI('n', "Toggle N cubemap", false);   
    addUI('o', "Toggle Dot Product Optimization", false);
    addUI('f', "Toggle 16-/32-bit shader precision", false);
    addUI('8', "Toggle 8-/16-bit[HILO] cube maps", true);
    addUI('m', "Toggle mipmapped cube maps", false);
    addUI('=', "Increase maximum mip level");
    addUI('-', "Decrease maximum mip level");
    addUI('p', "Print current fragment program");
    addUI('t', "Display text legend", true);
    addUI('?', "Print help");
    addUI('q', "Quit");
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    key('~', 0, 0); // to initialize the legend text

    timer.Start();
	glutMainLoop();

	return 0;
}
