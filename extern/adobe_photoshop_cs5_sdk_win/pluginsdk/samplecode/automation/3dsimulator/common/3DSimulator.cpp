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
//		3DSimulator.cpp
// 
//	Description:
//		Persistent automation plug-in that registers to be notified
//		of events.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "SPTypes.h"
#include "SPAccess.h"
#include "SPInterf.h"
#include "PITerminology.h"
#include "PIStringTerminology.h"
#include "PIUSuites.h"
#include "PITypes.h"
#include "PIActions.h"
//#include "PI3D.h"
#include "PS3DDescriptor.h"

PI3DDescriptorProcs g3DPluginProcs;

//-------------------------------------------------------------------------------
//	Globals -- Define global variables for plug-in scope.
//-------------------------------------------------------------------------------
SPBasicSuite			*sSPBasic = NULL; // this is part of the AutoPluginMain message
SPPluginRef				gPlugInRef = NULL;
static bool notifierOn = false;

//-------------------------------------------------------------------------------
//	Prototypes.
//-------------------------------------------------------------------------------

static void EventSimulator
	(
	/* IN */	DescriptorEventID		event,		// Incoming event.
	/* IN */	PIActionDescriptor		descriptor,	// Event descriptor.
	/* IN */	PIDialogRecordOptions	options,	// Outgoing dialog options.
	/* IN */	void*					data		// Your user data. 
);

// Startup call (used to install notifier):
static SPErr Startup (void);

// Shutdown call (used to remove notifier):
static SPErr Shutdown (void);

//Need to stub this out...
int					PI3DParseFile (PI3DImport *importer) { return 1; }

//-------------------------------------------------------------------------------
//
//	AutoPluginMain
//
//	All calls to the plug-in module come through this routine.
//	It must be placed first in the resource.  To achieve this,
//	most development systems require this be the first routine
//	in the source.
//
//	The entrypoint will be "pascal void" for Macintosh,
//	"void" for Windows.
//
//-------------------------------------------------------------------------------
DLLExport SPAPI SPErr AutoPluginMain(
	const char* caller,	// who is calling
	const char* selector, // what do they want
	void* message	// what is the message
)
{
	SPErr error = kSPNoError;

	try 
	{
		//all messages contain a SPMessageData*
		SPMessageData* basicMessage;
		basicMessage = (SPMessageData*) message;
		sSPBasic = basicMessage->basic;
		gPlugInRef = basicMessage->self;

		// check for SP interface callers
		if (sSPBasic->IsEqual(caller, kSPInterfaceCaller))
		{
			// pop an about box
			if (sSPBasic->IsEqual(selector, kSPInterfaceAboutSelector))
			{
				// this plug-in is marked hidden and you should never get to here
				// DoAbout(gPlugInRef, kSimulatorAboutID);
			}
			// time to start our plug in
			if (sSPBasic->IsEqual(selector, kSPInterfaceStartupSelector))
				error = Startup();

			// time to end our plug in
			if (sSPBasic->IsEqual(selector, kSPInterfaceShutdownSelector))
				error = Shutdown();
		}
	}

	catch(...)
	{
		if (error == 0)
			error = kSPBadParameterError;
	}

	return error;
}

//-------------------------------------------------------------------------------

enum U3DToolID
	{
	q3DToolRotateXY=0,
	q3DToolRotateZ,
	q3DToolPanXY,
	q3DToolPanXZ,
	q3DToolWalkYZ,
	q3DToolWalkXZ,
	q3DToolScale,
	q3DToolRotateX,
	q3DToolRotateY,
	q3DToolPanX,
	q3DToolPanY,
	q3DToolPanZ,
	q3DToolScaleX,
	q3DToolScaleY,
	q3DToolScaleZ
	
	};

//-------------------------------------------------------------------------------

#define c3DTransformObject	0
#define c3DTransformCamera	1

RenderState gRenderState;
RenderUpdate renderFunction = NULL;

#if __PIWin__
const int16 kKeypad1	= 0x61;
const int16 kKeypad2	= 0x62;
const int16 kKeypad4	= 0x64;
const int16 kKeypad6	= 0x66;
const int16 kKeypad8	= 0x68;
const int16 kKeypad9	= 0x69;
const int16 kF1Key		= 0x70;
const int16 kF2Key		= 0x71;
const int16 kF3Key		= 0x72;
const int16 kF4Key		= 0x73;
#else
const int16 kSpaceCode	  = 0x31;
const int16 kCommandCode  = 0x37;
const int16 kShiftCode	  = 0x38;
const int16 kCapsLockCode = 0x39;
const int16 kOptionCode   = 0x3A;
const int16 kControlCode  = 0x3B;
const int16 kPeriodCode   = 0x2F;
const int16 kEscapeCode	  = 0x35;
const int16 kDeleteCode	  = 0x33;
const int16 kKeypad0	  = 0x52;
const int16 kKeypad1	  = 0x53;
const int16 kKeypad2	  = 0x54;
const int16 kKeypad3	  = 0x55;
const int16 kKeypad4	  = 0x56;
const int16 kKeypad5	  = 0x57;
const int16 kKeypad6	  = 0x58;
const int16 kKeypad7	  = 0x59;
const int16 kKeypad8	  = 0x5B;
const int16 kKeypad9	  = 0x5C;

const int16 kF1Key	  = 0x7A;
const int16 kF2Key	  = 0x78;
const int16 kF3Key	  = 0x63;
const int16 kF4Key	  = 0x76;
const int16 kF5Key	  = 0x60;
const int16 kF6Key	  = 0x61;
const int16 kF7Key	  = 0x62;
const int16 kF8Key	  = 0x64;
const int16 kF9Key	  = 0x65;
const int16 kF10Key	  = 0x6D;
const int16 kF11Key	  = 0x67;
const int16 kF12Key	  = 0x6F;
#endif

//-------------------------------------------------------------------------------

#if __PIWin__
bool IsKeyDownInMap(void * /* notUsed */, const int16 keyCode)
{		
	short ks = GetAsyncKeyState( keyCode );
	return (0x8000 & ks ? true : false );
}
#else
bool IsKeyDownInMap (KeyMap& map, const short theKey)
{

	UInt8	*asBytes = (UInt8*)(&map[0]);
	int		index = (theKey >> 3);
	UInt8	mask  = (1 << ((theKey) & 7));

	UInt8   value = asBytes[index];
	return ( value & mask ) != 0;

} // IsKeyDownInMap
#endif

//-------------------------------------------------------------------------------
Boolean KeyICareAboutIsDown()
{
#if __PIWin__
		void * map = NULL;
#else
		KeyMap map;
		GetKeys(map);
#endif
	if(IsKeyDownInMap(map,kF1Key) ||
	   IsKeyDownInMap(map,kF2Key) ||
	   IsKeyDownInMap(map,kF3Key) ||
	   IsKeyDownInMap(map,kF4Key))
			return true;

	return false;
}


OSErr MyRenderDeviceIdleFunction(int32 updateType, Boolean *doInteraction, PIActionDescriptor stateDescriptor, RenderUpdate updateProc)
{
	OSErr err = noErr;
	bool keyEventHappened=KeyICareAboutIsDown();
	if(updateType == k3DRenderAppUpdateTypeCheckInteraction && keyEventHappened && doInteraction)
		{
		*doInteraction=true;
		return err;
		}
	else if(updateType == k3DRenderAppUpdateTypeSetState)
		{
		//Unpack stateDescriptor into gRenderState, we're about to get idled
		err = PI3DGetStateFromDescriptor(&g3DPluginProcs, stateDescriptor, NULL, &gRenderState);

		renderFunction = updateProc;
		return err;
		}
	else if(updateType == k3DRenderAppUpdateTypeDoInteraction)
		{
		//This function gets called in a tight loop from Photoshop
		//In it, you need to set whatever needs setting in the 3D state and the tell PS to update
#if __PIWin__
		void * map = NULL;
#else
		KeyMap map;
		GetKeys(map);
#endif
		if(IsKeyDownInMap(map,kF1Key))
			gRenderState.currentObjectPosition.zAngle += 10;
		else if(IsKeyDownInMap(map,kF2Key))
			gRenderState.currentObjectPosition.zAngle -= 10;
		else if(IsKeyDownInMap(map,kF3Key))
			gRenderState.currentObjectPosition.xAngle += 10;
		else if(IsKeyDownInMap(map,kF4Key))
			gRenderState.currentObjectPosition.xAngle -= 10;
			*doInteraction=false;
		
		PIActionDescriptor stateDesc=NULL;
		g3DPluginProcs.actionDescriptorProcs->Make(&stateDesc);
		if(stateDesc)
			{
			PI3DMakeStateDescriptor(&g3DPluginProcs,&gRenderState, NULL, stateDesc);
			err = (renderFunction)(k3DRenderPluginUpdateTypeUpdateState, stateDesc, NULL);
			g3DPluginProcs.actionDescriptorProcs->Free(stateDesc);
			}
		}
		
	return err;
}
	
static void EventSimulator
	(
	/* IN */	DescriptorEventID		 event ,		// Incoming event.
	/* IN */	PIActionDescriptor		 descriptor ,	// Event descriptor.
	/* IN */	PIDialogRecordOptions	 /*options*/,		// Outgoing dialog options.
	/* IN */	void*					/*data*/			// Your user data. 
)
{
	
	DescriptorTypeID typeID = 0;
	Boolean hasKey = false;
	DescriptorEnumID startStopID = 0;
	DescriptorEnumID stopID = 0;
	DescriptorEnumID startID = 0;
	DescriptorEnumID idleID = 0;
	bool using3DTool=false;
	sPSActionDescriptor->HasKey(descriptor, keyUsing, &hasKey);
	if (hasKey)
	{
		sPSActionDescriptor->GetClass(descriptor, keyUsing, &typeID);
		DescriptorClassID classID = 0;
		sPSActionControl->StringIDToTypeID(ktool3DStr, &classID);
		if (typeID == classID)
		{
			using3DTool = true;
		}
	}
	
	sPSActionDescriptor->HasKey(descriptor, keyWhat, &hasKey);
	if (hasKey)
	{
		DescriptorEnumTypeID enumTypeID;
		sPSActionDescriptor->GetEnumerated(descriptor, keyWhat, &enumTypeID, &startStopID);
	}

	sPSActionControl->StringIDToTypeID(kstartStr, &startID);
	sPSActionControl->StringIDToTypeID(kstopStr, &stopID);
	sPSActionControl->StringIDToTypeID(kidleStr, &idleID);
	
	if (eventNotify == event && using3DTool && startStopID == startID) 
	{
		//Convert to data structure
		
		g3DPluginProcs.actionDescriptorProcs=NULL;
		g3DPluginProcs.actionListProcs=NULL;
		g3DPluginProcs.actionControlProcs=NULL;
		g3DPluginProcs.zStringProcs=NULL;
		
		int32 error = sSPBasic->AcquireSuite(kPSActionDescriptorSuite,
															kPSActionDescriptorSuiteVersion, 
															(const void**)&g3DPluginProcs.actionDescriptorProcs);    
		if(!error)
			error = sSPBasic->AcquireSuite(kPSActionListSuite,
														  kPSActionListSuiteVersion,
														  (const void**)&g3DPluginProcs.actionListProcs);
		if(!error)
			error = sSPBasic->AcquireSuite(kPSBasicActionControlSuite,
														  kPSBasicActionControlSuitePrevVersion,
														  (const void**)&g3DPluginProcs.actionControlProcs);
		if(!error)
			error = sSPBasic->AcquireSuite(kASZStringSuite,
														  kASZStringSuiteVersion1, 
														  (const void**)&g3DPluginProcs.zStringProcs);
		
		//We don't need this one since we're just dealing with state descriptors
		//descriptorProcs.handleProcs=gFilterRecord->handleProcs;

		renderFunction = NULL;
		sPSActionControl->StringIDToTypeID(krenderFunctionStr, &typeID);
		
		hasKey = false;
		sPSActionDescriptor->HasKey(descriptor, typeID, &hasKey);
		int32 dataLength = 0;
		sPSActionDescriptor->GetDataLength(descriptor, typeID, &dataLength);
		if (hasKey && dataLength == sizeof(void*))
		{
			void *tempFunc=NULL;
			sPSActionDescriptor->GetData(descriptor, typeID, &tempFunc);
			renderFunction = (RenderUpdate)tempFunc;
			PIActionDescriptor dummy;
			renderFunction(k3DRenderPluginUpdateTypeSetIdleFunc,dummy,(void*)MyRenderDeviceIdleFunction);
		}
	}
	
}

//-------------------------------------------------------------------------------
//
//	Startup
//   
//	On the startup call we register our notifier to catch any actions
//	that we're interested in.
//
//	The startup sequence is based on your PiPL.  If you have
//
//		Persistent {},
//
//	Your plug-in will stay resident until Photoshop is quit.  If you have
//
//		Messages { startupRequired, doesNotPurgeCache, shutdownRequired, acceptProperty },
//
//	Your plug-in will receive startup and shutdown messages.  You can also
//	create a combination of the startup/shutdown calls by putting "no" in front
//	of the unnecessary call, such as "noStartupRequired".  purgeCache and
//	acceptProperty are required parameters that are not used in Photoshop 5.0
//	but may be in the future.
//
//	You must have a Persistent property for the Messages property to be
//	respected.
//
//	DO NOT do long processes at Startup.  The user will think something
//	happened during loading and bonk their machine, getting irate.
//
//	The normal calling process for a persistent plug-in with messages:
//		Reload
//		Startup
//		Execute (if user selects you or you are played)
//		Shutdown
//		Unload
//
//	GUARANTEES:
//		If you return from Reload with no error,
//			you are guaranteed an Unload call.
//		If you return from Startup with no error,
//			you are guaranteed a Shutdown call (if you asked for one.)
//
//-------------------------------------------------------------------------------
static SPErr Startup (void)
{
	SPErr error = kSPNoError;
	
	// Lets register our actions notification routine.  NOTE: It is our
	// responsibility to unregister this when we're done with it.  If we
	// don't and this plug-in gets unloaded, its a ticking time bomb --
	// whatever event we've registered for will eventually happen and the
	// host will call this routine which won't be there and it'll all go
	// boom.
	if ( ! notifierOn)
	{
		error = sPSActionControl->AddNotify(gPlugInRef,
											eventNotify, // event you want to listen for
											EventSimulator, // Proc for listening routine.
											NULL); // User data.
										
		if ( ! error )
			notifierOn = true;
	}

	return error;
}

//-------------------------------------------------------------------------------
//
//	Shutdown
//
//	It's our responsibility to unregister any notifiers or memory we allocated
//	at startup.  So we iterate through the Simulator list, both removing
//	any notifiers and cleaning out memory.
//
//-------------------------------------------------------------------------------
static SPErr Shutdown (void)
{
	SPErr error = kSPNoError;
	
	if (notifierOn)
			error = sPSActionControl->RemoveNotify(
				gPlugInRef,
				eventNotify);				// Event we registered.

	// clean up after ourselves
	PIUSuitesRelease();

	return error;
}
// end 3DSimulator.cpp