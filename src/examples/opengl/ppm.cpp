//.----------------------------------------------------------------------------.
//| ppm.cpp : Portable Pixel Map image format loading and writing functions.   |
//.----------------------------------------------------------------------------.

#include <stdio.h>
#include <string.h>

//.----------------------------------------------------------------------------.
//|   Function   : LoadPPM                                                     |
//|   Description: reads an image in from a ppm file. returns the color rgb    |
//|                array and dimensions performs auto-allocation of color array|
//|                if set to null before calling; assumes that color has been  |
//|                pre-alloced otherwise                                       |
//.----------------------------------------------------------------------------.
void LoadPPM(const char *pFileName, unsigned char* &pColor, unsigned int &iWidth, unsigned int &iHeight)
{
  FILE* fp = fopen(pFileName, "rb");
  if (fp == NULL) 
  { 
      printf("Error (LoadPPM): unable to open %s!\n", pFileName);
      pColor  = NULL; 
      iWidth  = 0; 
      iHeight = 0; 
      return; 
  }
  char buff[128];

  fgets(buff, 128, fp);
  while(buff[0] == '#') fgets(buff, 128, fp);
  if(strncmp(buff, "P6", 2))
    { fclose(fp); return; }

  fgets(buff, 128, fp);
  while(buff[0] == '#') fgets(buff, 128, fp);
        
  sscanf(buff, "%d %d", &iWidth, &iHeight);
        
  fgets(buff, 128, fp);
  while(buff[0] == '#') fgets(buff, 128, fp);
  printf("Reading %dx%d Image [%s]. . .\n", iWidth, iHeight, pFileName);
  int iNumComponents = iWidth * iHeight * 3;
  if (pColor == NULL) pColor = new unsigned char[iNumComponents];
  fread(pColor, iNumComponents, 1, fp);
  fclose(fp);
}


//.----------------------------------------------------------------------------.
//|   Function   : WritePPM                                                    |
//|   Description: Writes an unsigned byte RGB color array out to a PPM file.  |
//.----------------------------------------------------------------------------.
void WritePPM(const char *pFileName, unsigned char* pColor, unsigned int iWidth, unsigned int iHeight)
{
  FILE* fp = fopen(pFileName, "wb");
  if (fp == NULL) 
  { 
      printf("Error (WritePPM) : unable to open %s!\n", pFileName); 
      return; 
  }
  fprintf(fp, "P6\n%d %d\n255\n", iWidth, iHeight);
  fwrite(pColor, 1, iWidth * iHeight * 3, fp);
  fclose(fp);
}
