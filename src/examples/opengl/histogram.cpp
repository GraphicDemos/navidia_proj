#include "histogram.h"
#include "shared/data_path.h"

CGcontext g_context;

Histogram::Histogram(int width) :
    m_width(width),
    m_current_pass(0)
{
    m_count = new unsigned int [width];
    for(int i=0; i<m_width; i++) m_count[i] = 0;

    glGenQueriesARB(1, &m_query);

    InitCg();
}

Histogram::~Histogram()
{
    delete [] m_count;
    cgDestroyProgram(m_fprog);
    cgDestroyContext(g_context);
    glDeleteQueriesARB(1, &m_query);
}


static void 
cgErrorCallback()
{
    CGerror lastError = cgGetError();
    if(lastError) {
        printf("Cg error:\n");
        printf("%s\n\n", cgGetErrorString(lastError));
        printf("%s\n", cgGetLastListing(g_context));
        cgDestroyContext(g_context);
        exit(-1);
    }
}

void
Histogram::InitCg()
{
    cgSetErrorCallback(cgErrorCallback);
    g_context = cgCreateContext();

    // initialize fragment program
    data_path path;
    path.path.push_back("../../../MEDIA/programs");
    path.path.push_back("../../../../MEDIA/programs");
    std::string pathname = path.get_file("imgproc_histogram/histogram.cg");

    m_fprog_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    m_fprog = cgCreateProgramFromFile(g_context, CG_SOURCE, pathname.data(), m_fprog_profile, NULL, NULL);
    cgGLLoadProgram(m_fprog);

    m_min_param = cgGetNamedParameter(m_fprog, "min");
    m_max_param = cgGetNamedParameter(m_fprog, "max");
    m_channels_param = cgGetNamedParameter(m_fprog, "channels");

    // set channels to calculate luminance by default
    cgGLSetParameter3f(m_channels_param, 0.299f, 0.587f, 0.114f);
}

void
Histogram::SetChannels(float r, float g, float b)
{
    cgGLSetParameter3f(m_channels_param, r, g, b);
}

void
Histogram::BeginCount(int pass)
{
    m_current_pass = pass;
 
    // set fragment program parameters
    float min = pass / (float) m_width;
    float max = (pass+1) / (float) m_width;
    cgGLSetParameter1f(m_min_param, min);
    cgGLSetParameter1f(m_max_param, max);
//    printf("%d: %f, %f\n", pass, min, max);

    // begin occlusion query
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, m_query);       

    cgGLBindProgram(m_fprog);
    cgGLEnableProfile(m_fprog_profile);
}

void
Histogram::EndCount()
{
    // end occlusion query and get result
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    glGetQueryObjectuivARB(m_query, GL_QUERY_RESULT_ARB, &m_count[m_current_pass]);

    cgGLDisableProfile(m_fprog_profile);
}

void
Histogram::Display(float x, float y, float bar_width, float height)
{
    // draw border
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + m_width*bar_width + 2, y);
    glVertex2f(x + m_width*bar_width + 2, y + height + 2);
    glVertex2f(x, y + height + 2);
    glEnd();

    // find maximum value and total
    unsigned int max = 1;
    unsigned int total = 0;
    for(int i=0; i<m_width; i++) {
        total += m_count[i];
        if (m_count[i] > max) max = m_count[i];
    }
//    printf("total = %d\n", total);

    // draw columns
    x += 1;
    y += 1;
    glColor3f(1.0, 1.0, 0.0);
    for(int i=0; i<m_width; i++) {
        float top = (m_count[i] / (float) max) * height;
        glBegin(GL_QUADS);
        glVertex2f(x + (i * bar_width), y);
        glVertex2f(x + (i * bar_width) + bar_width, y);
        glVertex2f(x + (i * bar_width) + bar_width, y + top);
        glVertex2f(x + (i * bar_width), y + top);
        glEnd();
    }
}
