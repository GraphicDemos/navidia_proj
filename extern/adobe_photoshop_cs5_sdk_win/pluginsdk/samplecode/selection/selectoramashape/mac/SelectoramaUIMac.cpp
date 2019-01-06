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
//		SelectoramaUIMac.cpp
//
//
//	Description:
//		This file contains the dialog and platform-specific functions
//		for the Selection module Selectorama, an example module
//		that just returns a pixel selection.
//
//	Use:
//		Use selection modules to return pixel or path selections on a new
//		layer or the current layer.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "Selectorama.h"

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

/* Setup the parameters dialog.  Returns TRUE if it succeeds.				 */

Boolean DoUI (GPtr globals)
{
	
	short 		item, lastitem;
	short		numberErr = noErr;
	long		x;
	DialogPtr	dp;
	DialogTHndl	dialogHdl;
	dialogHdl = (DialogTHndl) GetResource ('DLOG', uiID);

	if (dialogHdl == NULL || *dialogHdl == NULL)
		return false;
	else
	{
		
	HNoPurge ((Handle) dialogHdl);

	CenterDialog (dialogHdl);
	
	dp = GetNewDialog (uiID, nil, (WindowPtr) -1);
		
	/* I am throwing away the results from these routines. 
	   Toolbox TechNote 37 does not document what error values they return.
	   Also, the worst that happens is that the interface isn't quite right. */
	
	(void) SetDialogDefaultItem (dp, ok);
	(void) SetDialogCancelItem (dp, cancel);
	(void) SetDialogTracksCursor (dp, TRUE);
		
	SetRadioGroupState (dp,
						kFirstItem,
						kLastItem,
						kFirstItem + gWhatArea);
						
	SetRadioGroupState (dp,
						kUseRadio1,
						kUseRadioLast,
						kUseRadio1 + gWhatChannels);

	SetRadioGroupState (dp,
						kCreateRadio1,
						kCreateRadioLast,
						kCreateRadio1 + gCreate);
	
	ShowHideItem (dp, kPercentStatic, (gWhatArea == iSelectRandom));
	ShowHideItem (dp, kPercentEdit, (gWhatArea == iSelectRandom));
	ShowHideItem (dp, kPercentSymbol, (gWhatArea == iSelectRandom));
	StuffNumber(dp, kPercentEdit, gPercent);
	if (gWhatArea == iSelectRandom) SelectTextItem(dp, kPercentEdit);
		
	SelectWindow (GetDialogWindow(dp));
	
	do
	{
		
		MoveableModalDialog (dp, gStuff->processEvent, nil, &item);
		
		if (lastitem != item && item != cancel)
		{ /* we just left this area.  Check to make sure its within bounds. */
			switch (lastitem)
			{
				case kPercentEdit:
					numberErr = FetchNumber(dp,
										    kPercentEdit,
										    kPercentMin,
										    kPercentMax,
										    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(dp,
									kPercentEdit,
									kPercentMin,
									kPercentMax,
									&x,
								    AlertID,
								    numberErr);
						item = kPercentEdit; // stay here
					}
					gPercent = x;						
					break;
			}
		}
		switch (item)
		{
			case ok:
				gWhatArea = GetRadioGroupState(dp, kFirstItem, kLastItem) - kFirstItem;
				gWhatChannels = GetRadioGroupState(dp, kUseRadio1, kUseRadioLast) - kUseRadio1;
				gCreate = GetRadioGroupState(dp, kCreateRadio1, kCreateRadioLast) - kCreateRadio1;
				if (gWhatArea == iSelectRandom)
				{
					numberErr = FetchNumber(dp,
										    kPercentEdit,
										    kPercentMin,
										    kPercentMax,
										    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(dp,
									kPercentEdit,
									kPercentMin,
									kPercentMax,
									&x,
								    AlertID,
								    numberErr);
						item = kPercentEdit; // go back
					}
					else gPercent = x; // it's okay, move on
				}
				break;

			case cancel:
				gResult = userCanceledErr;
				break;
			case kPercentEdit:
				// grab the number whether it's right or not
				numberErr = FetchNumber(dp, kPercentEdit, kPercentMin, kPercentMax, &x);
				if (numberErr == noErr)
				{ // no errors getting the number
					gPercent = x;
					// update display here	 
				}
				break;

			default:
			if (item >= kFirstItem && item <= kLastItem)
			{
				SetRadioGroupState (dp, kFirstItem, kLastItem, item);
				ShowHideItem (dp, kPercentStatic, (item == iSelectRandom + kFirstItem));
				ShowHideItem (dp, kPercentEdit, (item == iSelectRandom + kFirstItem));
				ShowHideItem (dp, kPercentSymbol, (item == iSelectRandom + kFirstItem));
				if (item == iSelectRandom + kFirstItem) SelectTextItem(dp, kPercentEdit);
			}
			else if (item >= kUseRadio1 && item <= kUseRadioLast)
				SetRadioGroupState (dp, kUseRadio1, kUseRadioLast, item);
			else if (item >= kCreateRadio1 && item <= kCreateRadioLast)
				SetRadioGroupState (dp, kCreateRadio1, kCreateRadioLast, item);
			break;
		}
		lastitem = item;
	}
	while (item != ok && item != cancel);

	DisposeDialog (dp);
		
	if (dialogHdl != NULL && *dialogHdl != NULL)
		HPurge ((Handle) dialogHdl);
	
	dp = NULL;
	dialogHdl = NULL;
	
	return item == ok;
	
	
	} // else
}

/*********************************************************************/
// end SelectoramaUIMac.cpp