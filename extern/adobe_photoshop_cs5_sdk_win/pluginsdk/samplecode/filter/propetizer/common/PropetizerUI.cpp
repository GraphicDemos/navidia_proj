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
/*
	File: PropetizerWin.cpp
*/

#include <vector>
#include <sstream>
#include "PropertyUtils.h"
#include "Propetizer.h"
#include "PIUI.h"

typedef string (* StringFunc)(void);

string GetDocumentNameString(void);
string GetModeString(void);
string GetLayerLockingString(void);
string GetWorkPathIndexString(void);
string GetClippingPathIndexString(void);
string GetTargetPathIndexString(void);
string InterpolationModeString(void);
string RulerUnitsString(void);
string SerialString(void);
string OldSerialString(void);
string HeightWidthString(void);
string ToolTipsString(void);
string PaintCursorKindString(void);
string PlayInProgressString(void);
string HostVersionString(void);
string ShowSliceNumbers(void);
string SliceLineColor(void);

typedef struct ItemAndFunc {
    const char * text;
	StringFunc func;
} ItemAndFunc;

ItemAndFunc const staticItems[] = { 
	{ "Serial: ", SerialString },
	{ "Old Serial: ", OldSerialString }, 
	{ "InterpolationMethod: ", InterpolationModeString },
	{ "LayerLocking: ", GetLayerLockingString },
	{ "Title: ",  GetDocumentNameString },
	{ "ImageMode: ", GetModeString },
	{ "WorkPathIndex: ", GetWorkPathIndexString },
	{ "ClippingPathIndex: ", GetClippingPathIndexString },
	{ "TargetPathIndex: ", GetTargetPathIndexString },
	{ "RulerUnits: ", RulerUnitsString },
	{ "Height, Width: ", HeightWidthString },
	{ "Tool tips: ", ToolTipsString },
	{ "Cursor Kind: ", PaintCursorKindString },
	{ "Play in Progress: ", PlayInProgressString },
	{ "Host Version: ", HostVersionString },
	{ "Show Slice Numbers: ", ShowSliceNumbers },
	{ "Slice Line Color: ", SliceLineColor },
};

typedef vector<string> (* VStringFunc)(void);

vector<string> GetLayerNames(void);
vector<string> GetChannelNames(void);
vector<string> GetPathNames(void);
vector<string> GetInterfaceColors(void);

typedef struct ComboAndFunc {
	const char * text;
	VStringFunc func;
} ComboAndFunc;

ComboAndFunc const comboItems[] = {
	{ "Layer Names: ", GetLayerNames },
	{ "Channel Names: ", GetChannelNames },
	{ "Path Names: ", GetPathNames },
	{ "Interface Colors: ", GetInterfaceColors },
};

string ModeToString(intptr_t mode);
string InterpolationToString(InterpolationMethod m);
string RulerUnitsToString(RulerUnits r);
string CursorKindToString(PaintCursorKind c);
string InterfaceColorToString(InterfaceColor color);

#if !__LP64__

class PropetizerDialog : public PIDialog {
private:
	double fBigNudgeV;
	double fBigNudgeH;
	double fRulerOriginV;
	double fRulerOriginH;
	double fGridMajor;
	double fGridMinor;
	int32 fWatchSuspension;
	bool fCopyright;
	string fURL;
	bool fWatermark;
	int32 fSliceID;
	int32 fSliceIndex;
	string fXMP;

	int32 fiBigNudgeV;
	int32 fiBigNudgeH;
	int32 fiRulerOriginV;
	int32 fiRulerOriginH;
	int32 fiGridMajor;
	int32 fiGridMinor;
	int32 fiWatchSuspension;
	int32 fiCopyright;
	int32 fiURL;
	int32 fiWatermark;
	int32 fiSliceID;
	int32 fiSliceIndex;
	int32 fiXMP;

	virtual void Init(void);
	virtual void Notify(int32 index);
public:
	PropetizerDialog() : PIDialog() {}
	~PropetizerDialog() {}

	void SetBigNudgeH(double in) { fBigNudgeH = in; }
	double GetBigNudgeH(void) { return fBigNudgeH; }
	void SetBigNudgeV(double in) { fBigNudgeV = in; }
	double GetBigNudgeV(void) { return fBigNudgeV; }
	void SetRulerOriginH(double in) { fRulerOriginH = in; }
	double GetRulerOriginH(void) { return fRulerOriginH; }
	void SetRulerOriginV(double in) { fRulerOriginV = in; }
	double GetRulerOriginV(void) { return fRulerOriginV; }
	void SetGridMajor(double in) { fGridMajor = in; }
	double GetGridMajor(void) { return fGridMajor; }
	void SetGridMinor(double in) { fGridMinor = in; }
	double GetGridMinor(void) { return fGridMinor; }
	void SetWatchSuspension(int32 in) { fWatchSuspension = in; }
	int32 GetWatchSuspension(void) { return fWatchSuspension; }
	void SetCopyright(bool in) { fCopyright = in; }
	bool GetCopyright(void) { return fCopyright; }
	void SetURL(string in) { fURL = in; }
	string GetURL(void) { return fURL; }
	void SetWatermark(bool in) { fWatermark = in; }
	bool GetWatermark(void) { return fWatermark; }
	void SetSliceID(int32 in) { fSliceID = in; }
	int32 GetSliceID(void) { return fSliceID; }
	void SetSliceIndex(int32 in) { fSliceIndex = in; }
	int32 GetSliceIndex(void) { return fSliceIndex; }
	void SetXMP(string inXMP) { fXMP = inXMP; }
	string GetXMP(void) { return fXMP; }

};

typedef string (* EStringFunc)(PropetizerDialog & pd);

string BigNudgeHString(PropetizerDialog & pd);
string BigNudgeVString(PropetizerDialog & pd);
string RulerOriginHeightString(PropetizerDialog & pd);
string RulerOriginWidthString(PropetizerDialog & pd);
string GridMajorString(PropetizerDialog & pd);
string GridMinorString(PropetizerDialog & pd);
string WatchSuspensionString(PropetizerDialog & pd);
string SliceIDString(PropetizerDialog & pd);
string SliceIndexString(PropetizerDialog & pd);

typedef struct EditAndFunc {
	const char * text;
	EStringFunc func;
} EditAndFunc;

EditAndFunc const editItems[] = {
	{ "BigNudge H:", BigNudgeHString },
	{ "V:", BigNudgeVString },
	{ "Ruler Origin H:", RulerOriginHeightString },
	{ "W:", RulerOriginWidthString },
	{ "Grid Major: ", GridMajorString },
	{ "Minor: ", GridMinorString },
	{ "Watch Suspension: ", WatchSuspensionString },
	{ "Slice ID: ", SliceIDString },
	{ "Index: ", SliceIndexString },
};

typedef bool (* BoolFunc)(PropetizerDialog & pd);

bool CopyrightChecked(PropetizerDialog & pd);
bool WatermarkChecked(PropetizerDialog & pd);

typedef struct CheckAndFunc {
	const char * text;
	BoolFunc func;
} CheckAndFunc;

CheckAndFunc const checkItems[] = {
	{ "Copyright", CopyrightChecked },
	{ "Watermark", WatermarkChecked },
};

/****************************************************************************/

int32 PropetizerData::DoUI(void)
{
	PropetizerDialog dialog;
	
	dialog.SetBigNudgeH(GetBigNudgeH());
	dialog.SetBigNudgeV(GetBigNudgeV());
	dialog.SetRulerOriginH(GetRulerOriginH());
	dialog.SetRulerOriginV(GetRulerOriginV());
	dialog.SetGridMajor(GetGridMajor());
	dialog.SetGridMinor(GetGridMinor());
	dialog.SetWatchSuspension(GetWatchSuspension());
	dialog.SetCopyright(GetCopyright());
	dialog.SetURL(GetURL());
	dialog.SetWatermark(GetWatermark());
	dialog.SetSliceID(GetSliceID());
	dialog.SetSliceIndex(GetSliceIndex());
	dialog.SetXMP(GetXMP());

	int32 id = dialog.Modal(NULL, NULL, 16001);
	
	if (id == 1)
	{
		SetBigNudgeH(dialog.GetBigNudgeH());
		SetBigNudgeV(dialog.GetBigNudgeV());
		SetRulerOriginH(dialog.GetRulerOriginH());
		SetRulerOriginV(dialog.GetRulerOriginV());
		SetGridMajor(dialog.GetGridMajor());
		SetGridMinor(dialog.GetGridMinor());
		SetWatchSuspension(dialog.GetWatchSuspension());
		SetCopyright(dialog.GetCopyright());
		SetURL(dialog.GetURL());
		SetWatermark(dialog.GetWatermark());
		SetSliceID(dialog.GetSliceID());
		SetSliceIndex(dialog.GetSliceIndex());
		SetXMP(dialog.GetXMP());
	}
	return id;
}

void PropetizerDialog::Init(void)
{
	int16 dIndex = 3;
	int32 i = 0;

	PIText sText;

	for (i = 0; i < sizeof(staticItems)/sizeof(ItemAndFunc); i++)
	{
        string s = staticItems[i].text + staticItems[i].func();
		sText = GetItem(dIndex++);
		sText.SetText(s.c_str());
	}

	for (i = 0; i < sizeof(comboItems)/sizeof(ComboAndFunc); i++)
	{
		sText = GetItem(dIndex++);
		sText.SetText(comboItems[i].text);
		vector<string> vs = comboItems[i].func();
		PIComboBox combo = GetItem(dIndex++);
		combo.Clear();
		if (!vs.empty())
		{
			for (size_t ii = 0; ii < vs.size(); ii++)
				combo.AppendItem(vs.at(ii).c_str());
			combo.SetCurrentSelection(0);
		}
	}

	// from here down I need to know about the item id's
	// so the notifier routine can read all the items
	// this isn't the cleanest and probably needs work
	fiBigNudgeH = dIndex + 1;
	fiBigNudgeV = fiBigNudgeH + 2;
	fiRulerOriginH = fiBigNudgeV + 2;
	fiRulerOriginV = fiRulerOriginH + 2;
	fiGridMajor = fiRulerOriginV + 2;
	fiGridMinor = fiGridMajor + 2;
	fiWatchSuspension = fiGridMinor + 2;
	fiSliceID = fiWatchSuspension + 2;
	fiSliceIndex = fiSliceID + 2;

	for (i = 0; i < sizeof(editItems)/sizeof(EditAndFunc); i++)
	{
		sText = GetItem(dIndex++);
		sText.SetText(editItems[i].text);
		sText = GetItem(dIndex++);
		sText.SetText(editItems[i].func(*this).c_str());
	}
	
	fiCopyright = dIndex;
	fiWatermark = fiCopyright + 1;

	for (i = 0; i < sizeof(checkItems)/sizeof(CheckAndFunc); i++)
	{
		PICheckBox check = GetItem(dIndex++);
		check.SetText(checkItems[i].text);
		check.SetChecked(checkItems[i].func(*this));
	}

	fiURL = dIndex;
	sText = GetItem(dIndex++);
	sText.SetText(GetURL().c_str());

	fiXMP = dIndex;
	sText = GetItem(dIndex++);
	sText.SetText(GetXMP().c_str());

	sText = GetItem(dIndex++);
	if ( IsRunningNatively() )
		sText.SetText("Running natively");
	else
		sText.SetText("Running in Rosetta!");
}

void PropetizerDialog::Notify(int32 index)
{
	PIText item;
	PICheckBox check;
	string text;
	
	if (index == 1)
	{
		item = GetItem(fiBigNudgeV);
		item.GetText(text);
		SetBigNudgeV(atof(text.c_str()));

		item = GetItem(fiBigNudgeH);
		item.GetText(text);
		SetBigNudgeH(atof(text.c_str()));

		item = GetItem(fiRulerOriginV);
		item.GetText(text);
		SetRulerOriginV(atof(text.c_str()));

		item = GetItem(fiRulerOriginH);
		item.GetText(text);
		SetRulerOriginH(atof(text.c_str()));

		item = GetItem(fiGridMajor);
		item.GetText(text);
		SetGridMajor(atof(text.c_str()));

		item = GetItem(fiGridMinor);
		item.GetText(text);
		SetGridMinor(atof(text.c_str()));

		item = GetItem(fiWatchSuspension);
		item.GetText(text);
		SetWatchSuspension(atoi(text.c_str()));

		check = GetItem(fiCopyright);
		SetCopyright(check.GetChecked());

		item = GetItem(fiURL);
		item.GetText(text);
		SetURL(text);

		item = GetItem(fiXMP);
		item.GetText(text);
		SetXMP(text);

		check = GetItem(fiWatermark);
		SetWatermark(check.GetChecked());

		item = GetItem(fiSliceID);
		item.GetText(text);
		SetSliceID(atoi(text.c_str()));

		item = GetItem(fiSliceIndex);
		item.GetText(text);
		SetSliceIndex(atoi(text.c_str()));
	}
}
string ModeToString(intptr_t mode)
{
	string s;
	switch(mode)
	{
		case plugInModeBitmap:
			s = "Bitmap";
			break;
		case plugInModeGrayScale:
			s = "GrayScale";
			break;
		case plugInModeIndexedColor:
			s = "IndexedColor";
			break;
		case plugInModeRGBColor:
			s = "RGB";
			break;
		case plugInModeCMYKColor:
			s = "CMYK";
			break;
		case plugInModeHSLColor:
			s = "HSL";
			break;
		case plugInModeHSBColor:
			s = "HSB";
			break;
		case plugInModeMultichannel:
			s = "Multichannel";
			break;
		case plugInModeDuotone:
			s = "Dutotone";
			break;
		case plugInModeLabColor:
			s = "Lab";
			break;
		case plugInModeGray16:
			s = "Gray16";
			break;
		case plugInModeRGB48:
			s = "RGB48";
			break;
		case plugInModeLab48:
			s = "Lab48";
			break;
		case plugInModeCMYK64:
			s = "CMYK64";
			break;
		case plugInModeDeepMultichannel:
			s = "DeepMultichannel";
			break;
		case plugInModeDuotone16:
			s = "Duotone16";
			break;
		default:
			s = "Unkown";
			break;
	}
	return s;
}

string InterpolationToString(InterpolationMethod m)
{
	string s;
	switch(m)
	{
		case  pointSample:
			s = "Point sample";
			break;
		case bilinear:
			s = "Bilinear";
			break;
		case  bicubic:
			s = "Bicubic";
			break;
		default:
			s = "Unknown";
			break;
	}
	return s;
}

string RulerUnitsToString(RulerUnits r)
{
	string s;
	switch (r)
	{
		case kRulerPixels:
			s = "Pixels";
			break;
		case kRulerInches:
			s = "Inches";
			break;
		case kRulerCm:
			s = "CM";
			break;
		case kRulerMillimeters:
			s = "MM";
			break;
		case kRulerPoints:
			s = "Points";
			break;
		case kRulerPicas:
			s = "Picas";
			break;
		case kRulerPercent:
			s = "Percent";
			break;
		default:
			s = "Unknown";
			break;
	}
	return s;
}

string CursorKindToString(PaintCursorKind c)
{
	string s;
	switch (c)
	{
		case standard:
			s = "Standard";
			break;
		case precise:
			s = "Precise";
			break;
		case brushSize:
			s = "Brush Size";
			break;
		default:
			s = "Unknown";
			break;
	}
	return s;
}

string InterfaceColorToString(InterfaceColor color)
{
	string s;
	switch (color)
	{
		case white:
			s = "white";
			break;
		case buttonUpFill:
			s = "buttonUpFill";
			break;
		case bevelShadow:
			s = "bevelShadow";
			break;
		case iconFillActive:
			s = "iconFillActive";
			break;
		case iconFillDimmed:
			s = "iconFillDimmed";
			break;
		case paletteFill:
			s = "paletteFill";
			break;
		case iconFrameDimmed:
			s = "iconFrameDimmed";
			break;
		case iconFrameActive:
			s = "iconFrameActive";
			break;
		case bevelHighlight:
			s = "bevelHighlight";
			break;
		case buttonDownFill:
			s = "buttonDownFill";
			break;
		case iconFillSelected:
			s = "iconFillSelected";
			break;
		case border:
			s = "border";
			break;
		case buttonDarkShadow:
			s = "buttonDarkShadow";
			break;
		case iconFrameSelected:
			s = "iconFrameSelected";
			break;
		case black:
			s = "black";
			break;
		case red:
			s = "red";
			break;
		default:
			s = "Uknown";
			break;
	}
	return s;
}

string GetDocumentNameString(void)
{
	string s;
	if (PIGetDocumentName(s))
		s = "Unknown";
	return s;
}

string GetModeString(void)
{
	string s = "Unknown";
	intptr_t i;
	if (!PIGetImageMode(i) && i)
		s = ModeToString(i);
	return s;
}

string GetLayerLockingString(void)
{	
	string s = "Unknown";
	bool b, b1, b2;

	if (!PIGetTargetLayerLock(b, b1, b2))
	{
		if (b)
			s = "transparency ";
		if (b1)
			s = s + "composite ";
		if (b2)
			s = s + "position";
		if (!b && !b1 && !b2)
			s = "";
	}

	return s;
}

string GetWorkPathIndexString(void)
{
	string s = "Unknown";
	intptr_t i;
	if (!PIGetWorkPathIndex(i))
	{
		ostringstream ss;
		ss << i;
		s = ss.str();
	}
	return s;
}

string GetClippingPathIndexString(void)
{
	string s = "Unknown";
	intptr_t i;
	if (!PIGetClippingPathIndex(i))
	{
		ostringstream ss;
		ss << i;
		s = ss.str();
	}
	return s;
}

string GetTargetPathIndexString(void)
{
	string s = "Unknown";
	intptr_t i;
	if (!PIGetTargetPathIndex(i))
	{
		ostringstream ss;
		ss << i;
		s = ss.str();
	}
	return s;
}

string InterpolationModeString(void)
{
	string s = "Unknown";
	InterpolationMethod m;
	if (!PIGetInterpolationMethod(m))
		s = InterpolationToString(m);
	return s;
}

string RulerUnitsString(void)
{
	string s = "Unknown";
	RulerUnits r;
	if (!PIGetRulerUnits(r))
		s = RulerUnitsToString(r);
	return s;
}

string SerialString(void)
{
	string s = "Unknown";
	(void)PIGetSerialString(s);
	return s;
}

string OldSerialString(void)
{
	string s = "Unknown";
	(void)PIGetSerialStringOld(s);
	return s;
}

string HeightWidthString(void)
{
	ostringstream ss;
	intptr_t i;
	if (!PIGetDocumentHeight(i))
		ss << i;
	else
		ss << "Unknown";
	
	ss << ", ";

	if (!PIGetDocumentWidth(i))
		ss << i;
	else
		ss << "Unknown";

	return ss.str();
}

string ToolTipsString(void)
{
	string s = "Unknown";
    bool b;
	if (!PIGetToolTips(b))
		if (b)
			s = " on";
		else 
			s = " off";
	return s;
}

string PaintCursorKindString(void)
{
	string s = "Unknown";
	PaintCursorKind cursorKind;
	if (!PIGetPaintCursorKind(cursorKind))
		s = CursorKindToString(cursorKind);
	return s;
}

string PlayInProgressString(void)
{
	string s = "Unknown";
	bool b;
	if (!PIGetPlayInProgress(b))
		if (b)
			s = "Yes";
		else
			s = "No";
	return s;
}

string HostVersionString(void)
{
	ostringstream ss;
	int32 major, minor, fix;
	if (!PIGetHostVersion(major, minor, fix))
		ss << major << '.' << minor << '.' << fix;
	else
		ss << "Unknown";
	return ss.str();
}

string ShowSliceNumbers(void)
{
	string s;
    bool b;
	if (!PIGetShowSliceNumbers(b))
		if (b)
			s = "Yes";
		else
			s = "No";
	else
		s = "Unknown";
	return s;
}

string SliceLineColor(void)
{
	ostringstream ss;
	RGBColor lineColor;
	if (!PIGetSliceLineColor(lineColor))
	{
		ios::fmtflags remember = ss.setf(ios::hex, ios::basefield);
		ss << lineColor.red << ", ";
		ss << lineColor.green << ", ";
		ss << lineColor.blue;
		ss.setf(remember);
	}
	else
		ss << "Unknown";
	return ss.str();
}

vector<string> GetLayerNames(void)
{
	vector<string> vs;
	intptr_t i;
	if (!PIGetNumberLayers(i) && i > 0)
	{
		vs.reserve(i);
		for(int32 a = 0; a < i; a++)
		{
			string s;
			if (!PIGetLayerName(a, s))
				vs.push_back(s);
		}
	}
	return vs;
}

vector<string> GetChannelNames(void)
{
	vector<string> vs;
	intptr_t i;
	if (!PIGetNumberChannels(i) && i > 0)
	{
		vs.reserve(i);
		for(int32 a = 0; a < i; a++)
		{
			string s;
			if (!PIGetChannelName(a, s))
				vs.push_back(s);
		}
	}
	return vs;
}

vector<string> GetPathNames(void)
{
	vector<string> vs;
	intptr_t i;
    if (!PIGetNumberPaths(i) && i > 0)
	{
		vs.reserve(i);
		for(int32 a = 0; a < i; a++)
		{
			string s;
			if (!PIGetPathName(a, s))
				vs.push_back(s);
		}
	}
	return vs;
}

vector<string> GetInterfaceColors(void)
{
	vector<string> vs;
	PIInterfaceColor iColor;
	for (int32 color = white; color < red; color++)
		if (!PIGetInterfaceColor(static_cast<InterfaceColor>(color), iColor))
		{
			string s = InterfaceColorToString(static_cast<InterfaceColor>(color));
			ostringstream ss;
			ios::fmtflags remember = ss.setf(ios::hex, ios::basefield);
			ss << static_cast<short>(iColor.color2.alpha) << ", ";
			ss << static_cast<short>(iColor.color2.r) << ", ";
			ss << static_cast<short>(iColor.color2.g) << ", ";
			ss << static_cast<short>(iColor.color2.b);
			ss << "  " << static_cast<short>(iColor.color32.alpha) << ", ";
			ss << static_cast<short>(iColor.color32.r) << ", ";
			ss << static_cast<short>(iColor.color32.g) << ", ";
			ss << static_cast<short>(iColor.color32.b);
			s = s + " " + ss.str();
			vs.push_back(s);
			ss.setf(remember);
		}
	return vs;
}

string BigNudgeHString(PropetizerDialog & pd)
{
	ostringstream ss;
	ss << pd.GetBigNudgeH();
	return ss.str();
}

string BigNudgeVString(PropetizerDialog & pd)
{
	ostringstream ss;
	ss << pd.GetBigNudgeV();
	return ss.str();
}

string RulerOriginHeightString(PropetizerDialog & pd)
{
    ostringstream ss;
	ss << pd.GetRulerOriginH();
	return ss.str();
}

string RulerOriginWidthString(PropetizerDialog & pd)
{
	ostringstream ss;
	ss << pd.GetRulerOriginV();
	return ss.str();
}

string GridMajorString(PropetizerDialog & pd)
{
	ostringstream ss;
    ss << pd.GetGridMajor();
	return ss.str();
}

string GridMinorString(PropetizerDialog & pd)
{
	ostringstream ss;
    ss << pd.GetGridMinor();
	return ss.str();
}

string WatchSuspensionString(PropetizerDialog & pd)
{
	ostringstream ss;
    ss << pd.GetWatchSuspension();
	return ss.str();
}

string SliceIDString(PropetizerDialog & pd)
{
	ostringstream ss;
    ss << pd.GetSliceID();
	return ss.str();
}

string SliceIndexString(PropetizerDialog & pd)
{
	ostringstream ss;
    ss << pd.GetSliceIndex();
	return ss.str();
}

bool CopyrightChecked(PropetizerDialog & pd)
{
	return pd.GetCopyright();
}

bool WatermarkChecked(PropetizerDialog & pd)
{
	return pd.GetWatermark();
}

#endif // #if !__LP64__

//-------------------------------------------------------------------------------
//
// DoAbout
//
// Pop a simple about box for this plug in.
//
// NOTE:	The global gFilterRecord is NOT a FilterRecord*. You must cast it to
//			an AboutRecord*. See PIAbout.h
//
//-------------------------------------------------------------------------------
void DoAbout(void)
{
#if __PIWin__
	ShowAbout((AboutRecord*)gFilterRecord);
#else // #if __PIWin__
	ShowAbout(AboutID);
#endif // #if __PIWin__
}

// end PropetizerUIWin.cpp