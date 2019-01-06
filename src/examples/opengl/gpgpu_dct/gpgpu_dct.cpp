/*
    Discrete Cosine Transform

    This example implements the forward and inverse discrete cosine transform,
	as used in the JPEG image compression algorithm, using fragment programs.
	
	The DCT operates on 8x8 pixel blocks. The 2D DCT is separated into two 1D
	DCTs, the first of which operates on the columns, the second one on the
	rows. Since the 1D DCT takes 8 pixels as input and produces 8 outputs, some
    rearrangement of the data is required to get the fragment program to operate
    efficiently. Each pass writes out into the RGBA components of two render
    targets (8 components total), where the targets are 1/8th of the original
    image width or height. This data is unpacked into normal planar format
    on subsequent passes.
	
	This code can perform the forward DCT followed by the inverse DCT at around
    160 frames per second for a 512 x 512 monochrome image on a GeForce 6800.
	
	This could be extended into a complete hardware accelerated JPEG viewer
	by performing the rest of the JPEG algorithm (Huffman decompression etc.) on
	the CPU, and adding resampling and color space conversion to the GPU code.

	References:
	http://en.wikipedia.org/wiki/Discrete_cosine_transform
	http://www.cs.cf.ac.uk/Dave/Multimedia/node231.html	
	http://www.faqs.org/faqs/compression-faq/part2/	

	This code is based on the work of the Independent JPEG Group.
*/

#if defined(WIN32)
#  include <windows.h>
#endif
#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <nv_png.h>

#include "dct_cpu.h"
#include "dct_gpu.h"

using namespace glh;
bool b[256];
DCT_GPU *dct;
int show_buffer = 2;
float quantize_level = 2.0;
bool mrt_available;

void init_opengl()
{
    glClearColor(0.2, 0.2, 0.2, 1.0);

    if(! glh_init_extensions(
        "GL_ARB_multitexture "
        "GL_ARB_vertex_program "
        "GL_ARB_fragment_program "
        "GL_NV_vertex_program "
        "GL_NV_fragment_program "
        "WGL_ARB_pbuffer "
        "WGL_ARB_pixel_format "
        "WGL_ARB_render_texture "
        "GL_NV_float_buffer "
        ))
    {
        fprintf(stderr, "Error - required extensions were not supported: %s\n", glh_get_unsupported_extensions());
    }

    mrt_available = glh_init_extensions("GL_ATI_draw_buffers") != 0;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dct->FDCT(dct->GetSrcBuffer(), dct->GetTempBuffer());
    dct->Quantize(dct->GetTempBuffer(), dct->GetDestBuffer(), quantize_level);
    dct->IDCT(dct->GetDestBuffer(), dct->GetTempBuffer());

    switch(show_buffer) {
    case 0:
        dct->DisplayBuffer(dct->GetSrcBuffer());
        break;
    case 1:
        dct->DisplayBuffer(dct->GetDestBuffer(), 4.0);
        break;
    case 2:
        dct->DisplayBuffer(dct->GetTempBuffer());
        break;
    }

    glutSwapBuffers();
}

void idle()
{
    if (b[' '])
        glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
    b[k] ^= 1;

    switch(k) {
    case 27:
    case 'q':
        exit(0);
        break;
    case '1':
    case '2':
    case '3':
    case '4':
        show_buffer = k - '1';
        break;
    case '=':
        quantize_level+=1.0;
        if (quantize_level > 8.0) quantize_level = 8.0;
        printf("quantize level: %f\n", quantize_level);
        break;
    case '-':
        quantize_level-=1.0;
        if (quantize_level < 0.0) quantize_level = 0.0;
        printf("quantize level: %f\n", quantize_level);
        break;
    }
    
	glutPostRedisplay();
}

void resize(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
}

void main_menu(int i)
{
    key((unsigned char) i, 0, 0);
}

void init_menu()
{    
    glutCreateMenu(main_menu);
    glutAddMenuEntry("Show original image [1]", '1');
    glutAddMenuEntry("Show DCT [2]", '2');
    glutAddMenuEntry("Show IDCT [3]", '3');
    glutAddMenuEntry("Increase quality [=]", '=');
    glutAddMenuEntry("Decrease quality [-]", '-');
    glutAddMenuEntry("Quit [esc]", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

LARGE_INTEGER frequency;

float GetTimeDifference(LARGE_INTEGER *pStart, LARGE_INTEGER *pEnd)
{
    float result;
    result = (float)(pEnd->QuadPart - pStart->QuadPart);
    result /= frequency.QuadPart;
    return result;
}

void benchmark_cpu(array2<unsigned char> &img, int repeat)
{
    QueryPerformanceFrequency(&frequency);

    array2<float> output_img;
    output_img.set_size(img.get_width(), img.get_height());

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    for(int i=0; i<repeat; i++) {
        fdct_image(img, output_img);
    }
    QueryPerformanceCounter(&end);

    float time = GetTimeDifference(&start, &end);
    printf("cpu time: %f\n", time / repeat);
}

void benchmark_gpu(int repeat)
{
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    for(int i=0; i<repeat; i++) {
        dct->FDCT(dct->GetSrcBuffer(), dct->GetDestBuffer());
    }
	glFinish();
    QueryPerformanceCounter(&end);

    float time = GetTimeDifference(&start, &end);
    printf("gpu time: %f\n", time / repeat);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("gpgpu_dct");

	init_opengl();

    array2<unsigned char> img;
    read_png_grey("lena_bw.png", img);
    
    dct = new DCT_GPU(img.get_width(), img.get_height(), mrt_available);

#if 1
    dct->LoadImage(img);
#else
    array2<float> img_dct;
    img_dct.set_size(img.get_width(), img.get_height());
    dct_image(img, img_dct);
    dct->LoadImageFloat(img_dct);
#endif

    //test_cpu_dct();
    //benchmark_cpu(img, 100);
    //benchmark_gpu(100);

	glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);
    init_menu();

	b[' '] = true;

	glutMainLoop();

	return 0;
}
