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
//		PropetizerUIMac.r
//
//	Description:
//		This file contains the Dialog resource information
//		for the Filter module Propetizer.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"

#include "Types.r"

//-------------------------------------------------------------------------------
//	Dialog resource
//-------------------------------------------------------------------------------

resource 'DLOG' (16001, "Propetizer UI", purgeable)
{
	{100, 150, 710, 600},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"Propetizer",
	centerParentWindowScreen
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (16001, "Propetizer UI", purgeable)
{
	{	/* array DITLarray: 47 elements */
		/* [1] */
		{8, 354, 28, 422},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{40, 354, 60, 422},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 10, 24, 280},
		StaticText {
			enabled,
			"static"
		},
		/* [4] */
		{26, 10, 42, 280},
		StaticText {
			enabled,
			"static"
		},
		/* [5] */
		{44, 10, 60, 280},
		StaticText {
			enabled,
			"static"
		},
		/* [6] */
		{62, 10, 78, 280},
		StaticText {
			enabled,
			"static"
		},
		/* [7] */
		{80, 10, 96, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [8] */
		{98, 10, 114, 170},
		StaticText {
			enabled,
			"static"
		},
		/* 9] */
		{116, 10, 132, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [10] */
		{134, 10, 150, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [11] */
		{152, 10, 168, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [12] */
		{170, 10, 186, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [13] */
		{188, 10, 204, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [14] */
		{98, 180, 114, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [15] */
		{116, 180, 132, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [16] */
		{134, 180, 150, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [17] */
		{152, 180, 168, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [18] */
		{170, 180, 186, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [19] */
		{188, 180, 204, 420},
		StaticText {
			enabled,
			"static"
		},
		/* [20] */
		{208, 10, 228, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [21] */
		{208, 180, 226, 420},
		Control {
			enabled,
			16001
		},
		/* [22] */
		{230, 10, 250, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [23] */
		{232, 180, 248, 420},
		Control {
			enabled,
			16002
		},
		/* [24] */
		{252, 10, 272, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [25] */
		{258, 180, 274, 420},
		Control {
			enabled,
			16003
		},
		/* [26] */
		{282, 10, 300, 170},
		StaticText {
			enabled,
			"static"
		},
		/* [27] */
		{281, 180, 299, 420},
		Control {
			enabled,
			16004
		},
		/* [28] */
		{306, 10, 324, 120},
		StaticText {
			enabled,
			"static"
		},
		/* [29] */
		{308, 130, 324, 190},
		EditText {
			enabled,
			"static"
		},
		/* [30] */
		{306, 220, 324, 270},
		StaticText {
			enabled,
			"static"
		},
		/* [31] */
		{308, 280, 324, 340},
		EditText {
			enabled,
			"static"
		},
		/* [32] */
		{330, 10, 348, 120},
		StaticText {
			enabled,
			"static"
		},
		/* [33] */
		{331, 130, 347, 190},
		EditText {
			enabled,
			"static"
		},
		/* [34] */
		{330, 220, 348, 270},
		StaticText {
			enabled,
			"static"
		},
		/* [35] */
		{331, 280, 347, 340},
		EditText {
			enabled,
			"static"
		},
		/* [36] */
		{354, 10, 372, 120},
		StaticText {
			enabled,
			"static"
		},
		/* [37] */
		{355, 130, 371, 190},
		EditText {
			enabled,
			"static"
		},
		/* [38] */
		{354, 220, 372, 270},
		StaticText {
			enabled,
			"static"
		},
		/* [39] */
		{355, 280, 371, 340},
		EditText {
			enabled,
			"static"
		},
		/* [40] */
		{378, 10, 396, 150},
		StaticText {
			enabled,
			"static"
		},
		/* [41] */
		{379, 160, 395, 220},
		EditText {
			enabled,
			"static"
		},
		/* [42] */
		{404, 10, 420, 120},
		StaticText {
			enabled,
			"static"
		},
		/* [43] */
		{405, 130, 419, 190},
		EditText {
			enabled,
			"static"
		},
		/* [44] */
		{404, 220, 420, 270},
		StaticText {
			enabled,
			"static"
		},
		/* [45] */
		{405, 280, 419, 340},
		EditText {
			enabled,
			"static"
		},
		/* [46] */
		{440, 10, 445, 170},
		CheckBox {
			enabled,
			"static"
		},
		/* [47] */
		{440, 180, 445, 420},
		CheckBox {
			enabled,
			"static"
		},
		/* [48] */
		{460, 10, 570, 170},
		EditText {
			enabled,
			"static"
		},
		/* [49] */
		{460, 180, 570, 420},
		EditText {
			enabled,
			"static"
		},
		/* [50] */
		{580, 10, 600, 270},
		StaticText {
			enabled,
			"static"
		},
	}
	
}; // end DITL

resource 'MENU' (16001) {
	16001,
	textMenuProc,
	allEnabled,
	enabled,
	"Channelnames",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'CNTL' (16001, "Channelnames", purgeable) {
	{208, 180, 226, 420},
	0,
	visible,
	0,
	16001,
	1008,
	0,
	""
};

resource 'MENU' (16002) {
	16002,
	textMenuProc,
	allEnabled,
	enabled,
	"Channelnames",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'CNTL' (16002, "Channelnames", purgeable) {
	{208, 180, 226, 420},
	0,
	visible,
	0,
	16002,
	1008,
	0,
	""
};

resource 'MENU' (16003) {
	16003,
	textMenuProc,
	allEnabled,
	enabled,
	"pathnames",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'CNTL' (16003, "pathnames", purgeable) {
	{233, 180, 249, 420},
	0,
	visible,
	0,
	16003,
	1008,
	0,
	""
};

resource 'MENU' (16004) {
	16004,
	textMenuProc,
	allEnabled,
	enabled,
	"interfacecolors",
	{	/* array: 1 elements */
		/* [1] */
		"temp", noIcon, noKey, noMark, plain,
	}
};

resource 'CNTL' (16004, "interfacecolors", purgeable) {
	{257, 180, 273, 420},
	0,
	visible,
	0,
	16004,
	1008,
	0,
	""
};
// end PropetizerUIMac.r
