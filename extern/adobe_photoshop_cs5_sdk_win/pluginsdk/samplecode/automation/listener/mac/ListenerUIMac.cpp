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
//		ListenerUIMac.cpp
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
//
//	DoUI
//
//	Display the Macintosh user interface.
//
//-------------------------------------------------------------------------------
SPErr DoUI (void)
{
	SPErr error = kSPNoError;
	
	short item = kDNoUI;

	ProcessEventProc processEvent = sPSUIHooks->processEvent;

	ResContext context(gPlugInRef);

	DialogTHndl dialogTemplate = (DialogTHndl) GetResource ('DLOG', kListenerDialogID);
	HNoPurge ((Handle) dialogTemplate);
	
	CenterDialog (dialogTemplate);
	
	DialogPtr dialog = GetNewDialog( kListenerDialogID, nil, (WindowPtr)-1L );
	
	PIWatchComboBox watchForCombo;
	PISetComboBox playSetCombo;
	PIActionComboBox playActionCombo;
	PIInstalledStaticText staticText;

	DoUIInit(dialog, watchForCombo, playSetCombo, playActionCombo, staticText);
	
	(void) SetDialogDefaultItem(dialog, kDOk_button);
	(void) SetDialogCancelItem(dialog, kDCancel_button);
	(void) SetDialogTracksCursor(dialog, TRUE);

	Boolean doneWithDialog = false;
	
	while (doneWithDialog == false)
	{
		MoveableModalDialog(dialog,
						    processEvent,
						    nil,
						    &item);

		switch (item)
		{
			case kDOk_button:
			case kDCancel_button:
				doneWithDialog = true;
				break;
				
			case kDWatchFor_editTextPopUp:
				watchForCombo.Update();
				break;
				
			case kDPlaySet_editTextPopUp:
				playSetCombo.Update();
				playActionCombo.Init();
				InvalItem(dialog, kDPlayAction_editTextPopUp);
				break;
				
			case kDPlayAction_editTextPopUp:
				playActionCombo.Update();
				break;

			default:
				break;
		}
	}
	
	DisposeDialog(dialog);
	HPurge ((Handle) dialogTemplate);
	
	if (item != kDOk_button)
	{
		error = 'STOP';
	}

	return error;
}
// end ListenerUIMac.cpp 