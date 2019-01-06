#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <string>

#include "diseasehead.h"
#include "reactiondiffusion.h"

#include <stopwatch.h>
#include <paramgl.h>
#include <shared/quitapp.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor g_object;
bool bLeftButtonDown = false;

CGcontext g_cgContext; 
Head      g_head;
ReactionDiffusion *g_pRD;
Stopwatch g_timer;

unsigned int g_eDisplayMode = Head::HEAD_DISPLAY_BLINN;

// settings
int   g_iFrameCount = 0;

float g_rBumpScale           = 1.0f;
float g_rDiseaseBumpScale    = 2.0f;
int   g_iDiseaseMapScale     = 4;
float g_rDiseaseColor[3]     = { 0.4f, 0.2f, 0.2f };
int   g_iUpdatesPerFrame     = 3;
float g_rRDk                 = 0.052f;
float g_rRDf                 = 0.012f;

float g_rLightPos[3]         = { 10, 10, 10 };

float g_rRDConstantIncrement = 0.001f;

ParamListGL *g_pParamList;  // parameter list

int g_iCurrentPreset = 0;
float g_presets[] = { 4, 0.0520f, 0.0120f,
                      6, 0.0500f, 0.0176f,
                      6, 0.0507f, 0.0208f,
                      5, 0.0500f, 0.0100f,
                      5, 0.0507f, 0.0230f,
                      6, 0.0618f, 0.0611f };

string g_helpstring = "GPGPU Disease\n\n"
                    "[Mouse]: Rotate.\n"
                    "[CTRL + Mouse]: Zoom.\n"
                    "[SHIFT + Mouse]: translate.\n\n"
                    "Keys:\n";

void addUI(char key, string description, bool initialValue = false)
{
    b[key] = initialValue;
    char *desc = new char[description.length() + 5];
    sprintf(desc, "%s [%c]", description.c_str(), key);
    glutAddMenuEntry(desc, key);
    g_helpstring.append(desc); 
    g_helpstring.append("\n");
}

void cgErrorCallback()
{
    CGerror lastError = cgGetError();

    if(lastError)
    {
        printf("%s\n\n", cgGetErrorString(lastError));
        printf("%s\n", cgGetLastListing(g_cgContext));
        printf("Cg error, exiting...\n");
        quitapp(0);
    }
} 

void getExtensions(void)
{
    //printf("%s\n", glGetString(GL_EXTENSIONS) );

    if (!glh_init_extensions("GL_ARB_multitexture "
        "GL_VERSION_1_4 "
        "WGL_ARB_pbuffer "
        "WGL_ARB_pixel_format "
        "GL_NV_float_buffer "
        "GL_ARB_vertex_buffer_object "
        )) 
    {
        fprintf(stderr, "Error - required extensions were not supported: %s", glh_get_unsupported_extensions());
        quitapp(-1);
    }
}

void initParameters()
{
    // create a new parameter list
    g_pParamList = new ParamListGL("misc");
    g_pParamList->bar_col_outer[0] = 0.8f;
    g_pParamList->bar_col_outer[1] = 0.4f;
    g_pParamList->bar_col_outer[2] = 0.0f;
    g_pParamList->bar_col_inner[0] = 0.2f;
    g_pParamList->bar_col_inner[1] = 0.4f;
    g_pParamList->bar_col_inner[2] = 0.8f;
    g_pParamList->text_col_unselected[0] = 0.2f;
    g_pParamList->text_col_unselected[1] = 0.4f;
    g_pParamList->text_col_unselected[2] = 0.8f;
    g_pParamList->text_col_selected[0] = 0.8f;
    g_pParamList->text_col_selected[1] = 0.4f;
    g_pParamList->text_col_selected[2] = 0.0f;

    // add some parameters to the list
    // function parameters are: name, default value, minimum, maximum, step, pointer to variable (optional)
    g_pParamList->AddParam(new Param<float>("Bump Scale", g_rBumpScale, 
        0.1, 4.0, 0.1, &g_rBumpScale));
    g_pParamList->AddParam(new Param<float>("Disease Bump Scale", 
        g_rDiseaseBumpScale, 0.1, 4.0, 0.1, 
        &g_rDiseaseBumpScale));
    g_pParamList->AddParam(new Param<int>("Disease Map Scale", g_iDiseaseMapScale, 
        1, 16, 1, &g_iDiseaseMapScale));
    g_pParamList->AddParam(new Param<float>("Disease Color Red", g_rDiseaseColor[0], 
        0.0, 1.0, 0.01, &(g_rDiseaseColor[0])));
    g_pParamList->AddParam(new Param<float>("              Green", g_rDiseaseColor[1], 
        0.0, 1.0, 0.01, &(g_rDiseaseColor[1])));
    g_pParamList->AddParam(new Param<float>("              Blue", g_rDiseaseColor[2], 
        0.0, 1.0, 0.01, &(g_rDiseaseColor[2])));

    g_pParamList->AddParam(new Param<int>("R-D Iters/frame", g_iUpdatesPerFrame, 
        1, 50, 1, &g_iUpdatesPerFrame));
    g_pParamList->AddParam(new Param<float>("R-D K", g_rRDk, 
        0.01, 0.1, 0.0001, &g_rRDk));
    g_pParamList->AddParam(new Param<float>("R-D F", 
        g_rRDf, 0.01, 0.1, 0.0001, &g_rRDf));

    g_pParamList->AddParam(new Param<float>("Light X", g_rLightPos[0], 
        -25, 25, 1, &(g_rLightPos[0])));
    g_pParamList->AddParam(new Param<float>("      Y", g_rLightPos[1], 
        -25, 25, 1, &(g_rLightPos[1])));
    g_pParamList->AddParam(new Param<float>("      Z", g_rLightPos[2], 
        -25, 25, 1, &(g_rLightPos[2])));
}

void init_opengl()
{
    getExtensions();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.80, 0.80, 0.80, 1);

    // Create cgContext.
    g_cgContext = cgCreateContext();
    cgSetErrorCallback(cgErrorCallback);

    g_head.Initialize(g_cgContext);
    g_object.dolly.dolly += vec3f(0, 0, -1.0f);

    g_pRD->Initialize(g_cgContext);

    int x, y;
    g_pRD->GetResolution(x, y);
    g_head.SetDiseaseTextures(g_pRD->GetDisplayTexture(), g_pRD->GetNormalMap(), x, y);

    initParameters();
}

void SetNextPreset()
{
    g_iCurrentPreset = (g_iCurrentPreset + 1) % (sizeof(g_presets) / (3 * sizeof(float)));
    g_iDiseaseMapScale = g_presets[3 * g_iCurrentPreset];
    g_rRDk = g_presets[3 * g_iCurrentPreset + 1];
    g_rRDf = g_presets[3 * g_iCurrentPreset + 2];
}

void SetDiseaseParameters()
{
    // update head and reaction diffusion parameters
    g_head.SetBumpScale(g_rBumpScale);
    g_head.SetDiseaseBumpScale(g_rDiseaseBumpScale);
    g_head.SetDiseaseMapScale(g_iDiseaseMapScale);
    g_head.SetDiseaseColor(g_rDiseaseColor);
    g_head.SetLightPosition(g_rLightPos);

    g_pRD->SetF(g_rRDf);
    g_pRD->SetK(g_rRDk);
}

void display()
{
    SetDiseaseParameters();

    g_timer.Reset();

    if (b[' '] || b['.'])
    {
        b['.'] = false;
        for (int i = 0; i < g_iUpdatesPerFrame; ++i)
            g_pRD->Update();
        g_pRD->GenerateDisplayMaps();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, b['w'] ? GL_LINE : GL_FILL);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -0.5f);
    g_object.apply_transform();

    if (b['h'])
        g_head.Display((Head::DisplayMode)g_eDisplayMode);

    if (b['d'])
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
        g_pRD->Display();
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    if (b['`'])
    {
        glColor3f(1.0, 1.0, 0.0);
        g_pParamList->Render(0, 0);
    }

    glutSwapBuffers();

    g_iFrameCount++;

    if (0 == g_iFrameCount % 50)
    {
        char title[100];
        sprintf(title, "GPGPU Disease | %f FPS", 1.0f / g_timer.GetTime());
        glutSetWindowTitle(title);
    }
}

void idle()
{
    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if (27==k || 'q'==k) exit(0);
    if ('?'==k)  printf(g_helpstring.c_str());

    switch(k)
    {
    case 't':
        g_eDisplayMode = (g_eDisplayMode + 1) % (unsigned int)Head::HEAD_DISPLAY_NUM_MODES;
        break;
    case 'r':
        g_pRD->Reset();
        break;
    case 'p':
        SetNextPreset();
        break;
    default:
        break;
    }

    g_object.keyboard(k, x, y);

    glutPostRedisplay();
}

// "special" key callback for manipulating sliders
void special(int key, int x, int y)
{
    g_pParamList->Special(key, x, y);
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

    g_object.reshape(w, h);
}

void mouse(int button, int state, int x, int y)
{
    g_object.mouse(button, state, x, y);
    bLeftButtonDown = (button == GLUT_LEFT_BUTTON);

    if (b['`'] && bLeftButtonDown) 
    {
        // call list mouse function
        g_pParamList->Mouse(x, y);
    }
}

void motion(int x, int y)
{
    if (b['`'] && bLeftButtonDown) 
    {
        // call parameter list motion function
        g_pParamList->Motion(x, y);
        glutPostRedisplay();
        return;
    }
    else
        g_object.motion(x, y);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("gpgpu_disease");

    int resolution = 64;
    if (argc > 1)
    {
        resolution = atoi(argv[1]);
    }
    g_pRD = new ReactionDiffusion(resolution, resolution);
    g_rRDf = g_pRD->GetF();
    g_rRDk = g_pRD->GetK();

    init_opengl();

    g_object.configure_buttons(1);
    g_object.dolly.dolly[2] = -2;

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutReshapeFunc(resize);

    glutCreateMenu(menu);
    addUI('`', "Display UI sliders", false);
    addUI('w', "Toggle wireframe", false);
    addUI('d', "Display reaction-diffusion", false);
    addUI('h', "Display diseased head", true);
    addUI(' ', "Toggle animation", true);
    addUI('.', "Single Step (when paused)");
    addUI('t', "Cycle disease mode");
    addUI('r', "Reset reaction-diffusion");
    addUI('p', "Cycle presets");
    addUI('?', "Print help");
    addUI('q', "Quit");
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    key('~', 0, 0); // to initialize the legend text

    g_timer.Start();
    glutMainLoop();

    return 0;
}