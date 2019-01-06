#ifndef _TESTFILTERDATA_
#define _TESTFILTERDATA_

#include <math.h>
#include <commctrl.h>
#include "PreviewManager.h"
#include "GPUFilterData.h"
#include "resource.h"
#include "ShaderSrc.h"


// This test filter does a Gaussian blur

// The Gaussian distribution function (un-normalized)
double Gauss(double x)
{
  return exp(-x*x*0.5);
}

// The pass class that the Gaussian blur filter uses
struct KernelPass : Pass
{
  float kernel[256];
  float duoweights[128];  // These weights apply to adjacent pairs of pixels
  float offsets[128*4];   // These are the relative weights between adjacent pairs of pixels
  int size;               // The size of the kernel. Smaller kernels = faster shader

  virtual void SetParameters(GPUFilterData *filter)
  {
    int param;

    // Set all of our parameters in the shader
    GLASSERT()
    param = program->GetUniformID("kernel");
    if(param != -1)
      glUniform1fvARB(param, size, kernel);

    GLASSERT()
    param = program->GetUniformID("duoweights");
    if(param != -1)
      glUniform1fvARB(param, size/2, duoweights);

    GLASSERT()
    param = program->GetUniformID("offsets");
    if(param != -1)
      glUniform4fvARB(param, size/2, offsets);

    GLASSERT()
    param = program->GetUniformID("kernellen");
    if(param != -1)
      glUniform1fARB(param, (float)size - 1);

    GLASSERT()
  }

  // This builds a kernel based on the given kernel size
  void SetKernel(int s)
  {
    size = s;

    // The peak of the kernel is at the end of the array
    double accum = kernel[size-1] = (float)Gauss(0.0);

    // Fill the kernel array
    for(int i = size - 2;i >= 0;i--)
    {
      float dist = sqrt(fWidth*fWidth + fHeight*fHeight);

      kernel[i] = (float)Gauss((float)(i-(size-1)) / dist * 3.0f);
      accum += 2*kernel[i];
    }

    // Do a forced normalization of the kernel, so that the mean brightness doesn't change
    for(i = 0;i < size;i++)
      kernel[i] = (float)((double)kernel[i] / accum);

    // Do the special thing!
    DoSpecialKernelThing();
  }

  // This separates the kernel into the duoweights and the offsets
  // It also does any pre-calculating it can, so that the shader
  // doesn't have to.
  void DoSpecialKernelThing()
  {
    // The shader ends up reading the center pixel twice,
    // so cut its weight in half to balance
    kernel[size - 1] /= 2;

    // This rearranges the kernel into duoweights and offsets
    // That way the shader can effectively do 2 texture lookups at once
    // using the linear filtering available on the card.
    // It also lifts some work out of the shader and into the CPU
    for(int i = 0;i < size/2;i++)
    {
      float tmp;
      duoweights[i] = kernel[2*i] + kernel[2*i+1];

      if(duoweights[i] < 0.00001)
        tmp = 0;
      else
        tmp = 2*i + kernel[2*i+1]/duoweights[i];

      offsets[4*i+0] = (fWidth ? -tmp : 0);
      offsets[4*i+1] = (fHeight ? -tmp : 0);
      offsets[4*i+2] = (fWidth ? tmp : 0);
      offsets[4*i+3] = (fHeight ? tmp : 0);
    }
  }
};

// Our filter data class
class TestFilterData : public GPUFilterData
{
private:
  // All of these shaders are basically the same, except for the kernel size
  vector<GLShader *> mShaders;      // Shaders that build the programs
  vector<GLProgram *> mPrograms;    // Programs that the passes use

  KernelPass mHPass;                // The horizontal blur pass
  KernelPass mVPass;                // The vertical blur pass
  GLShader mStandardVS;             // The standard vertex shader (for blitting)
  GLShader mSeparableSymmetricVS;   // The separable-symmetric vertex shader

public:
  TestFilterData() :
    mStandardVS(GLShader::Vertex, "Standard VS"),
    mSeparableSymmetricVS(GLShader::Vertex, "Separable-Symmetric VS")
  {
    GLShader *s1x1;
    GLProgram *p1x1;
    char buf[512];

    // Add 2 passes... SetBlurRadius will know what to do
    mPasses.push_back(&mHPass);
    mPasses.push_back(&mVPass);

    // Load the standard vertex shader (screen quad)
    mStandardVS.AddCodeFromFile("Standard_vs.glsl",standard_vs_src);
    if(!mStandardVS.Validate())
      throw mStandardVS.GetInfo();    // GetInfo() will return any errors that occurred

    // Load the separable-symmetric vertex shader
    mSeparableSymmetricVS.AddCodeFromFile("Separable-Symmetric_vs.glsl",Separable_Symmetric_vs_src);
    if(!mSeparableSymmetricVS.Validate())
      throw mSeparableSymmetricVS.GetInfo();

    // Load the standard fragment shader (for our purposes, a 1x1 kernel filter)
    s1x1 = new GLShader(GLShader::Fragment, "Separable 1x1 (Standard) FS");
    p1x1 = new GLProgram("Separable 1x1 (Standard)");
    s1x1->AddCodeFromFile("Standard_fs.glsl",standard_fs_src);
    if(!s1x1->Validate())
      throw s1x1->GetInfo();
    mShaders.push_back(s1x1);

    // Set up the standard program
    p1x1->AddShader(*s1x1);
    p1x1->AddShader(mStandardVS);
    if(!p1x1->Validate())
      throw p1x1->GetInfo();
    mPrograms.push_back(p1x1);

    // Now loop through all the kernel sizes that are powers of 2
    // (up to 128) and build a shader for each
    for(int i = 0;i < 8;i++)
    {
      GLShader *shader;
      GLProgram *program;

      // Name = "Separable-Symmetric [Size]x[Size]"
      itoa((1 << (i + 2)) - 1, buf + 256, 10);
      strcpy(buf, "Separable-Symmetric ");
      strcat(buf, buf + 256);
      strcat(buf, "x");
      strcat(buf, buf + 256);

      program = new GLProgram(buf);

      // Name = "Separable-Symmetric [Size]x[Size] FS"
      strcat(buf, " FS");

      shader = new GLShader(GLShader::Fragment, buf);

      // We tell the shader what kernel size it is by #defining KERNELSIZE
      strcpy(buf, "#define KERNELSIZE ");
      itoa(1 << i, buf + 19, 10);
      strcat(buf, "\n");
      shader->AddCode(buf);
      shader->AddCodeFromFile("Separable-Symmetric_fs.glsl",Separable_Symmetric_fs_src);
      if(!shader->Validate())
        throw shader->GetInfo();
      mShaders.push_back(shader);

      // Put the program together and link it
      program->AddShader(*shader);
      program->AddShader(mSeparableSymmetricVS);
      if(!program->Validate())
        throw program->GetInfo();
      mPrograms.push_back(program);
    }

    // Set the parameters
    SetBlurRadius(0);

    // Set the GUI information
    mGUI = IDD_GUI;
    mPreview = IDC_RENDER_HERE;
  }

  ~TestFilterData()
  {
    for(vector<GLProgram *>::iterator i = mPrograms.begin();i != mPrograms.end();i++)
      delete *i;
    for(vector<GLShader *>::iterator i = mShaders.begin();i != mShaders.end();i++)
      delete *i;
  }

  // This rebuilds the kernel and chooses the appropriate shader for
  // each pass to use, given the size of the blur
  void SetBlurRadius(float r)
  {
    mHPass.fWidth = r;
    mHPass.fHeight = 0;
    mVPass.fWidth = 0;
    mVPass.fHeight = r;

    int approx = (int)ceil(r) + 1;

    for(int i = 1,j = 0;i < approx;i<<=1,j++);

    mHPass.SetKernel(i);
    mVPass.SetKernel(i);

    mHPass.program = mPrograms[j];
    mVPass.program = mPrograms[j];
  }

  // This manages our GUI window
  virtual INT_PTR HandlePreviewMessages(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
  {
    ASSERT(PreviewManager::Get())

    static VPoint mousepos;   // The last known mouse position, for panning
    static VPoint margin;     // The right and bottom margins, for resizing calculations
    char buffer[256];
    HWND slider = GetDlgItem(hwnd, IDC_RADIUS_SLIDER);
    HWND edit = GetDlgItem(hwnd, IDC_RADIUS_EDIT);
    PreviewManager &preview = *PreviewManager::Get();

    try
    {
      // Handle messages
      switch(msg)
      {
        case WM_INITDIALOG:
          {
            RECT client;
            RECT previewwnd;

            // Basic initialization
            SendMessage(slider, TBM_SETRANGEMIN, (WPARAM)false, (LPARAM)0);
            SendMessage(slider, TBM_SETRANGEMAX, (WPARAM)true, (LPARAM)254990);
            SendMessage(slider, TBM_SETPOS, (WPARAM)true, (LPARAM)mPasses.front()->fWidth * 1000);
            sprintf(buffer, "%.2f", mPasses.front()->fWidth / 3.0f);
            SetWindowText(edit, buffer);
            GetClientRect(hwnd, &client);
            GetWindowRect(GetDlgItem(hwnd, IDC_RENDER_HERE), &previewwnd);
            margin.h = client.right - (previewwnd.right - previewwnd.left);
            margin.v = client.bottom - (previewwnd.bottom - previewwnd.top);
          }
          break;
        case WM_COMMAND:
          {
            int code = HIWORD(wparam);
            int senderID = LOWORD(wparam);
            int senderHWND = lparam;

            switch(senderID)
            {
            // If the user pushed the OK or Cancel buttons
            case IDOK:
            case IDCANCEL:
              EndDialog(hwnd, senderID);
              break;
            // If the user is messing with the radius text box
            case IDC_RADIUS_EDIT:
              switch(code)
              {
              // If the user just stopped messing with the radius text box, do a full update
              case EN_KILLFOCUS:
                GetWindowText(edit, buffer, 256);
                SetBlurRadius(max(0, min(85.0f, (float)atof(buffer))) * 3.0f);
                SendMessage(slider, TBM_SETPOS, (WPARAM)true, (LPARAM)mPasses.front()->fWidth * 1000);
                preview.ParamsChanged();
                KillTimer(hwnd, 2);
                break;
              // Or, if they just typed something, wait for half a second
              // and then do a partial update
              case EN_CHANGE:
                KillTimer(hwnd, 2);
                if(GetFocus() == GetDlgItem(hwnd, IDC_RADIUS_EDIT))
                  SetTimer(hwnd, 2, 500, 0);
                break;
              }
              break;
            }
          }
          break;
        // If the user is fiddling with the slider
        case WM_HSCROLL:
          if(GetDlgItem(hwnd, IDC_RADIUS_SLIDER) == (HWND)lparam)
          {
            int pos = (int)SendMessage(slider, TBM_GETPOS, 0, 0);

            SetBlurRadius(pos/1000.0f);

            // If they just let go of it, do a full update
            if(LOWORD(wparam) == TB_ENDTRACK)
              preview.ParamsChanged();
            // Otherwise, do a partial update
            else
              preview.ParamsChanging();

            // Update the text box and the screen
            sprintf(buffer, "%.2f", mPasses.front()->fWidth / 3.0f);
            SetWindowText(edit, buffer);
            preview.Draw();
          }
          break;
        // If the user pushed down the left mouse button, initialize dragging
        case WM_LBUTTONDOWN:
          mousepos.h = LOWORD(lparam);
          mousepos.v = HIWORD(lparam);
          SetCapture(hwnd);
          break;
        case WM_LBUTTONUP:
          ReleaseCapture();
          break;
        case WM_MOUSEMOVE:
          if(wparam & MK_LBUTTON)
          {
            // The user is dragging the mouse! Pan the preview window

            // These subtractions seem backwards, because
            // we're not moving the window, we're grabbing and moving
            // the image under it. It's all relative. 
            VPoint delta;
            delta.h = mousepos.h - GET_X_LPARAM(lparam);
            delta.v = mousepos.v - GET_Y_LPARAM(lparam);

            // Do the pan
            PreviewManager::Get()->Pan(delta);

            // Update the mouse position
            mousepos.h = GET_X_LPARAM(lparam);
            mousepos.v = GET_Y_LPARAM(lparam);
          }
          break;
        // If the user resized the window, fix up the preview window too
        case WM_SIZE:
          {
            VPoint size = {HIWORD(lparam), LOWORD(lparam)};

            size.h -= margin.h;
            size.v -= margin.v;

            if(size.h < 15)
              size.h = 15;
            if(size.v < 15)
              size.v = 15;

            switch(wparam)
            {
            case SIZE_MAXIMIZED:
            case SIZE_RESTORED:
              SetWindowPos(
                GetDlgItem(hwnd, IDC_RENDER_HERE),
                0,0,0,
                size.h,
                size.v,
                SWP_NOMOVE | SWP_NOZORDER);

              preview.Resize(size);
              break;
            case SIZE_MINIMIZED:      // If it was just minimzed, don't do anything
              break;
            default:
              break;
            }
          }
          break;
        case WM_TIMER:  // The timer that waits half a second after the user typed into the text box
          if(wparam == 2)
          {
            KillTimer(hwnd, 2);
            GetWindowText(edit, buffer, 256);
            SetBlurRadius(max(0, min(85.0f, (float)atof(buffer))) * 3.0f);
            SendMessage(slider, TBM_SETPOS, (WPARAM)true, (LPARAM)mPasses.front()->fWidth * 1000);
            preview.ParamsChanging();
          }
          break;
        // These message are the exception to the rule
        // regarding handling of dialog messages
        case WM_CHARTOITEM:
        case WM_COMPAREITEM:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_QUERYDRAGICON:
        case WM_VKEYTOITEM:
          return DefWindowProc(hwnd, msg, wparam, lparam);
      }

      return false;
    }
    catch(const char *err)
    {
      MessageBox(0, err, "Error", MB_OK);
      EndDialog(hwnd, IDCANCEL);
      return false;
    }
  }
};

#endif