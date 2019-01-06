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
//		GradientImportUIMac.r
//
//	Copyright 1990, Thomas Knoll.
//	All Rights Reserved.
//
//	Description:
//		This file contains the Dialog resource definitions for the
//		Import module GradientImport, a scriptable multiple-acquire
//		plug-in that creates 4 types of gradients in batches.
//
//	Use:
//		This import module is a great example of scripting
//		Import modules that use the old style Photoshop
//		multiple-acquire mechanism.
//
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

resource 'DLOG' (16001, "GradientImport UI", purgeable)
{
	{0, 0, 206, 255},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	16001,
	"GradientImport",
	centerParentWindowScreen
};

resource 'dlgx' (16001) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (16001, "GradientImport UI", purgeable)
{
	{
		{8, 180, 28, 248},   Button { enabled, "Import" },
		{40, 180, 60, 248},   Button { enabled, "Done" },
		{10, 90, 26, 135},	  EditText { enabled, "" },
		{36, 90, 52, 135},	  EditText { enabled, "" },
		{83, 30, 99, 150},    RadioButton { enabled, "Bitmap" },
		{99, 30, 115, 150},   RadioButton { enabled, "Grayscale" },
		{115, 30, 131, 150},  RadioButton { enabled, "Indexed Color" },
		{131, 30, 147, 150},  RadioButton { enabled, "RGB Color" },
		{165, 30, 181, 150},  CheckBox { enabled, "Invert" },
		{10, 40, 26, 85},	  StaticText { disabled, "Rows:" },
		{36, 20, 52, 85},	  StaticText { disabled, "Columns:" },
		{70, 20, 155, 160},   UserItem { disabled },
		{62, 30, 78, 67},	  StaticText { disabled, "Mode" },
		{181, 30, 200, 252},  CheckBox { enabled, "Convert alpha to transparency" }
	}
};
	
//-------------------------------------------------------------------------------

// end GradientImportUIMac.r
