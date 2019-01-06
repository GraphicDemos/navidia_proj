#include <math.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MACOS
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "texture.h"

void CheckErrors(void)
{
  GLenum error;
  
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL error: %s\n", (char *) gluErrorString(error));
  }
}

int isspace(int c)
{
  return (c==' ' || c=='\t' || c=='\n');
}

void SkipWhiteSpace(FILE *file)
{
  int c;

  while(isspace(c = getc(file)));

  /* if comment skip till end of line */
  while(c == '#') {
    while((c = getc(file) != '\n'));
    c = getc(file);
  }
  ungetc(c, file);
}

/* Simple PPM format image loader */

Texture *LoadTexturePPM(char *filename)
{
  FILE *texfile;
  Texture *tex;
  char text[256];
  int maxval;

  if ((texfile = fopen(filename, "rb") ) == 0) {
    FATAL_ERROR("Can't open texture file\n");
  }

  if ((tex = (Texture *) malloc(sizeof(Texture))) == 0) {
    FATAL_ERROR("Error allocating memory for texture structure!\n");
  }

  fscanf(texfile, "%s", text);
  if (strcmp(text, "P6") != 0) {
    printf("Sorry - only support PPM P6 (binary) file format\n");
    return 0;
  }
  SkipWhiteSpace(texfile);

  if (fscanf(texfile, "%d %d %d", &tex->w, &tex->h, &maxval) != 3)
    FATAL_ERROR("Error parsing PPM header\n");

  if (maxval != 255) {
    printf("Warning - PPM maxval is %d\n", maxval);
  }

  /* consume any trailing whitespace */
  SkipWhiteSpace(texfile);

  tex->npix = tex->w * tex->h;
  tex->ncomps = 3;
  tex->format = GL_RGB;

  tex->image = (GLubyte *) malloc ( tex->npix * 3 );
  fread(tex->image, 1, tex->npix * 3, texfile);

  fclose(texfile);

  printf("%s - w: %d  h: %d  ncomps: %d\n", filename, tex->w, tex->h, tex->ncomps);

  return tex;
}
