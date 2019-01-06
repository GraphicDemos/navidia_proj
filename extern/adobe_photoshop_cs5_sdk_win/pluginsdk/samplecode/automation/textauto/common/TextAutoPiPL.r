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
//		TextAutoPiPL.r
//
//	Description:
//		An automation plug in that uses a hidden filter plug in to
//		do something very boring and set all the pixels to 0.
//
//-------------------------------------------------------------------------------
#include "PIDefines.h"

#define plugInName "TextAuto"
#define plugInCopyrightYear	"2000"
#define plugInDescription \
	"An Actions Module that gets called by a format plug-in for Adobe Photoshop¨."

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

resource 'PiPL' ( 16000, "TextAuto", purgeable)
	{
		{
		Kind { Actions },
		Name { plugInName"..." },
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

		EnableInfo { "true" },

		Persistent {},
		
		Messages
			{
			startupRequired,
			doesNotPurgeCache,
			shutdownRequired,
			acceptProperty
			},
		
		
		HasTerminology
			{ 
			'TxAu', 
			'TxAu', 
			16000, 
			"9E5C2DAF-7973-44db-A2A4-F4922E0039B2"
			},
		}
	};

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (16000, "TextAuto dictionary", purgeable)
	{
	1, 0, english, roman,					/* aete version and language specifiers */
		{
		"Testing",							/* vendor suite name */
		"Adobe example plug-ins",			/* optional description */
		'TxAu',								/* suite ID */
		1,									/* suite code, must be 1 */
		1,									/* suite level, must be 1 */
			{								/* structure for automation */
			"Text Auto",						/* name */
			"No comment",					/* optional description */
			'TxAu',							/* class ID, must be unique or Suite ID */
			'TxAu',							/* event ID, must be unique */

			NO_REPLY,						/* never a reply */
			IMAGE_DIRECT_PARAMETER,			/* direct parameter, used by Photoshop */
				{							// filter or selection class here:
				}
			},
			{},	/* non-filter/automation plug-in class here */
			{}, /* comparison ops (not supported) */
			{ // Enumerations go here:
			}	/* end of any enumerations */
		}
	};
// end TextAutoPiPL.r
