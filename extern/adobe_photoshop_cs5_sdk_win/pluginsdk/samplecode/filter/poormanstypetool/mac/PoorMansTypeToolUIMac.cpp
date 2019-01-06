// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		PoorMansTypeToolUIMac.cpp
//
//	Description:
//		This file contains the source and routines for the
//		Filter module Poor Man's Type Tool, a module that 
//		uses the Channel Ports Suite for pixel munging.
//
//	Use:
//		This is a basic module to exemplify all the typical
//		functions a filter module will do: Read scripting
//		parameters, show a user interface, display a proxy,
//		write changed pixel information, and then write
//		scripting parameters.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "PoorMansTypeTool.h"

//-------------------------------------------------------------------------------
//	Prototypes.
//-------------------------------------------------------------------------------

// None.

//-------------------------------------------------------------------------------
//
//	DoAbout
//
//	Displays the About box.
//
//	Inputs:
//		AboutRecordPtr about	Is actually a pointer to some platform
//								specific data for Windows only.
//		AboutID					#define with ID of String resource to use for text
//
//	Outputs:
//		Displays About box (Dialog).  Press Escape, Enter, or Return to
//		dismiss, or click the mouse.
//
//-------------------------------------------------------------------------------
// Taken from ConditionalHeaders.h
#if !OPAQUE_TOOLBOX_STRUCTS
struct PopupPrivateData {
  MenuRef             mHandle;
  SInt16              mID;
};
typedef struct PopupPrivateData         PopupPrivateData;
typedef PopupPrivateData *              PopupPrivateDataPtr;
typedef PopupPrivateDataPtr *           PopupPrivateDataHandle;
#endif  /* !OPAQUE_TOOLBOX_STRUCTS */

void InitializeMenu(GPtr globals, DialogPtr hDlg, int item);
void ClearPopUp(MenuHandle menu);
static MenuHandle GetMenuHandleFromItemHandle(Handle itemHandle);
static void CToPStrCopy(StringPtr dst, const char* const src);
static void PToCStrCopy(char* dst, const StringPtr src);


void DoAbout (AboutRecordPtr /*about*/)
{
	ShowAbout (AboutID);
} // end DoAbout

#if !__LP64__

/*****************************************************************************/

/* The following routine sets up a dialog for proxy display.  So far,
   just stuffs the FilterRecord pointer into the refcon for later use. */
   
static void SetUpDialogForProxies (DialogPtr dp, GPtr stuff)
	{
	
	SetWRefCon (GetDialogWindow(dp), (long) stuff);
		
	}

/*****************************************************************************/

void InitializeMenu(GPtr globals, DialogPtr hDlg, int item)
{
	Rect r;
	Handle h;
	short type;
	short menuCount = 0;
	Str255 theString;

	GetDialogItem(hDlg, item, &type, &h, &r);
	if (h == NULL) return;

	// clear the pupup menu
	MenuHandle menuHandle = GetMenuHandleFromItemHandle(h);
	ClearPopUp(menuHandle);

	// the layers are read from the current ReadImageDocumentDesc
	// the first one in the list is the target
	// the actual layers are then added
	if (item == kDLayersPopUp)
	{
		if (gDocDesc == NULL) return;
		
		AppendMenu(menuHandle, "\pTarget Layer");
		menuCount++;

		ReadLayerDesc * layerDesc = gDocDesc->layersDescriptor;

		while (layerDesc != NULL)
		{
			CToPStrCopy(theString, layerDesc->name);
			AppendMenu(menuHandle, theString);
			layerDesc = layerDesc->next;
			menuCount++;
		}
	}
	
	// For Photoshop 7 you can now get at the scrap/clipboard document
	// you can't write to it and it probably doesn't have other layers
	// at this time
	else if (item == kDDocPopUp)
	{
		char madeUpName[256];
		int16 counter = 1;
		
		ReadImageDocumentDesc * localDesc = gDocInfo;
		
		while (localDesc != NULL && gStuff->hasImageScrap)
		{
			sprintf(madeUpName, "DocDesc #%d", counter);
			CToPStrCopy(theString, madeUpName);
			AppendMenu(menuHandle, theString);
			localDesc = localDesc->next;
			counter++;
			menuCount++;	
		}
	}

	else
		return; // an unknown menu id
	
	if (menuCount != 0)
		SetControlMaximum((ControlRef)h, menuCount);
}

/* UserItem to draw the data from the output buffer. */

static pascal void ShowOutputBuffer (DialogPtr dp, short item)
	{
	
	PSPixelMap pixels;
	PSPixelMask mask;
	PSPixelOverlay overlay;
	Rect r;
	Rect itemBounds;
	Handle h;
	short itemType;
	short itemHeight, itemWidth;
	short bufferHeight, bufferWidth;
	
	GPtr globals = (GPtr) GetMoveableWRefCon (dp);
	
	GetDialogItem (dp, item, &itemType, &h, &r);
	
	EraseRect(&r);

	itemBounds.left = ((r.right - r.left)
					  - (gProxyRect.right - gProxyRect.left)) 
					  / 2;
	itemBounds.top = ((r.bottom - r.top)
					  - (gProxyRect.bottom - gProxyRect.top))
					  / 2;
	itemBounds.right = ((r.right - r.left)
					  + (gProxyRect.right - gProxyRect.left)) 
					  / 2;
	itemBounds.bottom = ((r.bottom - r.top)
					  + (gProxyRect.bottom - gProxyRect.top))
					  / 2;
	
	
	r = itemBounds;

	/* do the border */
	PenNormal ();
	InsetRect (&r, -kGutter, -kGutter);
	FrameRect (&r);
	InsetRect (&r, kGutter, kGutter); 
	
	itemHeight = r.bottom - r.top;
	itemWidth = r.right - r.left;
	
	bufferHeight = gDocDesc->bounds.bottom - gDocDesc->bounds.top;
	bufferWidth  = gDocDesc->bounds.right  - gDocDesc->bounds.left;

	/* Set up the output map. */
	
	pixels.version       = 2;
	pixels.bounds.top    = gProxyRect.top;
	pixels.bounds.left   = gProxyRect.left;
	pixels.bounds.bottom = gProxyRect.bottom;
	pixels.bounds.right  = gProxyRect.right;
	pixels.imageMode     = gDocDesc->imageMode;
	pixels.rowBytes      = (gProxyRect.right - gProxyRect.left) 
							* gDocDesc->depth 
							/ 8;
	pixels.colBytes		 = 1;
	pixels.planeBytes    = (gProxyRect.bottom - gProxyRect.top) 
		             		* (gProxyRect.right - gProxyRect.left);
	pixels.baseAddr		 = gChannelData;
	
	pixels.mat			= NULL;
	pixels.masks		= NULL;
	pixels.maskPhaseRow = 0;
	pixels.maskPhaseCol = 0;
	
	// new for version 2 of PSPixelMap
	overlay.next = NULL;
	overlay.data = gOverlayData;
	overlay.rowBytes = gProxyRect.right - gProxyRect.left;
	overlay.colBytes = 1;
	overlay.r = 0;
	overlay.g = 200;
	overlay.b = 0;
	overlay.opacity = 50;
	overlay.overlayAlgorithm = kInvertedAlphaOverlay;
	
	pixels.pixelOverlays = &overlay;
	pixels.colorManagementOptions = kViewAsStandardRGB;
	
	if (gMaskData) 
		{
		mask.next = NULL;
		mask.maskData = gMaskData;
		mask.rowBytes = gProxyRect.right - gProxyRect.left;
		mask.colBytes = 1;
		mask.maskDescription = kSimplePSMask;
	
		pixels.masks = &mask;
		}
	
	/* Display the data. */
	
	if (gStuff->displayPixels != NULL)
		(*(gStuff->displayPixels)) (&pixels, &pixels.bounds, itemBounds.top, itemBounds.left, gStuff->platformData);
			
	}

/*****************************************************************************/
UserItemUPP gUserItemShowOutput = NULL;

/*****************************************************************************/

/* Set up an item so that it will display the data in the output portion of
   the filter record. */

static void MakeOutputProxy (DialogPtr dp, short proxyItem)
	{
	
	short itemType;
	Rect r;
	Handle h;

	GetDialogItem (dp, proxyItem, &itemType, &h				  		  , &r);
	gUserItemShowOutput = NewUserItemUPP(ShowOutputBuffer);
	SetDialogItem (dp, proxyItem,  itemType, (Handle) gUserItemShowOutput, &r);
	
	}

/*****************************************************************************/

const int32 popUpItemSize = 8;
int32 popUpItemAsIndex[popUpItemSize] = { 0, 1, 2, 4, 8, 16, 32, 64 };

static void SetSizePopUpMenu(DialogPtr dp,int32 itemAsNumber)
{
	int32 a;
	for (a = 0; a < popUpItemSize; a++)
	{
		if (popUpItemAsIndex[a] == itemAsNumber)
			break;
	}

	if (a > (popUpItemSize-1))
	{
		a = 1;
	}

	SetPopUpMenuValue(dp, kDSize, a);

}

static int32 GetSizePopUpMenu(DialogPtr dp)
{
	int32 x = GetPopUpMenuValue(dp, kDSize);

	if (x > (popUpItemSize-1))
	{
		x = 1;
	}

	return (popUpItemAsIndex[x]);
}


/* Display modal dialog, using proxy routines to preview results. */

Boolean DoUI (GPtr globals)
{
	long x = 0;
	short item, lastitem;
	short itemType;
	short numberErr;
	Handle h;
	DialogPtr dp;
	DialogTHndl dt;
	short lastV = gPointV;
	short lastH = gPointH;
	int32 lastXFactor = gXFactor;
	Boolean lastGaussianBlurData = gGaussianBlurData;
	Boolean lastAllLayerData = gViewAllLayerData;
	int16 lastLayerIndex = gViewLayerIndex;
	
	dt = (DialogTHndl) GetResource ('DLOG', uiID);
	HNoPurge ((Handle) dt);
	
	CenterDialog (dt);

	dp = GetNewDialog (uiID, nil, (WindowPtr) -1);

	GetDialogItem (dp, kDProxyItem, &itemType, &h, &gProxyRect);
	CalcProxyScaleFactor (&gDocDesc->bounds, &gProxyRect, &gScaleFactor);
	UpdateProxyView(globals);
	SetUpDialogForProxies (dp, globals);
	MakeOutputProxy (dp, kDProxyItem);
	
	(void) SetDialogDefaultItem (dp, kDOK);
	(void) SetDialogCancelItem (dp, kDCancel);
	(void) SetDialogTracksCursor (dp, TRUE);
	
	SetSizePopUpMenu(dp, lastXFactor);
	
	StuffNumber (dp, kDVertical, lastV);
	StuffNumber (dp, kDHorizontal, lastH);
	
	InitializeMenu(globals, dp, kDLayersPopUp);
	InitializeMenu(globals, dp, kDDocPopUp);
	
	SetCheckBoxState (dp, kDGaussianBlurData, lastGaussianBlurData);

	SetCheckBoxState(dp, kDAllLayers, lastAllLayerData);

	ShowHideItem(dp, kDWarningText, lastAllLayerData);

	SelectTextItem (dp, kDVertical); 
	
	ShowWindow (GetDialogWindow(dp));

	do
	{
		
		MoveableModalDialog (dp, gStuff->processEvent, nil, &item);

		if (lastitem != item && item != kDCancel)
		{ /* we just left this area.  Check to make sure its within bounds. */
			switch (lastitem)
			{
				case kDVertical:
					numberErr = FetchNumber(dp,
										    kDVertical,
										    gDocDesc->bounds.top,
										    gDocDesc->bounds.bottom,
										    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(dp,
									kDVertical,
									gDocDesc->bounds.top,
									gDocDesc->bounds.bottom,
									&x,
								    AlertID,
								    numberErr);
						item = kDVertical; // stay here
					}
					break;
				case kDHorizontal:
					numberErr = FetchNumber(dp,
										    kDHorizontal,
										    gDocDesc->bounds.left,
										    gDocDesc->bounds.right,
										    &x);
					if (numberErr != noErr)
					{ // shows alert if there's an error
						AlertNumber(dp,
									kDHorizontal,
									gDocDesc->bounds.left,
									gDocDesc->bounds.right,
									&x,
								    AlertID,
								    numberErr);
						item = kDHorizontal; // stay here
					}
			}
		}
		switch (item)
		{
			case kDGaussianBlurData:
				gGaussianBlurData = !gGaussianBlurData;
				SetCheckBoxState(dp, kDGaussianBlurData, gGaussianBlurData);
				UpdateProxyView(globals);
				InvalItem(dp, kDProxyItem);
				break;

			case kDAllLayers:
				gViewAllLayerData = !gViewAllLayerData;
				SetCheckBoxState(dp, kDAllLayers, gViewAllLayerData);
				ShowHideItem(dp, kDWarningText, gViewAllLayerData);
				UpdateProxyView(globals);
				InvalItem(dp, kDProxyItem);
				break;

			case kDLayersPopUp:
				gViewLayerIndex = GetPopUpMenuValue(dp, kDLayersPopUp) - 1;
				if (gViewLayerIndex == 0)
				{
					SetCheckBoxState(dp, kDAllLayers, gViewAllLayerData);
					PIEnableControl(dp, kDAllLayers);
				}
				else
				{
					gViewAllLayerData = false;
					SetCheckBoxState(dp, kDAllLayers, false);
					PIDisableControl(dp, kDAllLayers);
				}
				UpdateProxyView(globals);
				InvalItem(dp, kDProxyItem);
				break;
				
			case kDDocPopUp:
				int16 docIndex = GetPopUpMenuValue(dp, kDDocPopUp) - 1;
				gDocDesc = gDocInfo;
				while (docIndex-- > 0 && gDocDesc != NULL)
					gDocDesc = gDocDesc->next;
				gViewLayerIndex = 0;
				gViewAllLayerData = 0;
				SetCheckBoxState(dp, kDAllLayers, gViewAllLayerData);
				PIEnableControl(dp, kDAllLayers);
				InitializeMenu(globals, dp, kDLayersPopUp);
				ReleaseProxyMemory(globals);
				GetDialogItem (dp, kDProxyItem, &itemType, &h, &gProxyRect);
				CalcProxyScaleFactor (&gDocDesc->bounds, &gProxyRect, &gScaleFactor);
				UpdateProxyView(globals);
				InvalItem(dp, kDProxyItem);
				break;

			case kDVertical:
				// grab the number whether it's right or not
				numberErr = FetchNumber(dp, 
										kDVertical, 
										gDocDesc->bounds.top, 
										gDocDesc->bounds.bottom, 
										&x);
				if (numberErr == noErr)
				{ // no errors getting the number
					if (gPointV != x)
					{ // New number.  Update with it.
						gPointV = x;	 
						UpdateProxyView(globals);
						InvalItem(dp, kDProxyItem);		/* tell Dialog Mgr to update proxy item */
					}
				}
				break;

			case kDHorizontal:
				// grab the number whether it's right or not
				numberErr = FetchNumber(dp, 
										kDHorizontal, 
										gDocDesc->bounds.left, 
										gDocDesc->bounds.right, 
										&x);
				if (numberErr == noErr)
				{ // no errors getting the number
					if (gPointH != x)
					{ // New number.  Update with it.
						gPointH = x;	 
						UpdateProxyView(globals);
						InvalItem(dp, kDProxyItem);		/* tell Dialog Mgr to update proxy item */
					}
				}
				break;

			case kDSize:
				x = GetSizePopUpMenu(dp);
				if (x != gXFactor)
				{
					gXFactor = x;
					// Do Filtering operation
					UpdateProxyView(globals);

					// Invalidate Proxy Item
					UpdateProxyView(globals);
					InvalItem(dp, kDProxyItem);		/* tell Dialog Mgr to update proxy item */
				}
				break;

		}
		lastitem = item;
	}
	while (item != kDOK && item != kDCancel);

	DisposeDialog (dp);
	HPurge ((Handle) dt);

	if (item == kDCancel)
	{
		gPointV = lastV;
		gPointH = lastH;
		gXFactor = lastXFactor;
		gGaussianBlurData = lastGaussianBlurData;
		gViewAllLayerData = lastAllLayerData;
		gViewLayerIndex = lastLayerIndex;
		gResult = userCanceledErr;
	}

	if ( gUserItemShowOutput != NULL )
		DisposeUserItemUPP(gUserItemShowOutput);
	if ( gUserItemOutlineOK != NULL )
		DisposeUserItemUPP(gUserItemOutlineOK);
	if ( gUserItemOutlineGroup != NULL )
		DisposeUserItemUPP(gUserItemOutlineGroup);
	if ( gFilterProc != NULL )
		DisposeModalFilterUPP(gFilterProc);

	return (item == kDOK);
}

//-------------------------------------------------------------------------------
void ClearPopUp(MenuHandle menu)
{
	short menuItems = CountMenuItems(menu);
	
	while (menuItems > 0)
	{
		DeleteMenuItem(menu, menuItems);
		menuItems--;
	}
}

static MenuHandle GetMenuHandleFromItemHandle(Handle itemHandle)
{
	MenuHandle mh = NULL;

	mh = GetControlPopupMenuHandle((ControlRef)itemHandle);
	
	return (mh);		
}

static void CToPStrCopy(StringPtr dst, const char* const src)
{
	dst[0] = strlen(src);
	short index = 0;
	while (index < dst[0])
	{
		dst[index+1] = src[index];
		index++;
	}
}

static void PToCStrCopy(char* dst, const StringPtr src)
{
	short length = src[0];
	short index = 0;
	while (index <= length)
	{
		dst[index] = src[index+1];
		index++;
	}
	dst[index] = 0;
}

//-------------------------------------------------------------------------------

#else // #if !__LP64__

Boolean DoUI (GPtr globals)
{
	return false;
}

//-------------------------------------------------------------------------------

#endif // #if !__LP64__

// end PoorMansTypeToolUIMac.cpp