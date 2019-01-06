#ifndef NVIDIA_PNG_CODE
#define NVIDIA_PNG_CODE

#include "nv_png_dec.h"
#include <glh/glh_linear.h>
#include <glh/glh_array.h>
#include "data_path.h"



NV_PNG_API data_path get_png_path();
NV_PNG_API void set_png_path(const data_path & newpath);
#ifdef WIN32
NV_PNG_API void set_png_module_handle(unsigned long hM);
NV_PNG_API void set_png_module_restypename(const char * tname);
#endif

NV_PNG_API void read_png_rgba(const char * filename, glh::array2<glh::vec4ub> & image);
NV_PNG_API void read_png_rgb(const char * filename, glh::array2<glh::vec3ub> & image);
NV_PNG_API void read_png_grey(const char * filename, glh::array2<unsigned char> & image);

#endif
