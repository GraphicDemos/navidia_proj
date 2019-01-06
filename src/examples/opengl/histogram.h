#if defined(WIN32)
#include <windows.h>
#endif
#include <glh/glh_extensions.h>
#include "cg/cgGL.h"

class Histogram {
public:
    Histogram(int width);
    ~Histogram();

    void BeginCount(int pass);
    void EndCount();
    void Display(float x, float y, float bar_width, float height);

    void SetChannels(float r, float g, float b);
    int GetWidth() { return m_width; }

private:
    void InitCg();

    int m_width;
    unsigned int *m_count;

    int m_current_pass;
    GLuint m_query;

    CGprogram m_fprog;
    CGprofile m_fprog_profile;
    CGparameter m_min_param, m_max_param, m_channels_param;
};
