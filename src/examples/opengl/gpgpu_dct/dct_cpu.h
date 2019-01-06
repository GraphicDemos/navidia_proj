#include "glh/glh_array.h"
using namespace glh;

extern float quantize[];
extern float dequantize[];

void init_quantize_tables();
void test_cpu_dct();
void fdct_image(array2<unsigned char> &img, array2<float> &destimg);
