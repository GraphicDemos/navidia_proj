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
//		SelectoramaUIMac.r
//
//	Description:
//		This file contains the Dialog resource.
//
//
//-------------------------------------------------------------------------------

#include "Types.r"

//-------------------------------------------------------------------------------
//	Dialog resource
//-------------------------------------------------------------------------------

resource 'DLOG' (16001, "Selectorama  UI", purgeable)
{
	{259, 337, 383, 729},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"Selectorama",
	centerParentWindowScreen
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (16001, "Selectorama  UI", purgeable)
{
	{	/* array DITLarray: 16 elements */
		/* [1] */
		{8, 312, 28, 380},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{36, 312, 56, 380},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 35, 36, 76},
		StaticText {
			disabled,
			"Area:"
		},
		/* [4] */
		{8, 79, 24, 183},
		RadioButton {
			enabled,
			"Minimum"
		},
		/* [5] */
		{24, 79, 40, 183},
		RadioButton {
			enabled,
			"Maximum"
		},
		/* [6] */
		{40, 79, 56, 159},
		RadioButton {
			enabled,
			"Random"
		},
		/* [7] */
		{40, 167, 64, 231},
		StaticText {
			disabled,
			"Amount:"
		},
		/* [8] */
		{40, 235, 56, 264},
		EditText {
			enabled,
			"50"
		},
		/* [9] */
		{40, 270, 64, 287},
		StaticText {
			disabled,
			"%"
		},
		/* [10] */
		{72, 7, 100, 77},
		StaticText {
			disabled,
			"Channels:"
		},
		/* [11] */
		{72, 79, 88, 160},
		RadioButton {
			enabled,
			"Target"
		},
		/* [12] */
		{88, 79, 104, 160},
		RadioButton {
			enabled,
			"Merged"
		},
		/* [13] */
		{72, 171, 100, 228},
		StaticText {
			disabled,
			"Create:"
		},
		/* [14] */
		{72, 227, 88, 331},
		RadioButton {
			enabled,
			"Selection"
		},
		/* [15] */
		{88, 227, 104, 331},
		RadioButton {
			enabled,
			"Path"
		},
		/* [16] */
		{104, 227, 120, 331},
		RadioButton {
			enabled,
			"Layer"
		}
	}
};

//-------------------------------------------------------------------------------

// end SelectoramaUIMac.r
