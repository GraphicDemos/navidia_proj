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
//		ColorMungerUIMac.r
//
//	Description:
//		This file contains the Dialog resource information
//		for the Filter module ColorMunger, a module
//		showing the use of the Color Services suite.
//
//
//-------------------------------------------------------------------------------

#include "PIDefines.h"

#include "Types.r"
//-------------------------------------------------------------------------------
//	Dialog resource
//-------------------------------------------------------------------------------

resource 'DLOG' (16001, "ColorMunger UI", purgeable)
{
	{102, 148, 286, 644},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"ColorMunger",
	centerParentWindowScreen
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (16001, "ColorMunger UI", purgeable)
{
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{4, 420, 24, 488},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{40, 420, 60, 488},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{36, 16, 52, 100},
		RadioButton {
			enabled,
			"RGB"
		},
		/* [4] */
		{52, 16, 68, 100},
		RadioButton {
			enabled,
			"HSB"
		},
		/* [5] */
		{68, 16, 84, 100},
		RadioButton {
			enabled,
			"CMYK"
		},
		/* [6] */
		{84, 16, 100, 100},
		RadioButton {
			enabled,
			"Lab"
		},
		/* [7] */
		{100, 16, 116, 100},
		RadioButton {
			enabled,
			"Gray"
		},
		/* [8] */
		{116, 16, 132, 100},
		RadioButton {
			enabled,
			"HSL"
		},
		/* [9] */
		{132, 16, 148, 100},
		RadioButton {
			enabled,
			"XYZ"
		},
		/* [10] */
		{150, 16, 166, 80},
		Button {
			enabled,
			"Picker"
		},
		/* [11] */
		{40, 109, 56, 194},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{67, 109, 83, 194},
		EditText {
			enabled,
			""
		},
		/* [13] */
		{94, 109, 110, 194},
		EditText {
			enabled,
			""
		},
		/* [14] */
		{121, 109, 137, 194},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{16, 12, 173, 208},
		UserItem {
			disabled
		},
		/* [16] */
		{8, 16, 28, 68},
		StaticText {
			disabled,
			"Source"
		},
		/* [17] */
		{36, 224, 52, 308},
		RadioButton {
			enabled,
			"RGB"
		},
		/* [18] */
		{52, 224, 68, 308},
		RadioButton {
			enabled,
			"HSB"
		},
		/* [19] */
		{68, 224, 84, 308},
		RadioButton {
			enabled,
			"CMYK"
		},
		/* [20] */
		{84, 224, 100, 308},
		RadioButton {
			enabled,
			"Lab"
		},
		/* [21] */
		{100, 224, 116, 308},
		RadioButton {
			enabled,
			"Gray"
		},
		/* [22] */
		{116, 224, 132, 308},
		RadioButton {
			enabled,
			"HSL"
		},
		/* [23] */
		{132, 224, 148, 308},
		RadioButton {
			enabled,
			"XYZ"
		},
		/* [24] */
		{150, 224, 166, 288},
		Button {
			enabled,
			"Picker"
		},
		/* [25] */
		{40, 312, 62, 385},
		StaticText {
			enabled,
			""
		},
		/* [26] */
		{67, 312, 89, 385},
		StaticText {
			enabled,
			""
		},
		/* [27] */
		{94, 312, 116, 385},
		StaticText {
			enabled,
			""
		},
		/* [28] */
		{121, 312, 143, 385},
		StaticText {
			enabled,
			""
		},
		/* [29] */
		{16, 216, 173, 390},
		UserItem {
			disabled
		},
		/* [30] */
		{8, 224, 28, 300},
		StaticText {
			disabled,
			"Convert to"
		}
	}
}; // end DITL

// end ColorMungerUIMac.r
