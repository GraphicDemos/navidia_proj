#include "GUI_utils.h"

#include <atlstr.h>
#include <commdlg.h>

bool loadImageSource(HWND parent, char *start_path, char *outputFile)
{
  OPENFILENAME ofn;
  char currentDirectory[256], filename[256];
  TCHAR startDirectory[256];
  int index = 0;

  bool restoreDirectory = false;
  bool return_result = false;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ZeroMemory(currentDirectory, 256);
  ZeroMemory(filename, 256);

  restoreDirectory = (GetCurrentDirectoryA(256, currentDirectory) != 0);

  if (start_path) {
//      SetCurrentDirectoryA(start_path);
      while ((startDirectory[index++] = *start_path++) != 0);
      startDirectory[index] = '\0';
  }

  ofn.nMaxFile    = _MAX_PATH;;
  ofn.lpstrFile   = (LPTSTR)filename;
  ofn.hwndOwner   = parent;
  ofn.lStructSize = sizeof(OPENFILENAME);

  ofn.lpstrTitle      = _T("Load Source Image");
//  ofn.lpstrFilter     = _T("Image Files(*.exr *.bmp *.dds *.jpg)\0*.exr;*.bmp;*.dds;*.jpg\0All(*.*)\0*.*\0\0");
  ofn.lpstrFilter     = _T("OpenEXR Images (*.exr)\0*.exr\0All(*.*)\0*.*\0\0");
  ofn.nFilterIndex    = 1;
  ofn.nMaxFileTitle   = 0;
  ofn.lpstrInitialDir = (start_path != NULL) ? startDirectory : NULL;
  ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if(GetOpenFileName(&ofn))
  {
		while ((*outputFile++ = (char)*ofn.lpstrFile++) != 0);
		*outputFile = '\0';
	  return_result = true;
  } else {
	  return_result = false;
  }

  if(restoreDirectory)
    SetCurrentDirectoryA(currentDirectory);

  return return_result;
}

bool loadVideoSource(HWND parent, char *start_path, char *outputFile)
{
  OPENFILENAME ofn;
  char currentDirectory[256], filename[256];
  TCHAR startDirectory[256];
  int index = 0;

  bool restoreDirectory = false;
  bool return_result = false;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ZeroMemory(currentDirectory, 256);
  ZeroMemory(filename, 256);

  restoreDirectory = (GetCurrentDirectoryA(256, currentDirectory) != 0);

  if (start_path) {
//      SetCurrentDirectoryA(start_path);
      while ((startDirectory[index++] = *start_path++) != 0);
      startDirectory[index] = '\0';
  }

  ofn.nMaxFile    = _MAX_PATH;;
  ofn.lpstrFile   = (LPTSTR)filename;
  ofn.hwndOwner   = parent;
  ofn.lStructSize = sizeof(OPENFILENAME);

  ofn.lpstrTitle      = _T("Load Source Video");
  ofn.lpstrFilter     = _T("Video Files(*.avi *.mpg *.wmv *.asf)\0*.avi;*.mpg;*.wmv;*.asf\0All(*.*)\0*.*\0\0");
  ofn.nFilterIndex    = 1;
  ofn.nMaxFileTitle   = 0;
  ofn.lpstrInitialDir = (start_path != NULL) ? startDirectory : NULL;
  ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if(GetOpenFileName(&ofn))
  {
		while ((*outputFile++ = (char)*ofn.lpstrFile++) != 0);
		*outputFile = '\0';
	  return_result = true;
  } else {
	  return_result = false;
  }

  if(restoreDirectory)
    SetCurrentDirectoryA(currentDirectory);

  return return_result;
}

