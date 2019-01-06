#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "jdct.h"
#include "dct_cpu.h"

float quantize[DCTSIZE2];
float dequantize[DCTSIZE2];

// from jcdctmgr.c
/* For float AA&N IDCT method, divisors are equal to quantization
	* coefficients scaled by scalefactor[row]*scalefactor[col], where
	*   scalefactor[0] = 1
	*   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	* We apply a further scale factor of 8.
	* What's actually stored is 1/divisor so that the inner loop can
	* use a multiplication rather than a division.
	*/	
float aanscalefactor[] = {
	1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
	1.0f, 0.785694958f, 0.541196100f, 0.275899379f
};

// build (de)quantization tables
void init_quantize_tables()
{
    for(int y=0; y<DCTSIZE; y++) {
        for(int x=0; x<DCTSIZE; x++) {
            quantize[y*DCTSIZE+x] = 1.0f / (aanscalefactor[x] * aanscalefactor[y] * 8.0f);

            dequantize[y*DCTSIZE+x] = aanscalefactor[x] * aanscalefactor[y];
        }
    }
}

void dump_block(float *i)
{
    for(int y=0; y<DCTSIZE; y++) {
        for(int x=0; x<DCTSIZE; x++) {
            printf("%.2f, ", i[y*8+x]);
        }
        printf("\n");
    }

}

void mul_block(float *img, float *table)
{
   for(int i=0; i<DCTSIZE2; i++) {
        img[i] *= table[i];
    }
}

float frand()
{
    return rand() / (float) RAND_MAX;
}

void randomize_img(float *img, int n)
{
    for(int i=0; i<n; i++) {
        img[i] = frand();
    }
}

void test_cpu_dct()
{
    float input[DCTSIZE2], output[DCTSIZE2];

    init_quantize_tables();
    randomize_img(input, DCTSIZE2);

    printf("Input:\n");
    dump_block(input);

    // forward dct
    jpeg_fdct_float(input);
    // quantize
    mul_block(input, quantize);

    printf("\nAfter DCT:\n");
    dump_block(input);

    // dequantize and inverse dct
    jpeg_idct_float(input, output, dequantize);

    printf("\nAfter IDCT:\n");
    dump_block(output);
}

void fdct_image(array2<unsigned char> &img, array2<float> &destimg)
{
    float input[DCTSIZE2];

    for(int y=0; y<img.get_height(); y+=8) {
        for(int x=0; x<img.get_width(); x+=8) {

            for(int j=0; j<8; j++) {
                for(int i=0; i<8; i++) {
                    input[j*8+i] = (float) img(x+i, y+j) / 255.0;
                }
            }

            jpeg_fdct_float(input);
 
            for(int j=0; j<8; j++) {
                for(int i=0; i<8; i++) {
                    destimg(x+i, y+j) = input[j*8+i] / 8.0;
//                    if (i>2 || j>2) destimg(x+i, y+j) = 0.0;
                }
            }
        }
    }
}