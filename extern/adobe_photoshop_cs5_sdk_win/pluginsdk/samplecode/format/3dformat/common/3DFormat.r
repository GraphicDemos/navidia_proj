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
//		3DFormat.r
//
//	Description:
//		This file contains the resource definitions for the
//		File Format module 3DFormat, 
//		which writes a flat file with merged document pixels.
//
//	Use:
//		Format modules are called from the Save, Save as,
//		and Save a copy dialogs.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Definitions -- Required by include files.
//-------------------------------------------------------------------------------

// The About box and resources are created in PIUtilities.r.
// You can easily override them, if you like.

#define plugInName			"3DFormat"
#define plugInCopyrightYear	"1993"
#define plugInDescription \
	"An example plug-in file format module for Adobe Photoshop¨."

//-------------------------------------------------------------------------------
//	Definitions -- Required by other resources in this rez file.
//-------------------------------------------------------------------------------

// Dictionary (aete) resources:

#define vendorName			"AdobeSDK"
#define plugInAETEComment 	"3DFormat example file format module"

#define plugInSuiteID		'sdK4'
#define plugInClassID		'simP'
#define plugInEventID		typeNull // must be this

//-------------------------------------------------------------------------------
//	Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"

#if __PIMac__
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

#include "3DFormatTerminology.h"	// Terminology for plug-in.

//-------------------------------------------------------------------------------
//	PiPL resource
//-------------------------------------------------------------------------------

resource 'PiPL' (ResourceID, plugInName " PiPL", purgeable)
{
    {
		Kind { ImageFormat },
		Name { plugInName },
		Version { (latestFormatVersion << 16) | latestFormatSubVersion },

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

		// ClassID, eventID, aete ID, uniqueString:
		HasTerminology { plugInClassID, 
		                 plugInEventID, 
						 ResourceID, 
						 vendorName " " plugInName },
		
		SupportedModes {
			doesSupportBitmap,
			doesSupportGrayScale,
			doesSupportIndexedColor,
			doesSupportRGBColor,
			noCMYKColor,
			noHSLColor,
			noHSBColor,
			noMultichannel,
			noDuotone,
		 	noLABColor
		},

		
		// Tells Photoshop this is a 3D format.	
		EnableInfo {
			"in ( PSHOP_Supports3D )"
		},

		FmtFileType {  'p3d ', '8BIM' },
		
		ReadTypes { {'p3d ','8BIM' } },
	    
    	//WriteTypes { {'OBJ ','8BIM' } },
    	
    	ReadExtensions { { 'p3d ' } },
    	
    	//WriteExtensions { { '    ' } },
     	
    	FormatFlags { 
		  	fmtDoesNotSaveImageResources, 
		  	fmtCanRead,
	  		fmtCannotWrite,
    		fmtWritesAll,
			fmtCannotWriteTransparency,
			fmtCanCreateThumbnail
   		},	
   		
		FormatMaxSize {  { 30000, 30000 }  },
    	
		  FormatMaxChannels {
    	{ 1, 1, 1, 4,		/* Maximum # of channels for each plug-in mode */
		  0, 0, 0, 0,
		  0, 0, 0, 0,
		  0, 0, 0, 0 
		}
		},		
		
		FileProperties {}
		}
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
			vendorName " 3DFormat",						/* unique class name */
			plugInClassID,									/* class ID, must be unique or Suite ID */
			plugInAETEComment,								/* optional description */
			{												/* define inheritance */
				"<Inheritance>",							/* must be exactly this */
				keyInherits,								/* must be keyInherits */
				classFormat,								/* parent: Format, Import, Export */
				"parent class format",						/* optional description */
				flagsSingleProperty,						/* if properties, list below */
							
				"foo",
				keyMyFoo,
				typeBoolean,
				"foobar",
				flagsSingleProperty,
				
				"bar",
				keyMyBar,
				typeBoolean,
				"foobar",
				flagsSingleProperty
				/* no properties */
			},
			{}, /* elements (not supported) */
			/* class descriptions */
		},
		{}, /* comparison ops (not supported) */
		{}	/* any enumerations */
	}
};

//-------------------------------------------------------------------------------
//	History resource
//-------------------------------------------------------------------------------

resource StringResource (kHistoryEntry, "History", purgeable)
{
	plugInName ": ref num=^0."
};

//-------------------------------------------------------------------------------

// end 3DFormat.r
