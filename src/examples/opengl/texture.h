#define FATAL_ERROR(s)                                                        \
{                                                                             \
  fprintf (stderr, s);                                                        \
  fflush (stderr);                                                            \
  exit (-1);                                                                  \
}

typedef struct Texture {
  GLubyte *image;
  int w, h;     /* width, height */
  int npix;   	/* no. of pixels */
  int ncomps;   /* no. of components */
  int format;
} Texture;

Texture *LoadTexturePPM(char *filename);
