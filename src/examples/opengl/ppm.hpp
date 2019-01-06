//============================================================================
// ppm.hpp : Portable Pixel Map image format loading and writing functions.
//============================================================================

void LoadPPM( const char *pFileName, unsigned char* &pColor, unsigned int &iWidth, unsigned int &iHeight);
void WritePPM(const char *pFileName, unsigned char*  pColor, unsigned int  iWidth, unsigned int  iHeight);
