
#ifndef GUI_H
#define GUI_H

namespace GUI
{

// Initialization
void Initialize(int, int);
void Cleanup();

// Device management
HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9*, const D3DSURFACE_DESC*, void*);
void CALLBACK OnDestroyDevice(void*);
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9*, const D3DSURFACE_DESC*, void*);
void CALLBACK OnLostDevice(void*);

// Frame events
void CALLBACK OnFrameMove(IDirect3DDevice9*, double, float, void*);
void CALLBACK OnFrameRender(IDirect3DDevice9*, double, float, void*);

// Message events
LRESULT CALLBACK MsgProc(HWND, unsigned int, WPARAM, LPARAM, bool*, void*);
void CALLBACK Mouse(bool, bool, bool, bool, bool, int, int, int, void*);
void CALLBACK KeyboardProc(unsigned int, bool, bool, void*);
}
#endif
