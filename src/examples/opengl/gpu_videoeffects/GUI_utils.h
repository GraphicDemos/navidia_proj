#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#ifdef _WIN32
#include <windows.h>
#endif

bool loadImageSource(HWND parent, char *start_path, char *outputFile);
bool loadVideoSource(HWND parent, char *start_path, char *outputFile);

#endif
