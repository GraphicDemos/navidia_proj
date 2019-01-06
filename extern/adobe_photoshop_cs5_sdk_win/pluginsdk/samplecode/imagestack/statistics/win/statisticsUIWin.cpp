#include <stddef.h>
#include <stdlib.h>


#include "PITypes.h"
//#include "PIExtension.h"
#include "PIAbout.h"
#include "PIGeneral.h"

#include "statistics.h"

/* Displays the about dialog box for the plug-in module. */
extern BOOL FAR PASCAL AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

BOOL		gWin95=0;	// is it Windows95?
HBRUSH	gBackBr = NULL;

HANDLE hDllInstance;


/*****************************************************************************/

#define STATE_IDLE		0
#define STATE_AGAIN		1
#define STATE_CANCEL	-1

//#define SDK_VALIDATE	WM_USER+100

short	state = STATE_IDLE;
short	lastTitem = dRowsItem;
Boolean Validate (GPtr globals, const HWND hDlg, int *item);

/********************************************************************/
void SetEnvironVars(void)
{
	DWORD dwVer;
	
	dwVer = GetVersion();	

	if (dwVer >=  0x80000000 && (LOBYTE(LOWORD(dwVer)) >= 4))
		gWin95 = TRUE;
	else
		gWin95 = FALSE;
}

/********************************************************************/
void SetupBackBrush(BOOL bExiting)	// setup gBackBr for CTLCOLOR msgs
{
	if (gBackBr)
	{
		DeleteObject(gBackBr);
		gBackBr = NULL;
	}

	if (!bExiting)
		gBackBr = CreateSolidBrush(GetSysColor((gWin95 ? COLOR_BTNFACE : COLOR_WINDOW)));
}

/********************************************************************/
VOID EraseWindowToColor(HWND wnd, HDC hdc, DWORD color)
{
   RECT     rct;

   SetBkColor(hdc, color);
   GetClientRect(wnd, &rct);
   ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rct, NULL, 0, NULL);
}

/********************************************************************/
void DoAbout (GPtr globals)
{
	AboutRecordPtr aboutPtr = (AboutRecordPtr)gStuff;
	PlatformData *platform;
	platform = (PlatformData *)(aboutPtr->platformData);

#if qBuildEveWin
	{
	Handle dialogTemplate = nil;
	dialogTemplate = PIDMWinExpressDialog( hDllInstance, 16000 );
	if (nil != dialogTemplate) 
		{
		DialogBoxIndirect( hDllInstance, dialogTemplate, 
				(HWND)(platform->hwnd), AboutDlgProc);
		}
	}
#else
	DialogBox((HINSTANCE)hDllInstance, MAKEINTRESOURCE(16000),
		(HWND)platform->hwnd, (DLGPROC)AboutDlgProc);
#endif // qBuildEveWin
}

/*****************************************************************************/

// Every DLL has an entry point DLLInit
BOOL APIENTRY DLLInit /*LibMain*/(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
	// Required when using Zortech; causes blink to include startup code
	// extern __acrtused_dll;

	if (fdwReason == DLL_PROCESS_ATTACH)
		hDllInstance = hInstance;

	SetEnvironVars ();

	return TRUE;   // Indicate that the DLL was initialized successfully.
}

/*****************************************************************************/
