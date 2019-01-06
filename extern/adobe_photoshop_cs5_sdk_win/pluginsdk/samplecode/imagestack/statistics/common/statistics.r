/*
	File: statistics.r

	Copyright (c) 2006, Adobe Systems Incorporated.
	All rights reserved.

	Rez source file for image stack statistics.


DONE
	Mean
	Summation
	Maximum
	Minimum
	Variance
	Standard Deviation
	Skewness
	Kurtosis
	Range = (max - min)
	Median
	Entropy


*/

/*********************************************************************/

#define IDD_ABOUT       16000
#define IDD_PARAM     	16001

/*********************************************************************/
/* Defines required for include files */

#define plugInName						"Statistics"

/*********************************************************************/

#include "PIDefines.h"

#ifdef __PIMac__
	#include <Carbon.r>
	#include "PIGeneral.r"
	#include "statisticsRes.h"
	#include "PIUtilities.r"
#elif defined(__PIWin__)
	#define Rez
	#include "statisticsRes.h"
	#include "PIGeneral.h"
	#include "PIUtilities.r"
#endif

#include "PIActions.h"

//#if TARGET_MAC_OS
//#include <Carbon.r>
//#include "PIGeneral.r"
// #include "ViewTypes.r"
//#endif

//#if __PIWin__
//#include "PIGeneral.h"
//#endif

//#include "PIActions.h"
// #include "PITerminology.h"

//#include "statisticsRes.h"

/*********************************************************************/

#define ourClassID1						'avrg'
#define ourClassID2						'summ'
#define ourClassID3						'minn'
#define ourClassID4						'maxx'
#define ourClassID5						'medn'
#define ourClassID6						'vari'
#define ourClassID7						'stdv'
#define ourClassID8						'skew'
#define ourClassID9						'kurt'
#define ourClassID10					'rang'
#define ourClassID11					'entr'
#define ourEventID						typeNull // must be this

#if TARGET_MAC_OS
#include "VersionStrings.h"
#endif
// #include "PIVersion.r"

/*********************************************************************/

resource 'PiPL' (ResourceID, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Mean" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Mean=Mean" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "MEAN_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "MEAN_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "MEAN_ENTRY" },
			#else
				CodeWin32X86 { "MEAN_ENTRY" },
			#endif
		#endif
		
   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID1, ourEventID, ResourceID, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+1, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Summation" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Summation=Summation" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "SUM_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "SUM_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "SUM_ENTRY" },
			#else
				CodeWin32X86 { "SUM_ENTRY" },
			#endif
		#endif
		
   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID2, ourEventID, ResourceID+1, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+2, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Minimum" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Minimum=Minimum" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "MIN_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "MIN_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "MIN_ENTRY" },
			#else
				CodeWin32X86 { "MIN_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID3, ourEventID, ResourceID+2, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+3, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Maximum" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Maximum=Maximum" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "MAX_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "MAX_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "MAX_ENTRY" },
			#else
				CodeWin32X86 { "MAX_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID4, ourEventID, ResourceID+3, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+4, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Median" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Median=Median" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "MEDIAN_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "MEDIAN_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "MEDIAN_ENTRY" },
			#else
				CodeWin32X86 { "MEDIAN_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID5, ourEventID, ResourceID+4, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+5, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Variance" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Variance=Variance" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "VAR_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "VAR_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "VAR_ENTRY" },
			#else
				CodeWin32X86 { "VAR_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID6, ourEventID, ResourceID+5, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+6, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Standard Deviation" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/StandardDeviation=Standard Deviation" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "STDDEV_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "STDDEV_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "STDDEV_ENTRY" },
			#else
				CodeWin32X86 { "STDDEV_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID7, ourEventID, ResourceID+6, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+7, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Skewness" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Skewness=Skewness" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "SKEW_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "SKEW_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "SKEW_ENTRY" },
			#else
				CodeWin32X86 { "SKEW_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID8, ourEventID, ResourceID+7, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+8, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Kurtosis" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Kurtosis=Kurtosis" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "KURT_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "KURT_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "KURT_ENTRY" },
			#else
				CodeWin32X86 { "KURT_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID9, ourEventID, ResourceID+8, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+9, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Range" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Range=Range" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "RANGE_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "RANGE_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "RANGE_ENTRY" },
			#else
				CodeWin32X86 { "RANGE_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID10, ourEventID, ResourceID+9, "" },
		
		StackRenderer { },

		}
	};

/*********************************************************************/

resource 'PiPL' (ResourceID+10, purgeable)
	{
		{
		Kind { Acquire },

		Name { "Entropy" },

		ZStringName { "$$$/AdobePlugin/PIPLInfo/PluginName/Entropy=Entropy" },
		
		Version { (latestAcquireVersion << 16) | latestAcquireSubVersion },
		
		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "ENTROPY_ENTRY" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "ENTROPY_ENTRY" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "ENTROPY_ENTRY" },
			#else
				CodeWin32X86 { "ENTROPY_ENTRY" },
			#endif
		#endif

   		RequiredHost { '8BIM' },
   		
		PlugInMaxSize { 2000000, 2000000 },

		/* class ID, event ID, aete ID, uniqueString */
		HasTerminology { ourClassID11, ourEventID, ResourceID+10, "" },
		
		StackRenderer { },

		}
	};


/*********************************************************************/
	
/* Scripting resource */

resource 'aete' (ResourceID, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Mean=Mean",
			ourClassID1,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};


/*********************************************************************/
	
/* Scripting resource */

resource 'aete' (ResourceID+1, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Summation=Summation",
			ourClassID2,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};


/*********************************************************************/
	
/* Scripting resource */

resource 'aete' (ResourceID+2, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Minimum=Minimum",
			ourClassID3,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};


/*********************************************************************/
	
/* Scripting resource */

resource 'aete' (ResourceID+3, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Maximum=Maximum",
			ourClassID4,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+4, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Median=Median",
			ourClassID5,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+5, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Variance=Variance",
			ourClassID6,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+6, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/StandardDeviation=Standard Deviation",
			ourClassID7,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+7, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Skewness=Skewness",
			ourClassID8,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+8, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Kurtosis=Kurtosis",
			ourClassID9,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+9, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Range=Range",
			ourClassID10,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/

resource 'aete' (ResourceID+10, purgeable)
{
	1, 0, english, roman,
	{
		"$$$/AETE/Common/Photoshop=Photoshop",
		"",
		'PHTO',
		0,
		0,
		{},	/* events */
		{
			"$$$/AdobePlugin/statistics/Entropy=Entropy",
			ourClassID11,
			"",
			{},
			{}, /* elements (not supported) */
		},
		{}, /* comparison ops (not supported) */
		{}	/* enums */
	}
};

/*********************************************************************/