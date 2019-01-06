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
//		AutomationFilterUI.r
//
//	Description:
//		Dialog for the AutomationFilter Mac project.
//
//-------------------------------------------------------------------------------

#include "Types.r"

resource 'MENU' (16001) {
	16001,
	textMenuProc,
	allEnabled,
	enabled,
	"Layer",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'MENU' (16002) {
	16002,
	textMenuProc,
	allEnabled,
	enabled,
	"Channel",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};
resource 'CNTL' (16001, "Layer", purgeable) {
	{20, 115, 45, 325},
	0,
	visible,
	0,
	16001,
	1008,
	0,
	""
};

resource 'CNTL' (16002, "Channel", purgeable) {
	{60, 115, 85, 325},
	0,
	visible,
	0,
	16002,
	1008,
	0,
	""
};

resource 'DITL' (16001, "Automation Filter UI", purgeable) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{20, 360, 40, 428},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{48, 360, 68, 428},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{23, 2, 44, 60},
		StaticText {
			disabled,
			"Layer:"
		},
		/* [4] */
		{20, 72, 45, 202},
		Control {
			enabled,
			16001
		},
		/* [5] */
		{63, 2, 80, 60},
		StaticText {
			disabled,
			"Channel:"
		},
		/* [6] */
		{60, 72, 85, 202},
		Control {
			enabled,
			16002
		},
		/* [7] */
		{112, 20, 132, 88},
		Button {
			enabled,
			"0 %"
		},
		/* [8] */
		{112, 101, 132, 169},
		Button {
			enabled,
			"50 %"
		},
		/* [9] */
		{112, 182, 132, 250},
		Button {
			enabled,
			"100 %"
		},
		/* [10] */
		{112, 260, 132, 338},
		Button {
			enabled,
			"Random"
		},
		/* [11] */
		{140, 20, 156, 169},
		StaticText {
			disabled,
			"Tile Hieght:"
		},
		/* [12] */
		{140, 182, 156, 338},
		StaticText {
			disabled,
			"Tile Width:"
		}
	}
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DLOG' (16001, "Automation Filter UI", purgeable) {
	{103, 305, 267, 757},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"Automation Filter",
	centerParentWindowScreen
};
// end AutomationFilterUI.r
