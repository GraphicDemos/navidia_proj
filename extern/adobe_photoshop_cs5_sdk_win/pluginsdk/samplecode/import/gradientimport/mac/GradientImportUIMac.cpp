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
//		GradientImportUIMac.cpp
//
//	Copyright 1996-1997, Adobe Systems Incorporated.
//	All Rights Reserved.
//
//	Description:
//		This file contains the dialog and platform-specific functions
//		for the Import module Idle, a very simple example of a module
//		that just idles and sends update events.
//
//	Use:
//		This import module is good as an example of a module
//		that forces the background application to redraw
//		its windows.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "GradientImport.h"

//-------------------------------------------------------------------------------
//	Definitions -- Dialog items
//-------------------------------------------------------------------------------

static const uint8 dRowsItem = 3;
static const uint8 dColsItem = 4;
static const uint8 dFirstModeItem = 5;
static const uint8 dLastModeItem = 8;
static const uint8 dBitmapModeItem = dFirstModeItem;
static const uint8 dIndexedModeItem = dLastModeItem-1;
static const uint8 dInvertItem = 9;
static const uint8 dModeGroupItem = 12;
static const uint8 dConvertAlpha = 14;

//-------------------------------------------------------------------------------
//	Prototypes.
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//
//	DoAbout
//
//	Displays the About box.
//
//	Inputs:
//		AboutRecordPtr about	Is actually a pointer to some platform
//								specific data for Windows only.
//		AboutID					#define with ID of String resource to use for text
//
//	Outputs:
//		Displays About box (Dialog).  Press Escape, Enter, or Return to
//		dismiss, or click the mouse.
//
//-------------------------------------------------------------------------------

void DoAbout (AboutRecordPtr /*about*/)
{
	ShowAbout (AboutID);
} // end DoAbout

/*****************************************************************************/

#if !__LP64__

/*****************************************************************************/

/* Setup the parameters dialog.  Returns TRUE if it succeeds.				 */

Boolean OpenOurDialog (GPtr globals)
	{
	
	if (gDialog != NULL)
		return TRUE;
	
	gDialogHdl = (DialogTHndl) GetResource ('DLOG', uiID);
	
	if (gDialogHdl == NULL || *gDialogHdl == NULL)
		goto failure;
		
	HNoPurge ((Handle) gDialogHdl);

	CenterDialog (gDialogHdl);

	gDialog = GetNewDialog (uiID, nil, (WindowPtr) -1);
	
	if (gDialog == NULL)
		goto failure;
		
	/* I am throwing away the results from these routines. 
	   Toolbox TechNote 37 does not document what error values they return.
	   Also, the worst that happens is that the interface isn't quite right. */
	
	(void) SetDialogDefaultItem (gDialog, ok);
	(void) SetDialogCancelItem (gDialog, cancel);
	(void) SetDialogTracksCursor (gDialog, TRUE);
	
	SetOutlineGroup (gDialog, dModeGroupItem);
	
	StuffNumber (gDialog, dRowsItem, gLastRows);
	StuffNumber (gDialog, dColsItem, gLastCols);
	SetRadioGroupState (gDialog,
						dFirstModeItem,
						dLastModeItem,
						gLastMode - plugInModeBitmap + dFirstModeItem);
	SetCheckBoxState (gDialog, dInvertItem, gLastInvert && gStuff->canReadBack);
	SetCheckBoxState (gDialog, dConvertAlpha, gConvertAlpha && gHostIsPhotoshop401orLater);
	
	/* We will only be inverting if we can read the image back. */
		
	if (gStuff->canReadBack)
		PIEnableControl (gDialog, dInvertItem);
	else
		PIDisableControl (gDialog, dInvertItem);
	
	if (gHostIsPhotoshop401orLater)
		PIEnableControl (gDialog, dConvertAlpha);
	else
		PIDisableControl (gDialog, dConvertAlpha);
	
	SelectTextItem (gDialog, dRowsItem);
		
	return TRUE;
		
	failure:
	
	if (gDialogHdl != NULL && *gDialogHdl != NULL)
		HNoPurge ((Handle) gDialogHdl);
		
	gDialog = NULL;
	gDialogHdl = NULL;
	
	return FALSE;
	
	}

/*****************************************************************************/
/* Run the parameters dialog.  Returns TRUE if it succeeds.					 */

Boolean RunOurDialog (GPtr globals)
	{
	
	short item, lastitem;
	short numberErr;
	
	int32 x = 0;
	
	SelectWindow (GetDialogWindow(gDialog));
	
	do
	{
		
		MoveableModalDialog (gDialog, gStuff->processEvent, nil, &item);
		
		if (lastitem != item && item != cancel)
		{
			switch (lastitem)
			{ /* we just left this area.  Make sure its within bounds. */
				case dRowsItem:
					numberErr = FetchNumber(gDialog,
					    dRowsItem,
					    kRowsMin,
					    kRowsMax,
					    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(gDialog,
									dRowsItem,
									kRowsMin,
									kRowsMax,
									&x,
								    AlertID,
								    numberErr);
						item = dRowsItem; // stay here
					}
					break;
				case dColsItem:
					numberErr = FetchNumber(gDialog,
					    dColsItem,
					    kColumnsMin,
					    kColumnsMax,
					    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(gDialog,
									dColsItem,
									kColumnsMin,
									kColumnsMax,
									&x,
								    AlertID,
								    numberErr);
						item = dColsItem; // stay here
					}
					break;
			}
		}	
		switch (item)
		{
			case dRowsItem:
				// grab the number whether it's right or not
				numberErr = FetchNumber(gDialog, dRowsItem, kRowsMin, kRowsMax, &x);
				if (numberErr == noErr)
				{ // no errors getting the number
					/* uncomment for future proxy
					gLastRows = x; 
					(void) DoFilterRect (globals);
					InvalItem(gDialog, kProxyItem);		// tell Dialog Mgr to update proxy item
					*/
				}
				break;
			case dColsItem:
				// grab the number whether it's right or not
				numberErr = FetchNumber(gDialog, dColsItem, kColumnsMin, kColumnsMax, &x);
				if (numberErr == noErr)
				{ // no errors getting the number
					/* uncomment for future proxy
					gLastCols = x; 
					(void) DoFilterRect (globals);
					InvalItem(gDialog, kProxyItem);		// tell Dialog Mgr to update proxy item
					*/
				}
				break;
			case dInvertItem:
				SetCheckBoxState ( gDialog, dInvertItem, !GetCheckBoxState(gDialog, dInvertItem));
				break;

			case dConvertAlpha:
				gConvertAlpha = !GetCheckBoxState(gDialog, dConvertAlpha);
				SetCheckBoxState ( gDialog, dConvertAlpha, gConvertAlpha);
				break;
			default:
				if (item >= dFirstModeItem && item <= dLastModeItem)
				{
					SetRadioGroupState (gDialog, dFirstModeItem, dLastModeItem, item);
					if (gHostIsPhotoshop401orLater)
					{
						if (item == dBitmapModeItem || item == dIndexedModeItem)
						{
							SetCheckBoxState(gDialog, dConvertAlpha, false);
							PIDisableControl(gDialog, dConvertAlpha);
						}
						else
						{
							SetCheckBoxState(gDialog, dConvertAlpha, gConvertAlpha);
							PIEnableControl(gDialog, dConvertAlpha);
						}
					}
				}
				break;
		}
		lastitem = item;
	}
	while (item != ok && item != cancel);
	
	if (item == ok)
	{ // lastitem routine has already validated these values before we got here
		FetchNumber(gDialog, dRowsItem, kRowsMin, kRowsMax, &x);
		gLastRows = (short) x;
		FetchNumber(gDialog, dColsItem, kColumnsMin, kColumnsMax, &x);
		gLastCols = (short) x;
		x = GetRadioGroupState(gDialog, dFirstModeItem, dLastModeItem)
					 - dFirstModeItem + plugInModeBitmap;
		
		gLastMode = (short) x;

		if (gStuff->canReadBack)
			gLastInvert = GetCheckBoxState(gDialog, dInvertItem);
	
		gConvertAlpha = gHostIsPhotoshop401orLater && GetCheckBoxState(gDialog, dConvertAlpha);
	}

	return (item == ok);
	
	}

/*****************************************************************************/

/* Close the parameters dialog.					 							 */

void CloseOurDialog (GPtr globals)
	{
	
	if (gDialog != NULL)
		DisposeDialog (gDialog);
		
	if (gDialogHdl != NULL && *gDialogHdl != NULL)
		HPurge ((Handle) gDialogHdl);

	gDialog = NULL;
	gDialogHdl = NULL;
	
	}

/*****************************************************************************/

#else // #if !__LP64__

/*****************************************************************************/

Boolean OpenOurDialog (GPtr globals)
{
	return FALSE;
}

/*****************************************************************************/

Boolean RunOurDialog (GPtr globals)
{
	return FALSE;
}

/*****************************************************************************/

void CloseOurDialog (GPtr globals)
{
}

#endif // #if !__LP64__

/*****************************************************************************/

// end GradientImportUIMac.cpp