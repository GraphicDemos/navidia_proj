// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2003 Adobe Systems Incorporated
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
//		Propetizer.cpp
//
//	Description:
//		This file contains the routines and source
//		for the Filter module Propetizer, a module that
//		examines and displays Photoshop properties.
//
//	Use:
//		This is a basic module to exemplify all the typical
//		functions a filter module will do with a special
//		emphasis on assessing properties.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------


#include <vector>
#include <fstream>
#include "Propetizer.h"
#include "PropertyUtils.h"

void FileToHandle(const string & fileName, Handle & h);
void HandleToFile(const string & fileName, const Handle h);
void FileToString(const string & fileName, string & inString);
void StringToFile(const string & fileName, const string & outString);

void DoStart(void);


FilterRecordPtr gFilterRecord = NULL;
SPBasicSuite * sSPBasic = NULL;
SPPluginRef gPluginRef = NULL;

DLLExport MACPASCAL void PluginMain(const int16 selector,
								    FilterRecordPtr filterRecord,
								    intptr_t * data,
								    int16 * result);

//-------------------------------------------------------------------------------
//
//	PluginMain
//	
//	All calls to the plug in module come through this routine.
//
//	Inputs:
//		const int16 selector		Host provides selector indicating what
//									command to do.
//
//	Inputs and Outputs:
//		FilterRecord *filterRecord	Host provides a pointer to parameter block
//									containing pertinent data and callbacks.
//									See PIFilter.h
//
//		void *data					Use this to store a handle or pointer to our global
//									data structure, which is maintained by the
//									host between calls to the plug in.
//
//	Outputs:
//		int16 *result				Returns error result. Some errors are handled
//									by the host, some are silent, and some you
//									must handle. See PIGeneral.h.
//
//-------------------------------------------------------------------------------
DLLExport MACPASCAL void PluginMain(const int16 selector,
								    FilterRecordPtr filterRecord,
								    intptr_t * data,
								    int16 * result)
{
		// update our global parameters
	gFilterRecord = filterRecord;
	if (selector == filterSelectorAbout)
	{
		sSPBasic = ((AboutRecord*)gFilterRecord)->sSPBasic;
	}
	else
	{
		sSPBasic = gFilterRecord->sSPBasic;

		if (gFilterRecord->bigDocumentData != NULL)
			gFilterRecord->bigDocumentData->PluginUsing32BitCoordinates = true;
	}

	// do the command according to the selector
	switch (selector)
	{
		case filterSelectorAbout:
			DoAbout();
			break;
		case filterSelectorStart:
			DoStart();
			break;
		default:
			break;
	}
}

void DoStart(void)
{
	PropetizerData propetizerData;
	propetizerData.Start();
}

PropetizerData::PropetizerData()
	: fBigNudgeV(10.0), fBigNudgeH(10.0), fRulerOriginV(0.0), 
	  fRulerOriginH(0.0), fGridMajor(0.0), fGridMinor(0.0), 
	  fWatchSuspension(false), fCopyright(false), fURL(""), fXMP(""),
	  fWatermark(false), fSliceID(0), fSliceIndex(0), fBits(0)
{
}



//-------------------------------------------------------------------------------
//
//	PropetizerData::~PropetizerData
//
//	Get rid of anything we made during this plug in life cycle.
//
//-------------------------------------------------------------------------------
PropetizerData::~PropetizerData()
{
}



//-------------------------------------------------------------------------------
//
//	PropetizerData::DoStart
//
//	This is where all the action is.
//
//-------------------------------------------------------------------------------
void PropetizerData::Start(void)
{
	int16 error = 0;

	GetProperties();

	bool playInProgress;
	bool popDialog;

	if (!PIGetPlayInProgress(playInProgress))
	{
		if (playInProgress)
		{
			popDialog = ReadScriptParameters();
		}
		else
		{
			popDialog = true;
		}
	}
	else
	{
		popDialog = true;
	}
	
	if (popDialog)
	{
		if (DoUI() == 1) // the OK button
		{
			error = SetProperties();
		}
	}
	else 
	{
		error = SetProperties();
	}

	if (!error)
	{
		WriteScriptParameters();
	}
}



int16 PropetizerData::GetProperties(void)
{
	(void) PIGetBigNudge(fBigNudgeH, fBigNudgeV);
	(void) PIGetRulerOrigin(fRulerOriginH, fRulerOriginV);
	(void) PIGetGrid(fGridMajor, fGridMinor);
	intptr_t intPtr = 0;
	(void) PIGetWatchSuspension(intPtr);
	fWatchSuspension = (int32)intPtr;
	(void) PIGetCopyright(fCopyright);
	(void) PIGetURL(fURL);
	(void) PIGetWatermark(fWatermark);
	(void) PIGetSelectedSliceID(intPtr);
	fSliceID = (int32)intPtr;
	(void) PIGetSelectedSliceIndex(intPtr);
	fSliceIndex = (int32)intPtr;
	(void) PIGetXMP(fXMP);

	return 0;
}

int16 PropetizerData::SetProperties(void)
{
	double d1, d2;
	intptr_t i;
	bool b;
	string s;
	int16 error = 0;

	if (!PIGetBigNudge(d1, d2))
	{
		if (d1 != fBigNudgeH && d2 != fBigNudgeV)
		{
			if (!error)
				error = PISetBigNudge(fBigNudgeH, fBigNudgeV);
			if (!error)
			{
				fBits.set(iNudgeH) = true;
				fBits.set(iNudgeV) = true;
			}
		}
		else if (d1 != fBigNudgeH)
		{
			if (!error)
				error = PISetBigNudge(fBigNudgeH, d2);
			if (!error)
				fBits.set(iNudgeH) = true;
		}
		else if (d2 != fBigNudgeV)
		{
			if (!error)
				error = PISetBigNudge(d1, fBigNudgeV);
			if (!error)
				fBits.set(iNudgeV) = true;
		}
	}

	if (!PIGetRulerOrigin(d1, d2))
	{
		if (d1 != fRulerOriginH && d2 != fRulerOriginV)
		{
			if (!error)
				error = PISetRulerOrigin(fRulerOriginH, fRulerOriginV);
			if (!error)
			{
				fBits.set(iHorizontal) = true;
				fBits.set(iVertical) = true;
			}
		}
		else if (d1 != fRulerOriginH)
		{
			if (!error)
				error = PISetRulerOrigin(fRulerOriginH, d2);
			if (!error)
				fBits.set(iHorizontal) = true;
		}
		else if (d2 != fRulerOriginV)
		{
			if (!error)
				error = PISetRulerOrigin(d1, fRulerOriginV);
			if (!error)
				fBits.set(iVertical) = true;
		}
	}

	if (!PIGetGrid(d1, d2))
	{
		if (d1 != fGridMajor && d2 != fGridMinor)
		{
			if (!error)
				error = PISetGrid(fGridMajor, fGridMinor);
			if (!error)
			{
				fBits.set(iGridMajor) = true;
				fBits.set(iGridMinor) = true;
			}
		}
		else if (d1 != fGridMajor)
		{
			if (!error)
				error = PISetGrid(fGridMajor, d2);
			if (!error)
				fBits.set(iGridMajor) = true;
		}
		else if (d2 != fGridMinor)
		{
			if (!error)
				error = PISetGrid(d1, fGridMinor);
			if (!error)
				fBits.set(iGridMinor) = true;
		}
	}

	if (!PIGetWatchSuspension(i))
		if (i != fWatchSuspension)
		{
			if (!error)
				error = PISetWatchSuspension(fWatchSuspension);
			if (!error)
				fBits.set(iWatch) = true;
		}

	if (!PIGetCopyright(b))
		if (b != fCopyright && fCopyright)
		{
			if (!error)
				error = PISetCopyright(fCopyright);
			if (!error)
				fBits.set(iCopyright) = true;
		}

	if (!PIGetURL(s))
		if (s.compare(fURL) != 0)
		{
			if (!error)
				error = PISetURL(fURL);
			if (!error)
				fBits.set(iURL) = true;
		}

	if (!PIGetWatermark(b))
		if (b != fWatermark && fWatermark)
		{
			if (!error)
				error = PISetWatermark();
			if (!error)
				fBits.set(iWater) = true;
		}

	if (!PIGetSelectedSliceID(i))
		if (i != fSliceID)
		{
			if (!error)
				error = PISetSelectedSliceID(fSliceID);
			if (!error)
				fBits.set(iSliceID) = true;
		}

	if (!PIGetSelectedSliceIndex(i))
		if (i != fSliceIndex)
		{
			if (!error)
				error = PISetSelectedSliceIndex(fSliceIndex);
			if (!error)
				fBits.set(iSliceIndex) = true;
		}

	if (!PIGetXMP(s))
		if (s.compare(fXMP) != 0)
		{
			if (!error)
				error = PISetXMP(fXMP);
			if (!error)
				fBits.set(iXMP) = true;
		}

	return error;
}

void FileToHandle(const string & fileName, Handle & h)
{
	h = NULL;
	vector<char> data;
	#if __PIMac__
		ifstream inFile(fileName.c_str(), ios::in|ios::binary);
	#else
		// j systems have trouble opening the file with Shift-JIS chars
		// use this trick
		ifstream inFile(fopen(fileName.c_str(), "rb"));
	#endif
	do
	{
		char c;
		inFile.read(&c, 1);
		if (inFile.gcount())
		{
			data.push_back(c);
		}
	} while (inFile.gcount());

	size_t s = data.size();
	if (s)
	{
		h = sPSHandle->New((int32)s);
		if (h != NULL)
		{
			Ptr p = NULL;
			sPSHandle->SetLock(h, true, &p, NULL);
			if (p != NULL)
			{
				for(size_t a = 0; a < s; a++, p++)
					*p = data.at(a);
				sPSHandle->SetLock(h, false, &p, NULL);
			}
			else
			{
				sPSHandle->Dispose(h);
				h = NULL;
			}
		}
	}
}

void HandleToFile(const string & fileName, const Handle h)
{
	#if __PIMac__
		ofstream outFile(fileName.c_str(), ios::out|ios::binary);
	#else
		// j systems have trouble opening the file with Shift-JIS chars
		// use this trick
		ofstream outFile(fopen(fileName.c_str(), "wb"));
	#endif
	
	int32 s = sPSHandle->GetSize(h);
	
	if (s > 0)
	{
		Ptr p = NULL;
		sPSHandle->SetLock(h, true, &p, NULL);
		if (p != NULL)
		{
			Ptr o = p;
			for (int32 a = 0; a < s; a++, o++)
				outFile << *o;
			sPSHandle->SetLock(h, false, &p, NULL);
		}
	}
}

void FileToString(const string & fileName, string & inString)
{
	#if __PIMac__
		ifstream inFile(fileName.c_str(), ios::in);
	#else
		// j systems have trouble opening the file with Shift-JIS chars
		// use this trick
		ifstream inFile(fopen(fileName.c_str(), "r"));
	#endif
	do
	{
		char c;
		inFile.read(&c, 1);
		if (inFile.gcount())
		{
			inString += c;
		}
	} while (inFile.gcount());
}

void StringToFile(const string & fileName, const string & outString)
{
	#if __PIMac__
		ofstream outFile(fileName.c_str(), ios::out);
	#else
		// j systems have trouble opening the file with Shift-JIS chars
		// use this trick
		ofstream outFile(fopen(fileName.c_str(), "w"));
	#endif

	for (uint32 i =0; i < outString.length(); i++)
		outFile << outString[i];
}
// end Propetizer.cpp

