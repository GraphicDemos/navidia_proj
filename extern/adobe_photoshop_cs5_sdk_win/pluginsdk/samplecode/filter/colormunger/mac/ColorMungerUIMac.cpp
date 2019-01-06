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
//		ColorMungerUIMac.cpp
//
//	Copyright 1996-1997, Adobe Systems Incorporated.
//	All Rights Reserved.
//
//	Description:
//		This file contains the user interface source
//		for the Filter module ColorMunger, a module
//		showing the use of the Color Services suite.
//
//	Use:
//		This module takes a source color of any color space
//		and converts it to a target color in any color
//		space.  It shows how to convert colors as well as
//		pop the color picker.  It appears in
//		Filters>>Utilities>>ColorMunger.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "ColorMunger.h"

//-------------------------------------------------------------------------------
//	Definitions -- Dialog items
//-------------------------------------------------------------------------------

const uint16 kMin =	0;
const uint16 kMax =	32767;

const uint8 kPrecision = 6;

const uint8 dSourceRGB = 3;
const uint8 dSourceHSB = dSourceRGB+1;
const uint8 dSourceCMYK = dSourceHSB+1;
const uint8 dSourceLab = dSourceCMYK+1;
const uint8 dSourceGray = dSourceLab+1;
const uint8 dSourceHSL = dSourceGray+1;
const uint8 dSourceXYZ = dSourceHSL+1;
const uint8 dSourcePicker = dSourceXYZ+1;

const uint8 dSource1 = dSourceRGB;
const uint8 dSourceEnd = dSourceXYZ;

const uint8 dESource1 = 11;
const uint8 dESource2 = dESource1+1;
const uint8 dESource3 = dESource2+1;
const uint8 dESource4 = dESource3+1;

const uint8 dEditSource1 = dESource1;
const uint8 dEditSourceEnd = dESource4;

const uint8 dSourceGroupItem = 15;

const uint8 dTargetRGB = 17;
const uint8 dTargetHSB = dTargetRGB+1;
const uint8 dTargetCMYK = dTargetHSB+1;
const uint8 dTargetLab = dTargetCMYK+1;
const uint8 dTargetGray = dTargetLab+1;
const uint8 dTargetHSL = dTargetGray+1;
const uint8 dTargetXYZ = dTargetHSL+1;
const uint8 dTargetPicker = dTargetXYZ+1;

const uint8 dTarget1 = dTargetRGB;
const uint8 dTargetEnd = dTargetXYZ;

const uint8 dETarget1 = 25;
const uint8 dETarget2 = dETarget1+1;
const uint8 dETarget3 = dETarget2+1;
const uint8 dETarget4 = dETarget3+1;

const uint8 dEditTarget1 = dETarget1;
const uint8 dEditTargetEnd = dETarget4;

const uint8 dTargetGroupItem = 29;

//-------------------------------------------------------------------------------
//	Prototypes.
//-------------------------------------------------------------------------------

short Validate(DialogPtr dp, short *item, double *dx, Boolean useAlert);
void UpdateDialogInfo (GPtr globals, DialogPtr dp);
OSErr DoConversion (GPtr globals, DialogPtr dp, ColorServicesInfo *csinfo);

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

/* Display modal dialog, using proxy routines to preview results. */

Boolean DoUI (GPtr globals)

	{

	long		x = 1;
	double		dx = 0;
	short		numberErr = noErr;
	short		item, lastItem = 0, lastTitem = dEditSource1;
	short		cColor = gColor;
	int16		cSourceColor[4];
	short		cColorize = gColorize;
	int16		cTargetColor[4];	
	DialogPtr	dp;
	DialogTHndl	dt;
	PIType		key = NULLID;
	ColorServicesInfo	csinfo;
	
	CSCopyColor(cSourceColor, gSourceColor);
	CSCopyColor(cTargetColor, gTargetColor);
	
	dt = (DialogTHndl) GetResource ('DLOG', uiID);
	HNoPurge ((Handle) dt);
	
	CenterDialog (dt);

	dp = GetNewDialog (uiID, nil, (WindowPtr) -1);

	(void) SetDialogDefaultItem (dp, ok);
	(void) SetDialogCancelItem (dp, cancel);
	(void) SetDialogTracksCursor (dp, TRUE);
		
	SetOutlineGroup (dp, dSourceGroupItem);
	SetOutlineGroup (dp, dTargetGroupItem);

	UpdateDialogInfo (globals, dp);
	
	SelectTextItem (dp, lastTitem);
	
	ShowWindow (GetDialogWindow(dp));

	do
	{
		
		MoveableModalDialog (dp, gStuff->processEvent, nil, &item);
	
		if (lastItem != item &&
			item != cancel)
		{ // if its something that doesn't blast away everything else,
		  // we need to go back and validate the last field, then.
			x = lastItem;
			switch (lastItem)
			{ // We just left this field, so check for valid bounds.	
				
				// put other non-number cases here
				default:
					if ((numberErr = Validate(dp, &lastItem, &dx, true)) != noErr)
						item = x; // force this number to be stored in next switch
					break;

			} // end switch
		} // end if
		
		switch (item)
		{ // now check what item we just clicked/used
			case dSourcePicker:
				{
					int16 ioSpace = gColor;
					int16 ioColor[4];
					Str255 prompt = "";
					OSErr err = noErr;
					
					PIGetString(kPromptSource, prompt);
					CSCopyColor (ioColor, gSourceColor);
					
					err = CSPickColor(prompt, &ioSpace, ioColor);
					if (err == noErr)
					{ // Copy both to globals and csinfo to do
					  // conversion:
						CSCopyColor (gSourceColor, ioColor);
						CSCopyColor (csinfo.colorComponents, ioColor);
						gColor = csinfo.sourceSpace = ioSpace;

						DoConversion (globals, dp, &csinfo);
						UpdateDialogInfo (globals, dp);
					}
				}
				SelectTextItem(dp, lastTitem);
				break;
			case dTargetPicker:
				{
					int16 ioSpace = gColorize;
					int16 ioColor[4];
					Str255 prompt = "";
					OSErr err = noErr;
					
					PIGetString(kPromptTarget, prompt);
					CSCopyColor (ioColor, gTargetColor);
					
					err = CSPickColor(prompt, &ioSpace, ioColor);
					if (err == noErr)
					{
						CSCopyColor (gSourceColor, gTargetColor);
						gColor = gColorize;
						CSCopyColor (gTargetColor, ioColor);
						gColorize = ioSpace;
					
						UpdateDialogInfo (globals, dp);
					}
				}
				SelectTextItem(dp, lastTitem);
				break;
			default:
				if (item >= dSource1 && item <= dSourceEnd)
				{
					SetRadioGroupState (dp, dSource1, dSourceEnd, item);
					gColor = item - dSource1;
					
					DoConversion (globals, dp, &csinfo);
					SelectTextItem(dp, lastTitem);
				}
				else if (item >= dTarget1 && item <= dTargetEnd)
				{ // set group then do conversion
					SetRadioGroupState (dp, dTarget1, dTargetEnd, item);
					gColorize = item - dTarget1;
					
					DoConversion (globals, dp, &csinfo);
					SelectTextItem(dp, lastTitem);
				}
				else if (item >= dEditSource1 && item <= dEditSourceEnd)
				{
					numberErr = Validate(dp, &item, &dx, false);
					gSourceColor[item - dEditSource1] = (int16)dx;
					lastTitem = item;

					DoConversion (globals, dp, &csinfo);
				}
		} // end switch
		lastItem = item;
		
	} while (item != ok && item != cancel);

	DisposeDialog (dp);
	HPurge ((Handle) dt);

	if (item == cancel)
	{ // return to orig params
		CSCopyColor(gSourceColor, cSourceColor);
		CSCopyColor(gTargetColor, cTargetColor);
		gColor = cColor;
		gColorize = cColorize;
		
		gResult = userCanceledErr;
	}
	
	if ( gUserItemOutlineOK != NULL )
		DisposeUserItemUPP(gUserItemOutlineOK);
	if ( gUserItemOutlineGroup != NULL )
		DisposeUserItemUPP(gUserItemOutlineGroup);
	if ( gFilterProc != NULL )
		DisposeModalFilterUPP(gFilterProc);

	return (item == ok);
}

/*****************************************************************************/
// Loops through values and automatically grabs and validates item.  Returns
// noErr if okay, otherwise focus has been forced back to invalid item.

short Validate(DialogPtr dp, short *item, double *dx, Boolean useAlert)
{
	short 	numberErr = noErr;
	long 	x = 0; // returned long
	short 	loop = 1;
	
	const double propDoubles[] = {
		0, 0, 0
		/* iPropBigNudgeH, kNudgeMin, kNudgeMax,
		iPropBigNudgeV, kNudgeMin, kNudgeMax,
		iPropRulerOriginH, kOriginMin, kOriginMax,
		iPropRulerOriginV, kOriginMin, kOriginMax,
		iPropGridMajor, kGridMin, kGridMax */
		};

	const short	numPropDoubles = 
		(propDoubles[0] > 0) ? ((sizeof(propDoubles) / sizeof(double)) / 3) : 0;

	const long	propNumbers[] = {
		dESource1, kMin, kMax,
		dESource2, kMin, kMax,
		dESource3, kMin, kMax,
		dESource4, kMin, kMax,
		};

	const short	numPropNumbers = 
		(propNumbers[0] > 0) ? ((sizeof(propNumbers) / sizeof(long)) / 3) : 0;
	
	const short foundIt = -1; // flag that we found it

	
	// first loop through doubles, looking for match
	while (loop <= numPropDoubles && loop != foundIt)
	{
		if (propDoubles[loop*3-3] == *item)
		{
			numberErr = FetchDouble (dp,
									 *item,
									 propDoubles[loop*3-2], // max
									 propDoubles[loop*3-1], // min
									 dx); // returned double
			if (numberErr != noErr && useAlert)
			{ // out of bounds problem.  Report and change focus back.
				AlertDouble (dp,
							 *item,
							 propDoubles[loop*3-2], // max
							 propDoubles[loop*3-1], // min
							 dx, // value
							 AlertID, // global alert dialog
							 numberErr); // error we found
				*item = 0;
			}
			loop = foundIt; // found it!
		} else loop++; // next
	}

	if (loop != foundIt) loop = 1; // reset
	// now loop through longs, looking for match
	while (loop <= numPropNumbers && loop != foundIt)
	{
		if (propNumbers[loop*3-3] == *item)
		{
			numberErr = FetchNumber (dp,
									 *item,
									 propNumbers[loop*3-2], // max
									 propNumbers[loop*3-1], // min
									 &x); // returned long
			if (numberErr != noErr && useAlert)
			{ // out of bounds problem.  Report and change focus back.
				AlertNumber (dp,
							 *item,
							 propNumbers[loop*3-2], // max
							 propNumbers[loop*3-1], // min
							 &x, // value
							 AlertID, // global alert dialog
							 numberErr); // error we found
				*item = 0;
			}
			*dx = (double)x; // coerce long as returned double
			loop = foundIt; // found it!
		} else loop++; // next
	}
	return numberErr;
}

/*******************************************************************/

void UpdateDialogInfo (GPtr globals, DialogPtr dp)
{
	short loop = 0;
	
	SetRadioGroupState (dp, dSource1, dSourceEnd, dSource1 + gColor);
	SetRadioGroupState (dp, dTarget1, dTargetEnd, dTarget1 + gColorize);

	for (loop = 0; loop < 4; loop++)
		StuffNumber(dp, dEditSource1 + loop, gSourceColor[loop]);
	for (loop = 0; loop < 4; loop++)
		StuffNumber(dp, dEditTarget1 + loop, gTargetColor[loop]);
}

/*******************************************************************/

OSErr DoConversion (GPtr globals, DialogPtr dp, ColorServicesInfo *csinfo)
{
	OSErr	gotErr = noErr;
	short 	loop = 0;
	
	PopulateColorServicesInfo (globals, csinfo);
	gotErr = ColorServices (csinfo);
	gColorize = csinfo->resultSpace; // should be the same
	CSCopyColor (gTargetColor, csinfo->colorComponents);
	for (loop = 0; loop < 4; loop++)
		StuffNumber(dp, dEditTarget1 + loop, gTargetColor[loop]);

	return gotErr;
}

//-------------------------------------------------------------------------------

#else // #if !__LP64__

Boolean DoUI (GPtr globals)
{
	return FALSE;
}

//-------------------------------------------------------------------------------

#endif // #if !__LP64__

//-------------------------------------------------------------------------------

// end ColorMungerUIMac.cpp
