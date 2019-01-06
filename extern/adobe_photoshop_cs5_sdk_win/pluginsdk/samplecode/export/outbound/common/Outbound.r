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
//		Outbound.r
//
//	Copyright 1990-1992, Thomas Knoll.
//	All Rights Reserved.
//
//	Description:
//		This file contains the resource information
//		for the Export module Outbound, a module that
//		creates a file and stores raw pixel data in it.
//
//	Use:
//		This module shows how to export raw data to a file.
//		It uses a simple "FileUtilities" library that comes
//		with the SDK.  You use it via File>>Export>>Outbound.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Definitions -- Required by include files.
//-------------------------------------------------------------------------------

// The About box and resources are created in PIUtilities.r.
// You can easily override them, if you like.

#define plugInName			"Outbound"
#define plugInCopyrightYear	"1993"
#define plugInDescription \
	"An Export plug-in module for Adobe Photoshop�."

//-------------------------------------------------------------------------------
//	Definitions -- Required by other resources in this rez file.
//-------------------------------------------------------------------------------

// Dictionary (aete) resources:

#define vendorName				"AdobeSDK"
#define plugInAETEComment		"history export plug-in"

#define plugInSuiteID			'sdK5'
#define plugInClassID			'outB'
#define plugInEventID			typeNull // must be this

//-------------------------------------------------------------------------------
//	Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"

#ifdef __PIMac__
	#include "Types.r"
	#include "SysTypes.r"
	#include "PIGeneral.r"
	#include "PIUtilities.r"
#elif defined(__PIWin__)
	#define Rez
	#include "PIGeneral.h"
	#include "PIUtilities.r"
#endif

#include "PITerminology.h"
#include "PIActions.h"

#include "OutboundTerminology.h"	// Terminology specific to this plug-in.

//-------------------------------------------------------------------------------
//	PiPL resource
//-------------------------------------------------------------------------------

resource 'PiPL' (ResourceID, plugInName " PiPL", purgeable)
{
	{
		Kind { Export },
		Name { plugInName "..." },
		Version { (latestExportVersion << 16) | latestExportSubVersion },

		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "PluginMain" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "PluginMain" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "PluginMain" },
			#else
				CodeWin32X86 { "PluginMain" },
			#endif
		#endif

		HasTerminology
		{
			plugInClassID,		// class ID
			plugInEventID,		// event ID
			ResourceID,			// aete ID
			"" 					// uniqueString
		},
		
		SupportedModes
		{
			noBitmap, doesSupportGrayScale,
			doesSupportIndexedColor, doesSupportRGBColor,
			doesSupportCMYKColor, doesSupportHSLColor,
			doesSupportHSBColor, doesSupportMultichannel,
			doesSupportDuotone, doesSupportLABColor
		},
			
		EnableInfo
		{
			"in (PSHOP_ImageMode, GrayScaleMode,"
			"IndexedColorMode, RGBMode,"
			"CMYKMode, HSLMode,"
		    "HSBMode, MultichannelMode,"
		    "DuotoneMode, LabMode)"
		}

	}
};

//-------------------------------------------------------------------------------
//	PiMI resource (Photoshop 2.5 and other older hosts)
//-------------------------------------------------------------------------------

resource 'PiMI' (ResourceID, plugInName " PiMI", purgeable)
{ 
	  latestExportVersion,
	  latestExportSubVersion,
	  0,
	  supportsGrayScale +
	  supportsIndexedColor +
	  supportsRGBColor +
	  supportsCMYKColor +
	  supportsHSLColor +
	  supportsHSBColor +
	  supportsMultichannel +
	  supportsDuotone +
	  supportsLABColor,
	  '    ', /* No required host */
	  {},
};

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (ResourceID, plugInName " dictionary", purgeable)
{
	1, 0, english, roman,									/* aete version and language specifiers */
	{
		vendorName,											/* vendor suite name */
		"Adobe example plug-ins",							/* optional description */
		plugInSuiteID,										/* suite ID */
		1,													/* suite code, must be 1 */
		1,													/* suite level, must be 1 */
		{},													/* structure for filters */
		{													/* non-filter plug-in class here */
			vendorName " " plugInName,						/* unique class name */
			plugInClassID,										/* class ID, must be unique or Suite ID */
			plugInAETEComment,								/* optional description */
			{												/* define inheritance */
				"<Inheritance>",							/* must be exactly this */
				keyInherits,								/* must be keyInherits */
				classExport,								/* parent: Format, Import, Export */
				"parent class export",						/* optional description */
				flagsSingleProperty,						/* if properties, list below */
				
				"in",										/* our path */
				keyIn,										/* common key */
				typePlatformFilePath,						/* correct path for platform */
				"file path",								/* optional description */
				flagsSingleProperty
				
				/* no more properties */
			},
			{}, /* elements (not supported) */
			/* class descriptions */
		},
		{}, /* comparison ops (not supported) */
		{}	/* any enumerations */
	}
};

//-------------------------------------------------------------------------------
//	Resource text entries
//
//	Entering these as separate resources because then they
//	transfer directly over to windows via CNVTPIPL.
//
//	If I use a Macintosh 'STR#' resource, I could put all these
//	strings into a single resource, but there is no
//	parallel on Windows.  'STR ' resources, which hold
//	one string per ID, exist on Windows and Macintosh.
//-------------------------------------------------------------------------------

// Prompt string:
resource StringResource (kPrompt, plugInName " Prompt", purgeable)
{
	"Outbound export to file:"
};

// Creator and type:
resource StringResource (kCreatorAndType, plugInName " CreatorAndType", purgeable)
{
	"????"		// creator
	"????"		// type

};

//-------------------------------------------------------------------------------

// end Outbound.r
