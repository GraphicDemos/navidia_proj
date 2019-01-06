#define DCTSIZE     8
#define DCTSIZE2    64
#define FAST_FLOAT float

void jpeg_fdct_float (FAST_FLOAT * data);
void jpeg_idct_float (FAST_FLOAT *inptr, FAST_FLOAT *outptr, FAST_FLOAT *quantptr);
