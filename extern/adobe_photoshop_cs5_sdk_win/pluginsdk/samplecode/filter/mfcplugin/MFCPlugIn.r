// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
#include "PIGeneral.h"

resource 'PiPL' (16000, "Tommy PiPL", purgeable)
{
	{
		Kind { Filter },
		Name { "MFC PlugIn..." },
		Category { "AdobeSDK" },
		Version { (latestFilterVersion << 16) | latestFilterSubVersion },

		#if defined(_WIN64)
			CodeWin64X86 { "PluginMain" },
		#else
			CodeWin32X86 { "PluginMain" },
		#endif

		EnableInfo { "true" },

	}
};

