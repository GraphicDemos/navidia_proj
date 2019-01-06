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
//		Shape.r
//
//	Description:
//		This file contains the source and functions for the
//		Import module Shape, which returns a path, layer,
//		or selection with an interesting shape.
//
//	Use:
//		This module specifically works the path return
//		functionality of Photoshop.  The Paths are stored
//		by derezing a work path saved in Photoshop then
//		storing them in a "Path" resource, which is a
//		binary data resource of a "Path" as defined in the
//		"Path layout" section of the Photosop File Format.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Definitions -- Required by include files.
//-------------------------------------------------------------------------------

// The About box and resources are created in PIUtilities.r.
// You can easily override them, if you like.

#define plugInName1			"Shape"
#define plugInCopyrightYear	"1996"
#define plugInDescription \
	"An example plug-in selection module for Adobe Photoshop®."

//-------------------------------------------------------------------------------
//	Definitions -- Required by other resources in this rez file.
//-------------------------------------------------------------------------------

// Dictionary (aete) resources:

#define vendorName			"AdobeSDK"
#define plugInAETEComment1 	"nearestbase example color picker plug-in"

#define plugInEventID1		'shpE'
#define ResourceID1			16482
#define DialogID1			16500
//-------------------------------------------------------------------------------
//	Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "ShapeTerminology.h"	// Terminology for this plug-in.

//-------------------------------------------------------------------------------
//	PiPL resource
//-------------------------------------------------------------------------------

resource 'PiPL' (ResourceID1, plugInName1 " PiPL", purgeable)
{
    {
	    Kind { Selection },
	    Name { plugInName1 "..." },
	    Category { vendorName },
	    Version { (latestSelectionVersion << 16) | latestSelectionSubVersion },

		// ClassID, eventID, aete ID, uniqueString:
	    HasTerminology { plugInClassID, plugInEventID1, ResourceID1, vendorName " " plugInName1 },
		/* classID, eventID, aete ID, uniqueString (presence of uniqueString makes it
		   non-AppleScript */

		EnableInfo { "true" },

		#ifdef __PIMac__
			#if (defined(__i386__))
				
				CodeMacIntel32 { "PluginMain1" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "PluginMain1" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "PluginMain1" },
			#else
				CodeWin32X86 { "PluginMain1" },
			#endif
		#endif

	}
};

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (ResourceID1, plugInName1 " dictionary", purgeable)
{
	1, 0, english, roman,					/* aete version and language specifiers */
	{
		vendorName,							/* vendor suite name */
		"Adobe example plug-ins",			/* optional description */
		plugInSuiteID,							/* suite ID */
		1,									/* suite code, must be 1 */
		1,									/* suite level, must be 1 */
		{									/* structure for filters */
			vendorName " " plugInName1,		/* unique selection name */
			plugInAETEComment1,				/* optional description */
			plugInClassID,					/* class ID, must be unique or Suite ID */
			plugInEventID1,					/* event ID, must be unique */

			NO_REPLY,						/* never a reply */
			IMAGE_DIRECT_PARAMETER,			/* direct parameter, used by Photoshop */
			{								/* parameters here, if any */
				"shape",					/* parameter name */
				keyMyShape,					/* parameter key ID */
				typeMyShape,				/* parameter type ID */
				"shape type",				/* optional description */
				flagsEnumeratedParameter,	/* parameter flags */
	
				"create",					/* parameter name */
				keyMyCreate,				/* parameter key ID */
				typeMyCreate,				/* parameter type ID */
				"create type",				/* optional description */
				flagsEnumeratedParameter	/* parameter flags */

			}
		},
		{},	/* non-filter plug-in class here */
		{}, /* comparison ops (not supported) */
		{									/* enumerations */
			typeMyShape,					/* type shape 'tshP' */
			{
				"triangle",					/* first value */
				shapeTriangle,				/* 'shP0' */
				"triangle path",			/* optional description */
				
				"square",					/* second value */
				shapeSquare,				/* 'shP1' */
				"square path",				/* optional description */

				"circle",					/* third value */
				shapeCircle,				/* 'shP2' */
				"circle path",				/* optional description */

				"star",						/* fourth value */
				shapeStar,	 				/* 'shP3' */
				"star path",				/* optional description */
				
				"treble",					/* fifth value */
				shapeTreble,				/* 'shP4' */
				"treble path",				/* optional description */

				"ribbon",					/* sixth value */
				shapeRibbon,				/* 'shP5' */
				"ribbon path",				/* optional description */

				"note",						/* seventh value */
				shapeNote,					/* 'shP6' */
				"note path"					/* optional description */
			},

			typeMyCreate,					/* type shape 'tshP' */
			{
				"selection",				/* first value */
				createSelection,			/* 'crE0' */
				"make selection",			/* optional description */
				
				"path",						/* second value */
				createMaskpath,				/* 'crE1' */
				"make mask path",			/* optional description */

				"layer",					/* third value */
				createLayer,				/* 'crE2' */
				"make layer"				/* optional description */
			}
		}	/* any enumerations */
	}
};

//-------------------------------------------------------------------------------
//	Dialog resource
//-------------------------------------------------------------------------------

resource 'DLOG' (DialogID1, plugInName1 " UI", purgeable)
{
	{259, 337, 443, 581},
	movableDBoxProc,
	visible,
	goAway,
	0x0,
	DialogID1,
	plugInName1,
	centerParentWindowScreen
};

resource 'dlgx' (DialogID1) {
	versionZero {
		kDialogFlagsHandleMovableModal + kDialogFlagsUseThemeControls + kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (DialogID1, plugInName1 " UI", purgeable)
{
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{8, 164, 28, 232},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{36, 166, 56, 230},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{4, 11, 32, 57},
		StaticText {
			disabled,
			"Shape:"
		},
		/* [4] */
		{4, 67, 20, 159},
		RadioButton {
			enabled,
			"Triangle"
		},
		/* [5] */
		{20, 67, 36, 159},
		RadioButton {
			enabled,
			"Square"
		},
		/* [6] */
		{36, 67, 52, 159},
		RadioButton {
			enabled,
			"Circle"
		},
		/* [7] */
		{52, 67, 68, 159},
		RadioButton {
			enabled,
			"Star"
		},
		/* [8] */
		{68, 67, 84, 159},
		RadioButton {
			enabled,
			"Treble"
		},
		/* [9] */
		{84, 67, 100, 159},
		RadioButton {
			enabled,
			"Ribbon"
		},
		/* [10] */
		{100, 67, 116, 159},
		RadioButton {
			enabled,
			"Note"
		},
		/* [11] */
		{128, 7, 156, 57},
		StaticText {
			disabled,
			"Create:"
		},
		/* [12] */
		{128, 67, 144, 159},
		RadioButton {
			enabled,
			"Selection"
		},
		/* [13] */
		{144, 67, 160, 159},
		RadioButton {
			enabled,
			"Path"
		},
		/* [14] */
		{160, 67, 176, 159},
		RadioButton {
			enabled,
			"Layer"
		}
	}
};

//-------------------------------------------------------------------------------
//	Path resources.
//-------------------------------------------------------------------------------

data PathResource (ResourceID, "Triangle") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /*................ */
	$"0000 0000 0000 0000 0000 0000 0003 0000"            /*................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /*................ */
	$"0000 0000 0001 0086 8000 0016 8000 0086"            /*.......ÜÄ...Ä..Ü */
	$"8000 0016 8000 0086 8000 0016 8000 0001"            /*Ä...Ä..ÜÄ...Ä... */
	$"0015 8000 0088 0000 0015 8000 0088 0000"            /*..Ä..à....Ä..à.. */
	$"0015 8000 0088 0000 0001 00C2 8000 00D0"            /*..Ä..à.....¬Ä..- */
	$"0000 00C2 8000 00D0 0000 00C2 8000 00D0"            /*...¬Ä..-...¬Ä..- */
	$"0000"                                               /* .. */
};

data PathResource (ResourceID+1, "Square") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0004 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0001 002A AAAB 002A 684C 002A"            /* .......*™´.*hL.* */
	$"AAAB 002A 684C 002A AAAB 002A 684C 0001"            /* ™´.*hL.*™´.*hL.. */
	$"00D6 2AAB 002A 684C 00D6 2AAB 002A 684C"            /* .÷*´.*hL.÷*´.*hL */
	$"00D6 2AAB 002A 684C 0001 00D6 2AAB 00D5"            /* .÷*´.*hL...÷*´.’ */
	$"2AAB 00D6 2AAB 00D5 2AAB 00D6 2AAB 00D5"            /* *´.÷*´.’*´.÷*´.’ */
	$"2AAB 0001 002A AAAB 00D5 2AAB 002A AAAB"            /* *´...*™´.’*´.*™´ */
	$"00D5 2AAB 002A AAAB 00D5 2AAB"                      /* .’*´.*™´.’*´ */
};

data PathResource (ResourceID+2, "Circle") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0002 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0001 001F A5ED 00FB 2AAB 001F"            /* ........•Ì.˚*´.. */
	$"A5ED 0080 425F 001F A5ED 0005 8000 0001"            /* •Ì.ÄB_..•Ì..Ä... */
	$"00E0 0000 0005 0000 00E0 0000 0080 425F"            /* .‡.......‡...ÄB_ */
	$"00E0 0000 00FA 12F7"                                /* .‡...˙.˜ */
};

data PathResource (ResourceID+3, "Star") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0003 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0001 002C F915 0077 A6F5 002C"            /* .......,˘..w¶ı., */
	$"F915 0077 A6F5 002C F915 0077 A6F5 0001"            /* ˘..w¶ı.,˘..w¶ı.. */
	$"009B ACF9 0032 1643 009B ACF9 0032 1643"            /* .õ¨˘.2.C.õ¨˘.2.C */
	$"009B ACF9 0032 1643 0001 009F 2298 00B6"            /* .õ¨˘.2.C...ü"ò.∂ */
	$"42C8 009F 2298 00B6 42C8 009F 2298 00B6"            /* B».ü"ò.∂B».ü"ò.∂ */
	$"42C8 0000 0003 0000 0000 0000 0000 0000"            /* B».............. */
	$"0000 0000 0000 0000 0000 0000 0001 0046"            /* ...............F */
	$"EB3E 002F 4DEA 0046 EB3E 002F 4DEA 0046"            /* Î>./MÍ.FÎ>./MÍ.F */
	$"EB3E 002F 4DEA 0001 0048 A60E 00B9 0B21"            /* Î>./MÍ...H¶..π.! */
	$"0048 A60E 00B9 0B21 0048 A60E 00B9 0B21"            /* .H¶..π.!.H¶..π.! */
	$"0001 00C5 306F 0073 7A6F 00C5 306F 0073"            /* ...≈0o.szo.≈0o.s */
	$"7A6F 00C5 306F 0073 7A6F"                           /* zo.≈0o.szo */
};


data PathResource (ResourceID+4, "Treble") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0002 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0002 0059 D27D 0090 6C17 0057"            /* .......Y“}.êl..W */
	$"7777 008B 60B6 005B 5B06 008C 2222 0001"            /* ww.ã`∂.[[..å"".. */
	$"006A 05B0 008C EEEF 0062 2222 0091 1111"            /* .j.∞.åÓÔ.b"".ë.. */
	$"005B 49F5 0094 9F4A 0000 000A 0000 0000"            /* .[Iı.îüJ........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0002 004E 3E94 0087 6666 004C CCCD"            /* .....N>î.áff.LÃÕ */
	$"0087 1C72 004E 8889 0085 4444 0001 0051"            /* .á.r.Nàâ.ÖDD...Q */
	$"2222 0082 16C1 0053 3333 0080 B60B 0059"            /* "".Ç.¡.S33.Ä∂..Y */
	$"F49F 007C 1C72 0001 0064 EEEF 007E 3E94"            /* Ùü.|.r...dÓÔ.~>î */
	$"0066 6666 0086 6666 0066 CCCD 0088 BBBC"            /* .fff.Üff.fÃÕ.àªº */
	$"0002 0065 C71C 008B 7D28 0065 B05B 008C"            /* ...e«..ã}(.e∞[.å */
	$"CCCD 0063 49F5 008C 4444 0001 0060 DDDE"            /* ÃÕ.cIı.åDD...`›ﬁ */
	$"008B EEEF 005D DDDE 008B 60B6 0054 1111"            /* .ãÓÔ.]›ﬁ.ã`∂.T.. */
	$"0089 8E39 0002 0056 EEEF 0087 38E4 005F"            /* .âé9...VÓÔ.á8‰._ */
	$"49F5 0085 B05B 0060 49F5 0086 DDDE 0002"            /* Iı.Ö∞[.`Iı.Ü›ﬁ.. */
	$"0061 D27D 0088 0B61 0062 D82E 0089 3E94"            /* .a“}.à.a.bÿ..â>î */
	$"0062 60B6 0088 8889 0001 0063 2222 0087"            /* .b`∂.ààâ...c"".á */
	$"DDDE 0062 D82E 0087 1C72 0061 1C72 0082"            /* ›ﬁ.bÿ..á.r.a.r.Ç */
	$"AAAB 0001 0058 FA50 0082 2D83 0055 5555"            /* ™´...X˙P.Ç-É.UUU */
	$"0084 FA50 0054 AAAB 0085 7777 0001 0053"            /* .Ñ˙P.T™´.Öww...S */
	$"DDDE 0088 4FA5 0053 3333 0088 8889 0050"            /* ›ﬁ.àO•.S33.ààâ.P */
	$"4444 0089 7777 0000 0006 0000 0000 0000"            /* DD.âww.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0002 0035 D82E 008B 3E94 0035 5555 008A"            /* ...5ÿ..ã>î.5UU.ä */
	$"AAAB 0036 C71C 0089 9F4A 0001 0039 05B0"            /* ™´.6«..âüJ...9.∞ */
	$"0087 7D28 003B 05B0 0087 1C72 003C C16C"            /* .á}(.;.∞.á.r.<¡l */
	$"0086 C16C 0002 003E E93F 0087 1C72 0040"            /* .Ü¡l...>È?.á.r.@ */
	$"B60B 0087 1C72 0040 D27D 0087 5B06 0002"            /* ∂..á.r.@“}.á[... */
	$"0043 E93F 0087 1C72 0044 4444 0087 1C72"            /* .CÈ?.á.r.DDD.á.r */
	$"0043 F49F 0087 5555 0001 0043 D82E 0088"            /* .CÙü.áUU...Cÿ..à */
	$"EEEF 0043 8E39 0089 3E94 0042 60B6 008A"            /* ÓÔ.Cé9.â>î.B`∂.ä */
	$"71C7 0001 0038 4FA5 0090 16C1 0036 0B61"            /* q«...8O•.ê.¡.6.a */
	$"008D 82D8 0035 93E9 008C FA50 0000 001A"            /* .çÇÿ.5ìÈ.å˙P.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0002 0033 49F5 0087 16C1"            /* .........3Iı.á.¡ */
	$"002E EEEF 008A AAAB 002E EEEF 008A AAAB"            /* ..ÓÔ.ä™´..ÓÔ.ä™´ */
	$"0001 0030 9F4A 008D 4444 0031 1111 008D"            /* ...0üJ.çDD.1...ç */
	$"82D8 0037 1111 0090 E93F 0001 0041 C16C"            /* Çÿ.7...êÈ?...A¡l */
	$"008F DDDE 0047 1C72 008C CCCD 0048 16C1"            /* .è›ﬁ.G.r.åÃÕ.H.¡ */
	$"008C 38E4 0001 004A 7777 0088 B05B 004B"            /* .å8‰...Jww.à∞[.K */
	$"60B6 0088 8889 004C 93E9 0088 49F5 0002"            /* `∂.ààâ.LìÈ.àIı.. */
	$"0050 4444 0089 9F4A 0052 7D28 0089 F49F"            /* .PDD.âüJ.R}(.âÙü */
	$"0052 D82E 008C A4FA 0001 0052 38E4 008E"            /* .Rÿ..å§˙...R8‰.é */
	$"38E4 0053 3333 0090 5B06 0055 C71C 0096"            /* 8‰.S33.ê[..U«..ñ */
	$"0000 0001 005F 0000 0096 05B0 0063 8E39"            /* ....._...ñ.∞.cé9 */
	$"0092 7D28 0065 1C72 0091 4444 0002 0065"            /* .í}(.e.r.ëDD...e */
	$"49F5 008F F49F 0066 6666 008E 38E4 006B"            /* Iı.èÙü.fff.é8‰.k */
	$"7777 008F 5555 0001 0075 E93F 0092 9F4A"            /* ww.èUU...uÈ?.íüJ */
	$"0078 2D83 008C CCCD 0078 7777 008C 05B0"            /* .x-É.åÃÕ.xww.å.∞ */
	$"0002 0077 B60B 008B 5B06 0078 2D83 008A"            /* ...w∂..ã[..x-É.ä */
	$"AAAB 0078 2D83 008A AAAB 0001 0079 1C72"            /* ™´.x-É.ä™´...y.r */
	$"0086 49F5 0078 2D83 0084 4444 0075 A4FA"            /* .ÜIı.x-É.ÑDD.u§˙ */
	$"007E D27D 0002 0070 C16C 0081 3E94 006D"            /* .~“}...p¡l.Å>î.m */
	$"82D8 0083 8E39 006D C16C 0084 9F4A 0002"            /* Çÿ.Éé9.m¡l.ÑüJ.. */
	$"006D 82D8 0086 05B0 006D 82D8 0087 1C72"            /* .mÇÿ.Ü.∞.mÇÿ.á.r */
	$"006D A4FA 0087 2D83 0002 006E EEEF 0087"            /* .m§˙.á-É...nÓÔ.á */
	$"D27D 006E EEEF 0087 D27D 006E EEEF 0087"            /* “}.nÓÔ.á“}.nÓÔ.á */
	$"D27D 0002 006F 8E39 0089 16C1 006F A4FA"            /* “}...oé9.â.¡.o§˙ */
	$"0089 3E94 0070 6666 0089 27D2 0002 0073"            /* .â>î.pff.â'“...s */
	$"9F4A 0088 93E9 0073 3333 0088 8889 0073"            /* üJ.àìÈ.s33.ààâ.s */
	$"3333 0088 8889 0002 0075 5555 0087 D27D"            /* 33.ààâ...uUU.á“} */
	$"0075 5555 0087 D27D 0075 5555 0087 93E9"            /* .uUU.á“}.uUU.áìÈ */
	$"0002 0075 2D83 0085 D82E 0075 5555 0085"            /* ...u-É.Öÿ..uUU.Ö */
	$"B05B 0075 C71C 0085 D82E 0002 0077 7777"            /* ∞[.u«..Öÿ....www */
	$"0086 6666 0077 7777 0086 6666 0077 3333"            /* .Üff.www.Üff.w33 */
	$"0087 C16C 0001 0077 D27D 0089 4FA5 0077"            /* .á¡l...w“}.âO•.w */
	$"7777 008A AAAB 0075 DDDE 0090 82D8 0002"            /* ww.ä™´.u›ﬁ.êÇÿ.. */
	$"006D F49F 008E 4FA5 0068 8889 008D 82D8"            /* .mÙü.éO•.hàâ.çÇÿ */
	$"0068 7777 008C 1111 0002 0068 2222 008D"            /* .hww.å.....h"".ç */
	$"1111 0067 1C72 008C CCCD 0067 4FA5 008A"            /* ...g.r.åÃÕ.gO•.ä */
	$"1111 0001 0068 4FA5 0086 A4FA 0067 1C72"            /* .....hO•.Ü§˙.g.r */
	$"0083 8E39 0064 9F4A 007D 38E4 0001 0059"            /* .Éé9.düJ.}8‰...Y */
	$"8889 0078 71C7 0051 1111 007C 71C7 004E"            /* àâ.xq«.Q...|q«.N */
	$"8E39 007D 999A 0001 0046 F49F 0086 27D2"            /* é9.}ôö...FÙü.Ü'“ */
	$"0045 B05B 0086 6666 0045 27D2 0086 7D28"            /* .E∞[.Üff.E'“.Ü}( */
	$"0001 0041 C71C 0085 05B0 0041 6C17 0084"            /* ...A«..Ö.∞.Al..Ñ */
	$"FA50 0038 2222 0083 B05B"                           /* ˙P.8"".É∞[ */
};

data PathResource (ResourceID+5, "Ribbon") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 000B 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0001 005C 3333 0066 B05B 005C"            /* .......\33.f∞[.\ */
	$"71C7 0067 1C72 0060 27D2 006D 8889 0002"            /* q«.g.r.`'“.màâ.. */
	$"0065 C16C 0068 49F5 0069 F49F 0065 B05B"            /* .e¡l.hIı.iÙü.e∞[ */
	$"006B 8889 0066 E38E 0001 006D AAAB 006A"            /* .kàâ.f„é...m™´.j */
	$"2222 006F A4FA 006A AAAB 0070 71C7 006A"            /* "".o§˙.j™´.pq«.j */
	$"DDDE 0002 0071 B05B 006A 5B06 0072 7D28"            /* ›ﬁ...q∞[.j[..r}( */
	$"006A AAAB 0073 B05B 0069 60B6 0001 0075"            /* .j™´.s∞[.i`∂...u */
	$"A4FA 0066 1C72 0077 7777 0065 B05B 0078"            /* §˙.f.r.www.e∞[.x */
	$"0B61 0065 8889 0001 007C 9F4A 0068 0000"            /* .a.eàâ...|üJ.h.. */
	$"007B 05B0 0065 B05B 0077 2D83 0060 27D2"            /* .{.∞.e∞[.w-É.`'“ */
	$"0002 0072 6C17 0064 DDDE 006F A4FA 0067"            /* ...rl..d›ﬁ.o§˙.g */
	$"D27D 006E 1111 0067 16C1 0001 006C CCCD"            /* “}.n...g.¡...lÃÕ */
	$"0067 4FA5 006B 60B6 0066 6666 0069 0000"            /* .gO•.k`∂.fff.i.. */
	$"0064 D82E 0001 0068 60B6 0061 A4FA 0064"            /* .dÿ....h`∂.a§˙.d */
	$"4444 0063 8E39 0063 60B6 0063 F49F 0001"            /* DD.cé9.c`∂.cÙü.. */
	$"0062 4FA5 0066 C71C 0061 6C17 0067 1C72"            /* .bO•.f«..al..g.r */
	$"0061 6C17 0067 1C72 0002 0060 0000 0067"            /* .al..g.r...`...g */
	$"1C72 0060 0000 0067 1C72 0060 0B61 0067"            /* .r.`...g.r.`.a.g */
	$"1C72"                                               /* .r */
};

data PathResource (ResourceID+6, "Note") {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0002 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0002 0083 5B06 006A B05B 007E"            /* .......É[..j∞[.~ */
	$"93E9 0062 D82E 0088 71C7 0065 BBBC 0002"            /* ìÈ.bÿ..àq«.eªº.. */
	$"0089 6666 006A 60B6 0090 5B06 006F A4FA"            /* .âff.j`∂.ê[..o§˙ */
	$"0084 BBBC 006E 1111 0000 000B 0000 0000"            /* .Ñªº.n.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0002 0080 E93F 0061 999A 0073 E93F"            /* .....ÄÈ?.aôö.sÈ? */
	$"0062 2222 007B 9F4A 0064 BBBC 0001 007D"            /* .b"".{üJ.dªº...} */
	$"FA50 006B A4FA 0084 4444 006E EEEF 0089"            /* ˙P.k§˙.ÑDD.nÓÔ.â */
	$"8E39 0071 B05B 0001 008D 1111 006F 82D8"            /* é9.q∞[...ç...oÇÿ */
	$"0093 3333 0070 5B06 0093 3333 0070 5B06"            /* .ì33.p[..ì33.p[. */
	$"0001 0096 0B61 0071 C71C 0096 0B61 0071"            /* ...ñ.a.q«..ñ.a.q */
	$"C71C 0099 FA50 0072 2D83 0001 009E EEEF"            /* «..ô˙P.r-É...ûÓÔ */
	$"0071 38E4 00A1 6C17 006F A4FA 00A1 9F4A"            /* .q8‰.°l..o§˙.°üJ */
	$"006F 82D8 0002 00A1 F49F 006E 49F5 00A2"            /* .oÇÿ...°Ùü.nIı.¢ */
	$"2222 006E 38E4 0092 7D28 0072 FA50 0002"            /* "".n8‰.í}(.r˙P.. */
	$"0090 EEEF 006D 2222 008A AAAB 0062 D82E"            /* .êÓÔ.m"".ä™´.bÿ. */
	$"0091 F49F 0062 D82E 0001 009E D27D 0064"            /* .ëÙü.bÿ....û“}.d */
	$"9F4A 00A5 B05B 0062 D82E 00AC 0B61 0061"            /* üJ.•∞[.bÿ..¨.a.a */
	$"27D2 0001 00B1 60B6 004F 5555 00A5 B05B"            /* '“...±`∂.OUU.•∞[ */
	$"0051 C71C 00A4 F49F 0051 E93F 0001 00A4"            /* .Q«..§Ùü.QÈ?...§ */
	$"0B61 0053 7777 00A3 8E39 0053 E93F 009E"            /* .a.Sww.£é9.SÈ?.û */
	$"C16C 0058 05B0 0002 009E 6C17 0059 AAAB"            /* ¡l.X.∞...ûl..Y™´ */
	$"009F 49F5 0061 6C17 0091 1111 0061 6C17"            /* .üIı.al..ë...al. */
};

//-------------------------------------------------------------------------------

// end Shape.r
