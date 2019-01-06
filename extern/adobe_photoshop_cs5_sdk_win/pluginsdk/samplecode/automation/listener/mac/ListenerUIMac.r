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
//		ListenerUIMac.r
//
//	Description:
//		Dialog for the Listener Mac project. If you are a persistent plug in
//		you have to make sure your resources do not match the resources of other
//		plug ins. Conflicts will cause crashes.
//
//-------------------------------------------------------------------------------

#include "Types.r"
#include "PIUtilities.r"
#include "ListenerTerminology.h"

resource 'CNTL' (18501, purgeable) {
	{20, 120, 45, 330},
	0,
	visible,
	0,
	18501,
	1008,
	0,
	""
};

resource 'CNTL' (18502, purgeable) {
	{60, 120, 85, 330},
	0,
	visible,
	0,
	18502,
	1008,
	0,
	""
};

resource 'CNTL' (18503, purgeable) {
	{100, 120, 125, 330},
	0,
	visible,
	0,
	18503,
	1008,
	0,
	""
};

resource 'DITL' (18501, "Listener UI", purgeable) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{8, 350, 28, 418},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{40, 350, 60, 418},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{23, 2, 44, 114},
		StaticText {
			disabled,
			"Watch for event: "
		},
		/* [4] */
		{20, 120, 45, 330},
		Control {
			enabled,
			18501
		},
		/* [5] */
		{63, 2, 80, 114},
		StaticText {
			disabled,
			"Then play set: "
		},
		/* [6] */
		{60, 120, 85, 330},
		Control {
			enabled,
			18502
		},
		/* [7] */
		{103, 2, 120, 114},
		StaticText {
			disabled,
			"...action: "
		},
		/* [8] */
		{100, 120, 125, 330},
		Control {
			enabled,
			18503
		},
		/* [9] */
		{143, 2, 160, 130},
		StaticText {
			disabled,
			"Installed notifiers: "
		},
		/* [10] */
		{161, 10, 300, 424},
		StaticText {
			disabled,
			""
		}
	}
};

resource 'DLOG' (18501, "Listener UI", purgeable) {
	{202, 549, 445, 973},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	18501,
	"Listener",
	centerParentWindowScreen
};

resource 'MENU' (18501) {
	18501,
	textMenuProc,
	allEnabled,
	enabled,
	"Watch for",
	{	/* array: 4 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'MENU' (18502) {
	18502,
	textMenuProc,
	allEnabled,
	enabled,
	"Action Set",
	{	/* array: 2 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'MENU' (18503) {
	18503,
	textMenuProc,
	allEnabled,
	enabled,
	"Action Item",
	{	/* array: 6 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'dlgx' (18501) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

//-------------------------------------------------------------------------------
//	About dialog box.
//-------------------------------------------------------------------------------

resource 'DLOG' (kListenerAboutID, plugInName " Generic About Box", purgeable)
{
	{20, 0, 214, 390},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	ResourceID,
	"About " plugInName "...",
	alertPositionMainScreen	// Universal 3.0 headers.
};

resource 'dlgx' (kListenerAboutID) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (kListenerAboutID, plugInName " Generic About Box", purgeable)
{
	{
		{-80, 0, -60, 60}, 		Button { enabled, "Hidden" },
		{5, 5, 155, 380}, 		UserItem { enabled },
		{5, 5, 155, 380},		StaticText { disabled, "^0\n^1\n^2\n" },
		{160, 10, 180, 190}, 	Button { enabled, "www.adobe.com" },
		{160, 200, 180, 380},	Button { enabled, "partners.adobe.com" } 
	}
};

// end ListenerUIMac.r
