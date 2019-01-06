/***********************************************************************/ /** 
 ** \file nv_rtWindowsitf.h 
 **
 ** Definition of classes and functions that are exported.
 **
 ** Implemented objects using these interfaces are :
 ** 
 ** - "profilingwnd": Used to display profiling results
 ** - "progressbar": progress bar
 ** - "logwnd": logging window
 ** - "consolewnd": console window
 ** - "wndmsghandler": singleton used to handle windows messages.
 ** - "tabcontainer": container window with tabs
 ** - "simplecontainer": simple container which aligns the controls vertically
 ** - "splittercontainer": splitter container
 ** - "splittercontainerh" : horizontal splitter container
 ** - "foldingcontainer": folding container which aligns the controls vertically and allow you to open/close these items
 ** - "scalarwnd": a simple scalar value editor
 ** - "vectorwnd": a simple vector value editor
 ** - "stringwnd" : a simple string value editor
 ** - "toolbarwnd"
 ** - "colorwnd"
 ** - "combownd"
 ** - "booleanwnd"
 ** - "buttonwnd"
** .
 ** 
 **/ /***********************************************************************/
#ifndef __IDlgSettings__
#define __IDlgSettings__

#define WINDOWS_VERSION "1.41"

#include "sapi_smartptr_itf.h"

using namespace sapi;

class IEventsWnd;
class IContainerWnd;
/*************************************************************************/ /**
 ** Common Window definition
 **/ /*************************************************************************/
class IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void PeekMyself() = 0; ///< We should remove this...
	virtual long OnRender(float fElapsedTime) = 0; ///< in the case where the UI is rendered with 3D, for example
	virtual bool MsftMsgProc( unsigned long hWnd, unsigned int uMsg, unsigned int wParam, unsigned long lParam ) = 0; ///< specific to Microsoft msg pump
	virtual void Show() = 0; ///< should disapear for Visible() instead
	virtual void Hide() = 0; ///< should disapear for Visible() instead
	virtual int Visible(int bYes=1) = 0; ///< bYes=-1 to query the state
	virtual int Minimize(int bYes=1) = 0; ///< bYes=-1 to query the state
	virtual void *GetCWnd() = 0; // void* because we don't want MS here...
	virtual unsigned long GetHandle() = 0; // unsigned long because we don't want MS here (hwnd)...
	virtual void Register(IEventsWnd *pDataBase) = 0;
	virtual void UnRegister(IEventsWnd *pClient=NULL) = 0;
	virtual void SetPosition(int x, int y, int width=-1, int height=-1) = 0;
	virtual void GetPosition(int *x, int *y, int *width, int *height) = 0;
	virtual void SetZPos(int z=1) = 0;
	virtual void SetTitle(LPCSTR title) = 0;
	virtual void GetTitle(char * title, int maxlen) = 0;
	/// Use this to set some tags to the window.
	virtual void SetUserLong(unsigned long userparam1, unsigned long userparam2) = 0;
	/// return val : the first user long.
	virtual unsigned long GetUserLong(unsigned long *userparam1=NULL, unsigned long *userparam2=NULL) = 0;
	//
	// links with parent (new from v1.37)
	//
	virtual IContainerWnd *GetParentContainer() = 0;
	//
	// To destry the window. Note: this instance may still be referenced to various objects (RefCnt > 0).
	// so the destroy status will release some resources and will set as invalid.
	// Note that Destroy will notify registered users with the event 'rtDestroyingWindow', so this is the place where the user
	// can release the references to it.
	// (new from v1.37)
	//
	virtual void Destroy() = 0;
	virtual bool IsValid() = 0; // false is this was destroyed. This can be used to release the reference to this window
	//
	// This is void * because this interface can be used without the plug feature (another lib/include)...
	// so we must force the casting from void* to (plug::Plug *) :( I know... not very clean.
	//
	virtual void *GetPlug(int n=0) = 0;
};

/*************************************************************************/ /**
 ** must be implemented by the client if it wants to get back events
 ** 
 ** \u Note : if return false, it means that the object may handle this 
 ** event with a default process
 **/ /*************************************************************************/
class IEventsWnd
{
public:
	virtual bool rtMouseMove(IWindow *, float mx, float my, unsigned int buttons, unsigned long lval) = 0;
	virtual void rtDataUpdated(IWindow *, unsigned long types) = 0;
	virtual bool rtWindowClosed(IWindow *) = 0;
	virtual bool rtWindowDestroyed(IWindow *) = 0; ///< TO REMOVE
	virtual bool rtDestroyingWindow(IWindow *) = 0; ///< called when we asked for this window to be destroyed (1.37).
	virtual bool rtWindowResized(IWindow *, int cx, int cy) = 0;
	virtual bool rtWindowRepaint(IWindow *) = 0;
  /**
	used by controls to send events in values changes:
	ctl_bool, ctl_color, ctl_combo, ctl_scalar, ctl_string, ctl_vector
	return 0 if nothing changed by this event. 1 if we changed something in valptr.
  //
  Actually this event is called as follow :
  - Button :     rtCtrlEvent(object, "button", 0, 0, luser1, luser2 );
  - Checkbox :   rtCtrlEvent(object, "bool", (bool*)check_state_ptr, (float)check_state, luser1, luser2 );
  - Colordlg :   rtCtrlEvent(object, "color", (float*)color_vec_ptr, 0, luser1, luser2 );
  - Combodlg :   rtCtrlEvent(object, "combo", (int*)item_number_ptr, item_data[item_number], luser1, luser2 );
  - Scalardlg :  rtCtrlEvent(object, "scalar", (float*)value_ptr, value, luser1, luser2 );
  - Stringdlg :  rtCtrlEvent(object, "string", (char*)string_ptr, 0, luser1, luser2 );
  - \e (TOCHANGE) Toolbardlg : rtCtrlEvent(object, "toolbar", item_number, state_item[item_number], tag_item[item_number], luser1 );
  - Vectordlg :  rtCtrlEvent(object, "vector", vec_ptr, 0, luser1, luser2 );
  .
  **/
	virtual int rtCtrlEvent(IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2 ) = 0;

	/// This method can be implemented for any purpose. Depending on the instanciated object...
	//virtual void rtCustomMessage(IWindow *, const char *, unsigned long ul1, unsigned long ul2) =0;
	//...to be continued...
};
/*************************************************************************/ /**
 This window may be used only by nv_rtDbgTools module
 */ /*********************************************************************/
class IDlgClientProfiling : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void Printf(char* fmt, ... ) = 0;
	virtual void Clear() = 0;
	virtual void AddLine(LPCSTR ave2, LPCSTR min, LPCSTR max, LPCSTR name, LPCSTR card) = 0;
	virtual void SetFPS(int fps) = 0;
};

class IProgressBar : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void SetPercent(float l) = 0;
	virtual void AddPercent(float l) = 0;
	virtual void SetMessage(LPCSTR lpstr) = 0;
};

/*************************************************************************/ /**
 ** Logging dlg box interface
 **
 ** various cases in the line:
 ** <LI> no header : common msg
 ** <LI> "<<warning>> message" : Warning msg
 ** <LI> "<<error>> message" : Error Msg
 ** <LI> "<<event>> message" : event Msg used by entities
 ** <LI> "<<distrib>> message" : Networking msg
 ** <LI> "<<sound>> message" : Sound Msg
 **/ /*************************************************************************/ 
class IDlgLog : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void Clear() = 0;
	virtual void AddMessage(LPCSTR fmt, ...) = 0;
};

/*************************************************************************/ /**
 ** Singleton Interface of Windows message loop
 **/ /*************************************************************************/ 
class IWndMsgHandler
{
public:
	/** return only when windows has finished.
	 */
	virtual unsigned int HandleMessageLoop_Blocking() = 0;
	/** returns only wen a Key was pressed : Virtual Key code value returned
	 */
	virtual unsigned int HandleMessageLoop_ReturnOnKeyPressed() = 0;
	/** returns At every event. Virtual Key Code returned. Or 0 otherwise.
	 */
	virtual unsigned int HandleMessageLoop_OnePass() = 0;
	/** returns 0 if the method handled the message
	 */
	virtual bool MsftMsgProc( unsigned long hWnd, unsigned int uMsg, unsigned int wParam, unsigned long lParam ) = 0; ///< specific to Microsoft msg pump
	/** To initialize the environnment : devices in the case of D3D (DXUTInitialize3DEnvironment)
	 */
	virtual long CreateUIResources(void *window, void *devitf) = 0;
	/** when we need to Reset the UI
	 */
	virtual long ResetUIResources() = 0;
	/** when we lost the resources
	 */
	virtual void LostUIResources() = 0;
	/** when we destroy the whole
	 */
	virtual void DestroyUIResources() = 0;
};

/*************************************************************************/ /**
 ** Container window
 **/ /*************************************************************************/ 
class IGDIView : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void ShowSliders(bool bYes=true) = 0;
	virtual void SetZoom(float zoom=1) = 0;
	virtual void SetOffset(float x=0, float y=0) = 0;
	virtual void SetViewFrame(float x1, float y1, float x2, float y2) = 0;
};

/*************************************************************************/ /**
 ** Cardiogram-like graphic.
 ** \todo create methods to display this timeline...
 **/ /*************************************************************************/ 
class ICardio : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void SetPencil(int id, float y, unsigned long color=0xFFFFFF) = 0;
	virtual void DelPencil(int id) = 0;
	virtual void SetYBounds(float ymin, float ymax) = 0;
	virtual void Update(float dt=1) = 0;
};
/*************************************************************************/ /**
 ** Cardiogram-like graphic.
 ** \todo create methods to display this timeline...
 **/ /*************************************************************************/ 
class IConsoleWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void SetItalic(bool bYes=true) = 0;
	virtual void SetBold(int val=1) = 0;
	virtual void SetUnderline(bool bYes=true) = 0;
	virtual void SetColor(unsigned long rgbcolor=0) = 0;
	virtual void SetColor(unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void SetFontSize(int sz=0) = 0;
	virtual void SetFont(LPCSTR name=NULL) = 0;

	virtual void Clear() = 0;
	virtual void SetCursor(int x, int y) = 0;
	virtual void SetCursorToEnd() = 0;
	virtual void SetCursorToTop() = 0;
	virtual void Printf(char* fmt, ... ) = 0;
};

/*************************************************************************/ /**
 ** window container
 ** 
 **/ /*************************************************************************/ 
class IContainerWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual int getNumItems() = 0;
	virtual int InsertItem(int Itemnum, LPCSTR title, IWindow *lpWnd) = 0;
	virtual int AppendItem(LPCSTR title, IWindow *lpWnd) = 0;
	virtual int InsertItem(int Itemnum, LPCSTR title, LPCSTR windowname) = 0;
	virtual int AppendItem(LPCSTR title, LPCSTR windowname) = 0;
  /// \arg lpWnd return the interface. \warning if you get this pointer, reference counter is incremented. So you'll have to call Release after you're done.
	virtual int RemoveItem(int Itemnum, IWindow **lpWnd=NULL) = 0; 
	virtual int RemoveItem(IWindow *lpWnd) = 0;
	virtual int selectItem(int Itemnum) = 0;
};
/*************************************************************************/ /**
 ** window container with splitters
 ** 
 **/ /*************************************************************************/ 
class ISplitterContainerWnd : public IContainerWnd
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual int GetSplitterPos(int row) = 0;
	virtual int SetSplitterPos(int row, int y) = 0;
};
/*************************************************************************/ /**
 ** Scalar value DlgBox : title, val, slider
 ** 
 **/ /*************************************************************************/ 
class IScalarWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual float getValue() = 0;
	virtual LPCSTR getValueAsString() = 0;
	virtual void setIntMode(bool bYes=true) = 0;
	virtual void setValue(float s) = 0;
	virtual void setBounds(float min, float max) = 0;
	virtual void setStep(float s) = 0;
};
/*************************************************************************/ /**
 ** Vector 2,3,4 dimension
 ** 
 **/ /*************************************************************************/ 
class IVectorWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void setIntMode(bool bYes=true) = 0;
	virtual void setDimension(int dim) = 0;
	virtual int getValuesAsInt(int *x, int *y=NULL, int *z=NULL, int *w=NULL) = 0;
	virtual int getValuesAsFloat(float *x, float *y=NULL, float *z=NULL, float *w=NULL) = 0;
	virtual int getValuesAsDouble(double *x, double *y=NULL, double *z=NULL, double *w=NULL) = 0;
	virtual void setValue(float x, float y, float z, float w) = 0;
	virtual void setValue(int n, float val) = 0;
};
/*************************************************************************/ /**
 ** Simple 1 line string + optional file browser button
 ** 
 **/ /*************************************************************************/ 
class IStringWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void setString(LPCSTR str, bool bReplaceSelected=false) = 0;
	virtual LPCSTR getString() = 0;
	virtual LPCSTR getSelectedString() = 0;
	virtual int getStringLength() = 0;
	virtual int selectString(int start, int len) = 0;
	virtual void clearString(bool bOnlySelected=false) = 0;
	virtual LPCSTR ShowFileBrowseButton(LPCSTR filter=NULL,int bYes=1) = 0; ///<filter can be "Text|*.txt|All|*.*||"
};

/*************************************************************************/ /**
 ** Combo Box : to choose among various strings
 ** 
 **/ /*************************************************************************/ 
class IComboWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual void	clear() = 0;
	virtual int		addString(LPCSTR str, unsigned long ultag=0) = 0;
	virtual int		insert(int index, LPCSTR str, unsigned long ultag=0) = 0;
	virtual int		remove(int index) = 0;
	virtual int		select(int index) = 0;
	virtual int		getSelected(unsigned long *pultag=NULL, LPCSTR *Name=NULL) = 0;
	virtual unsigned long		getItem(int item, LPCSTR *Name=NULL) = 0;
	virtual int		getNumItems() = 0;
};

/*************************************************************************/ /**
 **
 ** Toolbar
 ** 
 **/ /*************************************************************************/ 
class IToolbarWnd : public IWindow
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	virtual int AppendItem(LPCSTR title, LPCSTR imagefilename, int ddsidx=0, int states=1, unsigned long tag=0) = 0;
	virtual int AppendItemFromMemoryBMP(LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int AppendItemFromMemoryPNG(LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int AppendItemFromMemoryTGA(LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int AppendItemFromMemoryDDS(LPCSTR title, void * imagedata, int ddsidx, int states=1, unsigned long tag=0) = 0;
	virtual int InsertItem(int pos, LPCSTR title, LPCSTR imagefilename, int ddsidx=0, int states=1, unsigned long tag=0) = 0;
	virtual int InsertItemFromMemoryBMP(int pos, LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int InsertItemFromMemoryPNG(int pos, LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int InsertItemFromMemoryTGA(int pos, LPCSTR title, void * imagedata, int states=1, unsigned long tag=0) = 0;
	virtual int InsertItemFromMemoryDDS(int pos, LPCSTR title, void * imagedata, int ddsidx=0, int states=1, unsigned long tag=0) = 0;
	virtual bool DeleteItem(int pos) = 0;
	virtual bool DeleteItem(LPCSTR title) = 0;
	virtual void ItemsSetState(int item, int state) = 0;
	virtual int ItemsGetState(int item) = 0;
	virtual int Vertical(int bYes=-1) = 0;
	virtual void ShowText(bool bYes=true) = 0;
	virtual void SetMinMaxHeight(int hmin, int hmax) = 0;
	virtual bool GetItemInfos(int item, int &states, unsigned long &tag,
					char * title, int titlesz, 
					char * imagefname, int imagefnamesz,
					int &ddsidx) = 0;
	/// associate a python script to a button : pIPython is IPython class from nv_rtPythonItf.h
	virtual void SetPythonInterface(void * pIPython) = 0;
	virtual bool SetItemPythonScriptEvent(int item, LPCSTR script) = 0;
	virtual LPCSTR GetItemPythonScriptEvent(int item) = 0;
};


#endif

