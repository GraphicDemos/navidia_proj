// ADOBE SYSTEMS INCORPORATED
// Copyright  2007 Adobe Systems Incorporated
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
//		3DSimulator.r
//
//	Description:
//		Resource information for 3DSimulator
//
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//	Definitions -- Required by other resources in this rez file.
//-------------------------------------------------------------------------------

#define vendorName			"AdobeSDK"
#define plugInAETEComment 	"3D Simulator persistent actions plug-in"

#define plugInSuiteID		'sdKG'
#define plugInClassID		plugInSuiteID
#define plugInEventID		'lstN'

#define SimulatorResourceID	16000

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


//-------------------------------------------------------------------------------
//	PiPL resource
//-------------------------------------------------------------------------------

resource 'PiPL' (SimulatorResourceID, plugInName " PiPL", purgeable)
	{
		{
		Kind { Actions },
		Name { plugInName "..." },
		Category { "**Hidden**" },
		Version { (latestActionsPlugInVersion << 16) | latestActionsPlugInSubVersion },

		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "AutoPluginMain" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "AutoPluginMain" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "AutoPluginMain" },
			#else
				CodeWin32X86 { "AutoPluginMain" },
			#endif
		#endif

		// Include "Persistent" if you want your plug-in to stay loaded
		// until Photoshop quits:
		Persistent {},
		
		// Messages {} is only honored if your plug-in is Persistent:
		Messages
			{
			startupRequired,
			doesNotPurgeCache,
			shutdownRequired,
			acceptProperty
			},
		
		}
	};

// end 3DSimulator.r
