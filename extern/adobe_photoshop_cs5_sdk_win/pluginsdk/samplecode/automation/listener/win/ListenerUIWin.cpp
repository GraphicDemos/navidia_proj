// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		ListenerUIWin.cpp
//
//	Description:
//		This file contains the source code and routines for the
//		Automation module Listener, an example of a module
//		that uses the Actions suite in a persistent
//		Automation plug-in.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "Listener.h"
#include "ListenerUI.h"

//-------------------------------------------------------------------------------
//	File globals.
//-------------------------------------------------------------------------------
DLLExport BOOL WINAPI ListenerProc(HWND hDlg, 
								   UINT wMsg, 
								   WPARAM wParam, 
								   LPARAM lParam);

//-------------------------------------------------------------------------------
//
//	DoUI
//
//	Display the Windows user interface.
//
//-------------------------------------------------------------------------------
SPErr DoUI (void)
{
	SPErr error = noErr;
	
	INT_PTR item = kDNoUI;	// Error value.
	
	item = DialogBoxParam(GetDLLInstance(gPlugInRef),
		                  MAKEINTRESOURCE(kListenerDialogID),
						  GetActiveWindow(),
						  (DLGPROC)ListenerProc,
						  NULL);

	if (item != kDOk_button)
	{
		error = 'STOP';
	}
	return error;
}

//-------------------------------------------------------------------------------
//
//	ListenerProc
//
//	Handle dialog interaction
//
//-------------------------------------------------------------------------------
DLLExport BOOL WINAPI ListenerProc(HWND hDlg, 
								   UINT wMsg, 
								   WPARAM wParam, 
								   LPARAM /*lParam*/)
{
	static PIWatchComboBox watchForCombo;
	static PISetComboBox playSetCombo;
	static PIActionComboBox playActionCombo;
	static PIInstalledStaticText staticText;

	switch (wMsg)
	{
		case WM_INITDIALOG:
			CenterDialog(hDlg);
			DoUIInit(hDlg, watchForCombo, playSetCombo, playActionCombo, staticText);
			return TRUE;

		case WM_COMMAND:
		{
			int item = LOWORD (wParam);
			int cmd = HIWORD (wParam);
			switch (item)
			{
				case kDOk_button:
				case kDCancel_button:
					if (cmd == BN_CLICKED)
					{
						EndDialog(hDlg, item);
						return TRUE;
					}
					break;
				case kDWatchFor_editTextPopUp:
					if (cmd == CBN_SELENDOK)
					{
						watchForCombo.Update();
					}
					break;
				case kDPlaySet_editTextPopUp:
					if (cmd == CBN_SELENDOK)
					{
						playSetCombo.Update();
						playActionCombo.Init();
					}
					break;
				case kDPlayAction_editTextPopUp:
					if (cmd == CBN_SELENDOK)
					{
						playActionCombo.Update();
					}
					break;
				default:
					break;
			}
		
			return TRUE;
			break;
		}

		default:
			return FALSE;
			break;
	}
	return TRUE;
}
//-------------------------------------------------------------------------------
// end ListenerUIWin.cpp
