// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		PoorMansTypeToolUIMac.r
//
//	Description:
//		This file contains the Dialog source for the
//		Filter module Poor Man's Type Tool.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Set up included files for Macintosh.
//-------------------------------------------------------------------------------

#include "PIDefines.h"

#include "Types.r"
//-------------------------------------------------------------------------------
//	Dialog resource
//-------------------------------------------------------------------------------
resource 'MENU' (16002) {
	16002,
	textMenuProc,
	allEnabled,
	enabled,
	"Hello",
	{	/* array: 7 elements */
		/* [1] */
		"1", noIcon, noKey, noMark, plain,
		/* [2] */
		"2", noIcon, noKey, noMark, plain,
		/* [3] */
		"4", noIcon, noKey, noMark, plain,
		/* [4] */
		"8", noIcon, noKey, noMark, plain,
		/* [5] */
		"16", noIcon, noKey, noMark, plain,
		/* [6] */
		"32", noIcon, noKey, noMark, plain,
		/* [7] */
		"64", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (16003) {
	16003,
	textMenuProc,
	allEnabled,
	enabled,
	"Hello",
	{	/* array: 1 elements */
		/* [1] */
		"makethislonger", noIcon, noKey, noMark, plain,
	}
};

resource 'MENU' (16004) {
	16004,
	textMenuProc,
	allEnabled,
	enabled,
	"Hello",
	{ /* array: 1 elements */
	  /* [1] */
	  "makethislonger", noIcon, noKey, noMark, plain,
	 }
};

resource 'CNTL' (16002, purgeable) {
	{128, 236, 144, 292},
	0,
	visible,
	0,
	16002,
	1009,
	0,
	""
};

resource 'CNTL' (16003, purgeable) {
	{8, 200, 28, 300},
	0,
	visible,
	0,
	16003,
	1009,
	0,
	""
};

resource 'CNTL' (16004, purgeable) {
	{40, 200, 60, 300},
	0,
	visible,
	0,
	16004,
	1009,
	0,
	""
};

resource 'DLOG' (16001, "Poor Mans UI", purgeable) {
	{150, 150, 400, 550},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"Poor Mans",
	centerParentWindowScreen
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (16001, "Poor Mans UI", purgeable) {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{8, 316, 28, 384},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{40, 316, 60, 384},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{72, 200, 88, 280},
		StaticText {
			disabled,
			"Vertical:"
		},
		/* [4] */
		{72, 355, 88, 384},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{100, 200, 116, 280},
		StaticText {
			disabled,
			"Horizontal:"
		},
		/* [6] */
		{100, 355, 116, 384},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{132, 200, 148, 280},
		StaticText {
			disabled,
			"Size:"
		},
		/* [8] */
		{132, 328, 148, 384},
		Control {
			enabled,
			16002
		},
		/* [9] */
		{172, 260, 188, 384},
		CheckBox {
			enabled,
			"Gaussian Blur"
		},
		/* [10] */
		{192, 260, 208, 384},
		CheckBox {
			enabled,
			"Show All Layers"
		},
		/* [11] */
		{8, 8, 230, 190},
		UserItem {
			disabled
		},
		/* [12] */
		{212, 200, 260, 384},
		StaticText {
			disabled,
			"Warning! Preview may be incorrect."
		},
		/* [13] */
		{8, 200, 28, 300},
		Control {
			enabled,
			16003
		},
		/* [14] */
		{40, 200, 60, 300},
		Control {
			enabled,
			16004
		}
	}
};
// end PoorMansTypeToolUIMac.r