/*----------------------------------------------------------------------------------*/ /**

\file nv_GenericTools.h

modhelper is made to help defining the basis for a modular framework using rtModules.

- if you want the app to not use the modules, just define \b NO_RTMODULE
- if you want the plugs to not be used, just define \b NO_PLUGS
.

depending on the system, the defines may do something different. For systems which don't
have any module or plugs library, define \b NO_RTMODULE or \b NO_PLUGS...

\remarks define \c MODHELPER_SINGLE_FILE one time in one of your source code to allow implementation
of some functions (in modhelper.inl)
\code
#define MODHELPER_SINGLE_FILE
#include "modhelper.h"
\endcode

**/ //----------------------------------------------------------------------------------

#ifndef __HELPERSTOOLKIT__
#define __HELPERSTOOLKIT__

#pragma message("Using nv_GenericTools. More details at http://nvwebs/tristan/GUI/index.html")
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

//# define NO_RTMODULE
//# define NO_PLUGS

#ifndef NO_RTMODULE
# if _MSC_VER <= 1200
# 	pragma message("Note: for VC6, linking with nv_GenericTools_vc6.lib\n")
# 	pragma comment(lib,"nv_GenericTools_vc6.lib")
# else
//# 	pragma message("Note: for .NET, linking with nv_GenericTools.lib\n")
# 	pragma comment(lib,"nv_GenericTools.lib")
# endif
#endif

#include <stdio.h> // for _rtsimplelog(FILE *fd, const char * fmt, ...)
#include <conio.h>

#ifndef NO_PLUGS
# include "nv_GenericTools/nv_plug/plug.h"
#endif
#ifndef NO_RTMODULE
# include <map>
# include <string>
# include "nv_GenericTools/nv_rtmodules/nv_rtWindowsitf.h"
# include "nv_GenericTools/nv_rtmodules/nv_rtPythonItf.h"
#endif
#ifndef NO_CURVES
# include "nv_GenericTools/nv_curves/curvereader.h"
#endif

#ifndef NO_CURVES
extern CurvePool mycurves;
#endif


/** \name Log message type for LOG_MSG macro
 **/
//@{
#define LOG_MSG 0
#define LOG_WARN 1
#define LOG_ERR 2
#define LOG_STATE 3
#define LOG_NET 4
#define LOG_SND 5
#define LOG_EVENT 6
#define LOG_FUNC 7
//@}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Use modules (dll's) for windows & scripting
//
#ifndef NO_RTMODULE

#pragma message("Using MODULES.")

extern IRTFactory *g_dbgFactory;
extern IRTFactory *g_dlgFactory;
extern IRTFactory *g_pythonFactory;

extern SmartPtr<IDlgLog> spitfLogWin;
extern SmartPtr<IPython> spPython;
extern SmartPtr<IConsoleWnd> spitfConsole;
extern SmartPtr<IProgressBar> spitfProgress;
extern SmartPtr<IContainerWnd> spItfTabContainer; // used to embed the main window and some others
extern IWndMsgHandler* spItfMsgHandler;

extern void opendlls(LPCSTR ui, LPCSTR mainwndtitle);
extern void UnpackWindows();
extern void PackWindows(LPCSTR mainwndtitle, int x, int y, int w, int h);
extern bool CreateWindowContainer(const char *name,int x,int y,int w,int h,const char *classname, const char *parent);
extern bool DestoyWindow(const char *name);
extern bool EmptyWindow(const char *name);
extern bool AppendWindow(LPCSTR winname, LPCSTR winparentname);

extern void _rtsimplelog(int level, const char * fmt, ...);
extern void _rtsimplelog(FILE *fd, const char * fmt, ...);
extern void closedlls();
extern void SetDefaultUI(bool bOver3D);
extern bool savescript(const char *n);
extern void opendlls(const char *ui, const char *mainwndtitle);
extern bool loadsetupscript(const char *fname);
extern bool loadbasicobjects();
extern bool MsftMsgProc( unsigned long hWnd, unsigned int uMsg, unsigned int wParam, unsigned long lParam );
extern long OnRender(float fElapsedTime);
extern void SetPosition(LPCSTR win, int x, int y, int width, int height);

extern bool CreateFloatVecControl(const char *name, const char *parentname, void *vecptr, plug::PlugType pt);

extern bool CreateValueControl(const char *name, const char *parentname, float *scalarval, float min, float max);
extern bool CreateValueControl(const char *name, const char *parentname, int *scalarval, float min, float max);
extern bool CreateValueControl(const char *name, const char *parentname, char *scalarval, float min, float max);
extern bool CreateValueControl(const char *name, const char *parentname, bool *scalarval, float min, float max);

extern bool CreateButtonControl(const char *name, const char *parentname, IEventsWnd *nofification_itf, unsigned int tag1, unsigned int tag2);
extern bool CreateCheckControl(const char *name, const char *parentname, float *val);
extern bool CreateCheckControl(const char *name, const char *parentname, int *val);
extern bool CreateCheckControl(const char *name, const char *parentname, char *val);
extern bool CreateCheckControl(const char *name, const char *parentname, bool *val);

extern bool CreateStringControl(const char *name, const char *parentname, void *strptr, const char *filebrowsefilter);
extern bool CreateColorControl(const char *name, const char *parentname, void *colptr);

extern bool CreateComboControl(const char *name, const char *parentname, float *strptr);
extern bool CreateComboControl(const char *name, const char *parentname, int *strptr);
extern bool CreateComboControl(const char *name, const char *parentname, unsigned int *strptr);
extern bool CreateComboControl(const char *name, const char *parentname, unsigned long *strptr);
extern bool CreateComboControl(const char *name, const char *parentname, char *strptr);
extern bool CreateComboControl(const char *name, const char *parentname, bool *strptr);

extern bool CreateComboItem(const char *item, const char *cbname, unsigned long tag);
extern bool EraseComboItems(const char *cbname);

extern bool CreateRadioControl(const char *name, const char *parentname, float *strptr);
extern bool CreateRadioControl(const char *name, const char *parentname, int *strptr);
extern bool CreateRadioControl(const char *name, const char *parentname, char *strptr);
extern bool CreateRadioControl(const char *name, const char *parentname, bool *strptr);

extern bool CreateRadioItem(const char *cbname, const char *item, unsigned long tag);
extern bool EraseRadioItems(const char *cbname);

extern bool CreateToolbar(const char *name, const char *parentname, bool vertical, int x,int y,int w,int h);
extern bool CreateToolbarItem(const char *tbname, const char *item, int *ptr, unsigned long tag);
extern bool CreateToolbarItem(const char *tbname, const char *item, bool *ptr, unsigned long tag);
extern bool CreateToolbarItem(const char *tbname, const char *item, char *ptr, unsigned long tag);
extern bool CreateToolbarItem(const char *tbname, const char *item, unsigned long tag);
extern bool Control_NotifyMe(const char *name, IEventsWnd *pclient, unsigned long tag1, unsigned long tag2);
//
/// an Empty implementation of events
//
class CEmptyEventImpl : public IEventsWnd
{
public:
	virtual bool rtMouseMove(IWindow *, float mx, float my, unsigned int buttons, unsigned long lval){return false;};
	virtual void rtDataUpdated(IWindow *, unsigned long types){};
	virtual bool rtWindowClosed(IWindow *){return false;};
	virtual bool rtWindowDestroyed(IWindow *){return false;};
	virtual bool rtDestroyingWindow(IWindow *){return false;};
	virtual bool rtWindowResized(IWindow *, int cx, int cy){return false;};
  virtual bool rtWindowRepaint(IWindow *){return false;};
	// used by controls to send events in values changes:
	// ctl_bool, ctl_color, ctl_combo, ctl_scalar, ctl_string, ctl_vector
	// return 0 if nothing changed by this event. 1 if we changed something in valptr.
  //
  // Actually this event is called as follow :
  // Button :     rtCtrlEvent(object, "button", 0, 0, luser1, luser2 );
  // Checkbox :   rtCtrlEvent(object, "bool", (bool*)check_state_ptr, (float)check_state, luser1, luser2 );
  // Colordlg :   rtCtrlEvent(object, "color", (float*)color_vec_ptr, 0, luser1, luser2 );
  // Combodlg :   rtCtrlEvent(object, "combo", (int*)item_number_ptr, item_data[item_number], luser1, luser2 );
  // Scalardlg :  rtCtrlEvent(object, "scalar", (float*)value_ptr, value, luser1, luser2 );
  // Stringdlg :  rtCtrlEvent(object, "string", (char*)string_ptr, 0, luser1, luser2 );
  // -=ARGL...=- Toolbardlg : rtCtrlEvent(object, "toolbar", item_number, state_item[item_number], tag_item[item_number], luser1 );
  // Vectordlg :  rtCtrlEvent(object, "vector", vec_ptr, 0, luser1, luser2 );
	virtual int rtCtrlEvent(IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2 )
  {
    return 0;
  }
};
//
// Some storage of the windows. Simplified the management of the windows you created
//
struct MODHELPER_ltstr
{
  bool operator()(const std::string s1, const std::string s2) const
  {
	return strcmp(s1.c_str(), s2.c_str()) < 0;
  }
};
typedef std::map<std::string, SmartPtr<IWindow>, MODHELPER_ltstr> WndRepositoryType;
extern WndRepositoryType *wndrepository;

//
// INITIALIZATION
//
#define OPENMODULES(ui) opendlls(ui, NULL)
#define CLOSEMODULES() closedlls()
#define SETDEFAULTUI(bOver3D) SetDefaultUI(bOver3D)
//
// LOGGING
//
#define LOGMSG _rtsimplelog
#define LOGMSGSHOW() if(spitfLogWin) spitfLogWin->Show()
#define LOGMSGHIDE() if(spitfLogWin) spitfLogWin->Hide()
#define SETLOGWNDPOS(x,y,w,h) if(spitfLogWin) spitfLogWin->SetPosition(x,y,w,h);
#define LOGWNDNAME "logwnd"
//
// CONSOLE
//
#define SETCONSOLEWNDPOS(x,y,w,h) if(spitfConsole) spitfConsole->SetPosition(x,y,w,h);
#define CONSOLESHOW() if(spitfConsole) spitfConsole->Show()
#define CONSOLEHIDE() if(spitfConsole) spitfConsole->Hide()
//
// PROGRESSBAR
//
#define PROGRESSMSG(msg) if(spitfProgress) spitfProgress->SetMessage(msg);\
							else  fprintf(stderr, "progressmsg : %s\n", msg);
#define PROGRESSINC(i) if(spitfProgress) spitfProgress->AddPercent(i);\
							else fprintf(stderr, ".");
#define PROGRESSSHOW() if(spitfProgress) spitfProgress->Show()
#define PROGRESSHIDE() if(spitfProgress) spitfProgress->Hide()
#define PROGRESSSET(p) if(spitfProgress) spitfProgress->SetPercent(p);\
							else fprintf(stderr, "%.1f\r", p);
//
// SCRIPTING
//
#define LOADSETUPSCRIPT(n) loadsetupscript(n)
#define SAVESETUPSCRIPT(n) savescript(n)
#define LOGSCRIPTRESULT() LOGMSG(LOG_MSG, spPython ? spPython->GetStringResult() : "No Python object !");
#define EXECUTESCRIPT if(spPython) spPython->ExecuteString
//
// WINDOW 'CONTAINERS'
//
#define CREATEWINDOWCONTAINER(name,x,y,w,h,parent) CreateWindowContainer(name,x,y,w,h,"simplecontainer",parent)
#define CREATEWINDOWCONTAINER_TAB(name,x,y,w,h,parent) CreateWindowContainer(name,x,y,w,h,"tabcontainer",parent)
#define CREATEWINDOWCONTAINER_SPLIT(name,x,y,w,h,parent) CreateWindowContainer(name,x,y,w,h,"splittercontainer",parent)
#define CREATEWINDOWCONTAINER_SPLITH(name,x,y,w,h,parent) CreateWindowContainer(name,x,y,w,h,"splittercontainerh",parent)
#define CREATEWINDOWCONTAINER_FOLDING(name,x,y,w,h,parent) CreateWindowContainer(name,x,y,w,h,"foldingcontainer",parent)
//
// CONTROLS FOR 1D...4D VALUES
//
#define CREATECTRL_VALUE(name, parentname, ptr, min, max) CreateValueControl(name, parentname, ptr, min, max);
#define CREATECTRL_VEC2(name, parentname, ptr) CreateFloatVecControl(name, parentname, ptr, plug::PLUG_FLOATVEC2);
#define CREATECTRL_VEC3(name, parentname, ptr) CreateFloatVecControl(name, parentname, ptr, plug::PLUG_FLOATVEC3);
#define CREATECTRL_VEC4(name, parentname, ptr) CreateFloatVecControl(name, parentname, ptr, plug::PLUG_FLOATVEC4);
//
// CHECKBOX CTRL
//
#define CREATECTRL_CHECK(name, parentname, ptr) CreateCheckControl(name, parentname, ptr);
#define CTRL_CHECKBOX_CB2(name, l, action1, action2, tag1_, tag2_)\
  class CEvent_##l : public CEmptyEventImpl\
  {\
    virtual int rtCtrlEvent(IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2 )\
      { if(val == 0.0f) { action1; } else { action2; } return 0; }\
  };\
  static CEvent_##l s_event_##l;\
  Control_NotifyMe(name, &s_event_##l, tag1_, tag2_);
#define CTRL_CHECKBOX_CB1(name, l, action1, action2, tag1_, tag2_) CTRL_CHECKBOX_CB2(name, l, action1, action2, tag1_, tag2_)
#define CTRL_CHECKBOX_CB(name, action1, action2, tag1_, tag2_) CTRL_CHECKBOX_CB1(name,__COUNTER__, action1, action2, tag1_, tag2_)
#define CREATECTRL_CHECK_CB(name, parentname, ptr, action1, action2, tag1_, tag2_)\
  CreateCheckControl(name, parentname, ptr);\
  CTRL_CHECKBOX_CB(name, action1, action2, tag1_, tag2_);
//
// BUTTON CTRL
//
#define CREATECTRL_BUTTON(name, parentname) CreateButtonControl(name, parentname, NULL, 0, 0);
#define CREATECTRL_BUTTON_NOTIFYME(name, parentname, nofification_itf, tag1, tag2) CreateButtonControl(name, parentname, nofification_itf, tag1, tag2);
#define CREATECTRL_BUTTON_CB2(name, l, parentname, action, tag1_, tag2_)\
  class CEvent_##l : public CEmptyEventImpl\
  {\
    virtual int rtCtrlEvent(IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2 )\
      { action; return 0;}\
  };\
  static CEvent_##l s_event_##l;\
  CreateButtonControl(name, parentname, &s_event_##l, tag1_, tag2_);
#define CREATECTRL_BUTTON_CB1(name, l, parentname, action, tag1_, tag2_) CREATECTRL_BUTTON_CB2(name, l, parentname, action, tag1_, tag2_)
#define CREATECTRL_BUTTON_CB(name, parentname, action, tag1_, tag2_) CREATECTRL_BUTTON_CB1(name,__COUNTER__, parentname, action, tag1_, tag2_)
//
// STRING/FILENAME/COLOR CTRL
//
#define CREATECTRL_STRING(name, parentname, ptr) CreateStringControl(name, parentname, ptr, NULL);
#define CREATECTRL_FILENAME(name, parentname, ptr, filter) CreateStringControl(name, parentname, ptr, filter);
#define CREATECTRL_COLOR(name, parentname, ptr) CreateColorControl(name, parentname, ptr);
//
// COMBOBOX CTRL
//
#define CREATECTRL_COMBO(name, parentname, ptr) CreateComboControl(name, parentname, ptr);
#define ERASECOMBOITEMS(name) EraseComboItems(name);
#define CREATECOMBOITEM(name, parentname, tag) CreateComboItem(name, parentname, (unsigned long)tag);
//
// RADIO BUTTONS CTRL
//
#define CREATECTRL_RADIO(name, parentname, ptr) CreateRadioControl(name, parentname, ptr);
#define ERASERADIOITEMS(name) EraseRadioItems(name);
#define CREATERADIOITEM(name, parentname, tag) CreateRadioItem(name, parentname, (unsigned long)tag);
//
// TOOLBAR CTRL
//
#define CREATETOOLBAR_H(name, parentname, x,y,w,h) CreateToolbar(name, parentname, false, x,y,w,h)
#define CREATETOOLBAR_V(name, parentname, x,y,w,h) CreateToolbar(name, parentname, true, x,y,w,h)
#define CREATETOOLBARITEM_CHECK(tbname, item, ptr, tag) CreateToolbarItem(tbname, item, ptr, tag)
#define CREATETOOLBARITEM(tbname, item, tag) CreateToolbarItem(tbname, item, tag)
//
// WINDOWS NOTIFICATION
//
#define CTRL_NOTIFYME(name, pclient, tag1, tag2) Control_NotifyMe(name, pclient, tag1, tag2)
#define CTRL_NOTIFYME_CB2(name, l, action, tag1_, tag2_)\
  class CEvent_##l : public CEmptyEventImpl\
  {\
    virtual int rtCtrlEvent(IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2 )\
      { action; return 0; }\
  };\
  static CEvent_##l s_event_##l;\
  Control_NotifyMe(name, &s_event_##l, tag1_, tag2_);
#define CTRL_NOTIFYME_CB1(name, l, action, tag1_, tag2_) CTRL_NOTIFYME_CB2(name, l, action, tag1_, tag2_)
/** create an event and write the code in the define itself.
Arguments to use are : IWindow *ctrl, const char *ctrltype, unsigned long valptr, float val, unsigned long tag1, unsigned long tag2
 **/
#define CTRL_NOTIFYME_CB(name, action, tag1_, tag2_) CTRL_NOTIFYME_CB1(name,__COUNTER__, action, tag1_, tag2_)
//
// WINDOWS-GENERIC CALLS
//
#define PACKWINDOWS(n, x, y, w, h) PackWindows(n, x, y, w, h)
#define UNPACKWINDOWS() UnpackWindows()
#define APPENDWINDOW(name, parentname) AppendWindow(name, parentname)

#define DESTROYWINDOW(name) DestoyWindow(name)
#define EMPTYWINDOW(name) EmptyWindow(name)
#define SHOWWINDOW(name, bYes) {\
  if(wndrepository){ WndRepositoryType::iterator iwnd;\
  iwnd = wndrepository->find(name);\
  if((iwnd != wndrepository->end()) && (iwnd->second)) { iwnd->second->Visible(bYes);}\
  else {LOGMSG(LOG_WARN, "SHOWWINDOW: Window %s not found", name); } } }
#define MINIMIZEWINDOW(name, bYes) {\
  if(wndrepository){ WndRepositoryType::iterator iwnd;\
  iwnd = wndrepository->find(name);\
  if((iwnd != wndrepository->end()) && (iwnd->second)) { iwnd->second->Minimize(bYes);}\
  else {LOGMSG(LOG_WARN, "MINIMIZEWINDOW: Window %s not found", name); } } }
#define SETPOSITION(win, x, y, w, h) SetPosition(win, x, y, w, h)
//
// WINDOWS MESSAGING/MANAGEMENT
//
#define ON_RENDER_WINDOWS(t) OnRender(t)
#define	CREATEUIRESOURCES(w,d) (spItfMsgHandler ? spItfMsgHandler->CreateUIResources(w,d) : 0)
#define	RESETUIRESOURCES() (spItfMsgHandler ? spItfMsgHandler->ResetUIResources() : 0)
#define	LOSTUIRESOURCES() (spItfMsgHandler ? spItfMsgHandler->LostUIResources() : 0)
#define	DESTROYUIRESOURCES() (spItfMsgHandler ? spItfMsgHandler->DestroyUIResources() : 0)
// Simplest solution : just relay the Msft windows messages thanks to this call
#define MSFTMSGPROC_WINDOWS(hWnd, uMsg, wParam, lParam ) MsftMsgProc( hWnd, uMsg, wParam, lParam )
// These 3 calls are intended to be used ONLY when nothing is available to relay the Windows messages
// Here we catch by ourselves the messages dedicated to the windows (Peeking)
#define MSGPROC_WINDOWS_LOOP() (spItfMsgHandler ? spItfMsgHandler->HandleMessageLoop_Blocking() : 0)
#define MSGPROC_WINDOWS_ONEPASS() (spItfMsgHandler ? spItfMsgHandler->HandleMessageLoop_OnePass() : 0)
#define MSGPROC_WINDOWS_RETURNONKEY() (spItfMsgHandler ? spItfMsgHandler->HandleMessageLoop_ReturnOnKeyPressed() : _getch())

//
// PLUG-SPECIFIC CALLS FOR WINDOWS
//
#ifndef NO_PLUGS
inline plug::IPlug *_get_ctrl_val(LPCSTR name)
{
  WndRepositoryType::iterator iwnd;
  if(!wndrepository) return NULL;
  iwnd = wndrepository->find(name);
  if((iwnd != wndrepository->end()) && (iwnd->second)) 
  { if(iwnd->second->GetPlug(0)) return (plug::IPlug *)iwnd->second->GetPlug(0); 
  else return NULL;}
  else {LOGMSG(LOG_WARN, "_get_ctrl_val: Window %s not found", name); return NULL;} 
}
# define GETCONTROL_PLUG_VAL(ctrlname) _get_ctrl_val(ctrlname)
# define GETCONTROL_PLUG_MIN(ctrlname) NULL
# define GETCONTROL_PLUG_MAX(ctrlname) NULL
# define SETCTRL_VALUE(name, val)\
{\
  plug::IPlug *p = _get_ctrl_val(name);\
  if(p){ p->setValue val; }\
}
# define REGISTERPLUG(p) \
	if(!spPython) opendlls(NULL, NULL);\
	if(spPython) spPython->RegisterPlug((void*)p, NULL);
#else
# define REGISTERPLUG(p)
#endif // NO_PLUGS

#else //NOT using RTMODULES
/////////////////////////////////////////////////////////////////////////////////////////////////////
// 
/// \name INITIALIZATION
//@{
#ifndef NO_PLUGS
/// instanciates the dll's and create the basic objects. ui can be NULL if we use system ui, "opengl"/"d3d" for custom ui
# define OPENMODULES(ui) {\
 plugrepository = new plug::PlugContainer;\
 fprintf(stderr,"no module for this system\n"); }
///< Delete the basic objects and release the dll's
# define CLOSEMODULES() {\
  plugrepository = NULL;\
  plugmap.clear();\
  mycurves.clear();}
#else
# define OPENMODULES(ui) {\
 fprintf(stderr,"no module for this system\n"); }
///< Delete the basic objects and release the dll's
# define CLOSEMODULES()
#endif
#define SETDEFAULTUI(bOver3D) ///< default UI is 3D or typical system windows ?
//@}
/// \name LOGGING
//@{
#define LOGMSG _rtsimplelog	///< Log a message with a type (LOG_MSG, LOG_WARN, LOG_ERR...). Use this function instead of fprintf(stdin/err...). This function will behave like print/fprintf if nv_rtWindows.dll isn't available
#define LOGMSGSHOW()			///< show the Log window 
#define LOGMSGHIDE()			///< hide the Log window
//@}
/// \name CONSOLE
//@{
#define SETCONSOLEWNDPOS(x,y,w,h) ///< set the position of the console window
#define CONSOLESHOW()		///< show the Console window (for Python)
#define CONSOLEHIDE()		///< hide the Console window (for Python)
//@}
/// \name PROGRESSBAR
//@{
#define PROGRESSMSG(m) fprintf(stderr, "progressmsg : %s\n", m); ///< set the message for the progress bar.
#define PROGRESSINC(i)  fprintf(stderr, "."); ///< increment the progressbar with i
#define PROGRESSSHOW()		///< show the progress bar
#define PROGRESSHIDE()		///< hide the progress bar
#define PROGRESSSET(f)  fprintf(stderr, "%.1f\r", (float)f); ///< set the progressbar value to f
//@}
/// \name SCRIPTING
//@{
#define LOADSETUPSCRIPT(n)	///< Load a Python script
#define SAVESETUPSCRIPT(n) _rtbdummy(n) ///< save python definitions (windows, plug connections...) as a Python script in a file
#define LOGSCRIPTRESULT()	///< Log the result of the previous Python script run
#define EXECUTESCRIPT _rtdummy ///< Execute a Python Script
//@}

/** \name CONTAINER WINDOWS

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
 **/
//@{
#define CREATEWINDOWCONTAINER(name,x,y,w,h,parent)			///< Creates a normal window : a container for various other containers or controls
#define CREATEWINDOWCONTAINER_TAB(name,x,y,w,h,parent)		///< Creates a Tab container : a container for various other containers or controls stored in Tabs.
#define CREATEWINDOWCONTAINER_SPLIT(name,x,y,w,h,parent)		///< Creates a Splitter container : a container for various other containers or controls separated with split bars.
#define CREATEWINDOWCONTAINER_SPLITH(name,x,y,w,h,parent)	///< Creates a Splitter container : a container for various other containers or controls separated with vertical split bars.
#define CREATEWINDOWCONTAINER_FOLDING(name,x,y,w,h,parent)	///< Creates a Folding container : a container for various other containers or controls.	You can fold/Unfold this container by clicking in the check box made for this.
//@}
/** \name CONTROLS FOR 1D...4D VALUES

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
	\arg ptr is a pointer to a variable in your code
	\arg min/max to set the slider's scale
 **/
//@{
#define CREATECTRL_VALUE(name, parentname, ptr, min, max)
#define CREATECTRL_VEC2(name, parentname, ptr)	///< create a 2D vector control.
#define CREATECTRL_VEC3(name, parentname, ptr) ///< create a 3D vector control.
#define CREATECTRL_VEC4(name, parentname, ptr) ///< create a 4D vector control.
//@}
/** \name CONTROLS FOR 1D...4D VALUES

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
 **/
//@{
#define CREATECTRL_CHECK(name, parentname, ptr) ///< create a check box connected to a \c bool variable. \c ptr is pointing to the variable you want to attach
/// declares 2 actions for checked and unchecked cases. Just write your C++ code in \c action1 and \c action2. Considere arguments as what you get in IEventsWnd::rtCtrlEvent
#define CTRL_CHECKBOX_CB(name, action1, action2, tag1_, tag2_) CTRL_CHECKBOX_CB1(name,__COUNTER__, action1, action2, tag1_, tag2_)
/// create a checkbox and declares 2 actions for checked and unchecked cases. Just write your C++ code in \c action1 and \c action2. Considere arguments as what you get in IEventsWnd::rtCtrlEvent
#define CREATECTRL_CHECK_CB(name, parentname, ptr, action1, action2, tag1_, tag2_)
//@}
/** \name STRING/FILENAME/COLOR CONTROLS

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
 **/
//@{
#define CREATECTRL_STRING(name, parentname, ptr) ///< Create a string control connected to a sapi::String
#define CREATECTRL_FILENAME(name, parentname, ptr, filter) ///< Create a String control for filename. Adds a browse button.
#define CREATECTRL_COLOR(name, parentname, ptr) ///< Create a color control, providing the picking color control of MFC
//@}
/** \name COMBO-BOX CONTROL

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
	\arg ptr is a pointer to a variable in your code
 **/
//@{
#define CREATECTRL_COMBO(name, parentname, ptr) ///< Create a combo box control connected to \c unsigned \c long variable pointer
#define ERASECOMBOITEMS(name)				///< Erase all the combo items of 'name'.
#define CREATECOMBOITEM(name, parentname, tag) ///< Append a new item 'name' to the combo 'parentname'. tag will be the value copied in the \c unsigned \c long variable connected to the combo.
//@}
/** \name 'RADIO' CONTROL. Should add some more commands for items mgt, here.

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
	\arg ptr is a pointer to a variable in your code
 **/
//@{
#define CREATECTRL_RADIO(name, parentname, ptr) ///< create a Radio control
#define ERASERADIOITEMS(name) ///< erase items
#define CREATERADIOITEM(name, parentname, tag) ///< append a radio item
//@}
/** \name TOOLBAR CONTROL

	\arg parent argument is the name of the parent container. If not parent, set it to NULL.
	\arg name argument is the name of the new created control
	\arg ptr is a pointer to a variable in your code

	\todo Need To Implement Event Catching For Various Items.
 **/
//@{
#define CREATETOOLBAR_H(name, parentname, x,y,w,h) ///< Create a horizontal Toolbar
#define CREATETOOLBAR_V(name, parentname, x,y,w,h) ///< Create a vertical Toolbar
#define CREATETOOLBARITEM_CHECK(tbname, item, ptr, tag) ///< Create a checkbox item. \c ptr can be int, bool or char type.
#define CREATETOOLBARITEM(tbname, item, tag) ///< Create a TB item not connected to any variable. You will use the events (CTRL_NOTIFYME) to catch this.
//@}
/** \name GENERIC NOTIFICATION
 **/
//@{
#define CTRL_NOTIFYME(name, pclient, tag1, tag2) ///< ask an object 'name' (window, control) to send notifications to \c pclient. tag1 & tag2 will be sent to the events. \c pclient is an implementation of IEventsWnd
/// declare an action for the window/control \c name. Just write your C++ code in \c action. Considere arguments as what you get in IEventsWnd::rtCtrlEvent
#define CTRL_NOTIFYME_CB(name, action, tag1_, tag2_)
//@}
/** \name WINDOWS-GENERIC CALLS
 **/
//@{
#define PACKWINDOWS(n, x, y, w, h) ///< easy call to pack basic windows like Console, logging... to see if we keep it.
#define UNPACKWINDOWS()
#define APPENDWINDOW(name, parentname) ///< append an existing window to a parent window container
#define DESTROYWINDOW(name) ///< Destroys a window/control. Note that this won't destroy the plugs related to the variable (ptr argument)
#define SHOWWINDOW(name, bYes) ///< Show the window if it was hidden
#define MINIMIZEWINDOW(name, bYes) ///< minimize the window. Not working if the window is nested into another
#define SETPOSITION(win, x, y, w, h) ///< set the position of the window in the screen. Not working if this window is nested into another
//@}
/** \name RESOURCE MANAGEMENT AND RENDERING CALL

	Use these functions when you are taking care of the windows events by yourself. You'll have to call these functions at various places depending on what
	messages you got. See the main documentation page for details.

	windows need some resources to be available for example if it is an overlay of 3D : textures must be loaded...

	There is also various ways to transmit the messages to the windows, depending on how we integrated them into the app.

 **/
//@{
#define ON_RENDER_WINDOWS(t) ///< call this to render the windows. Of course this will do something only when you are renderin the windows as an overlay of your 3D. MFC (for example) doesn't care.
#define	CREATEUIRESOURCES(w,d) ///< call this when your app just started up and that you are allocating some basic resources
#define	RESETUIRESOURCES() ///< call this when the app decided to reset the resources (resolution change for example)
#define	LOSTUIRESOURCES() ///< call this when you lost the control over the application's resources. D3D-tytpical case...
#define	DESTROYUIRESOURCES() ///< call this when the app wants to detroy the resources.
//@}
/** \name TYPICAL WINDOWS MESSAGING

	There are various ways to transmit the messages to the windows, depending on how we integrated them into the app.
 **/
//@{
#define MSFTMSGPROC_WINDOWS(hWnd, uMsg, wParam, lParam ) ///< Simplest solution : just relay the Msft windows messages thanks to this call
// These 3 calls are intended to be used ONLY when nothing is available to relay the Windows messages
// Here we catch by ourselves the messages dedicated to the windows (Peeking)
#define MSGPROC_WINDOWS_LOOP() ///< takes the control over the windows messaging loop. Does not return. It will loop for you.
#define MSGPROC_WINDOWS_ONEPASS() ///< handles one single windows-messaging loop and returns. Good if you want to do some work in between
#define MSGPROC_WINDOWS_RETURNONKEY() ///< handle the windows messaging loop but returns when a key is pressed. Cool for simple apps.
//@}
/** \name PLUG-SPECIFIC CALLS FOR WINDOWS

	Controls have Plugs attached to them. This allow you to easily connect variable to them so that variable will update the control and vice versa.
 **/
//@{
#define GETCONTROL_PLUG_VAL(name) NULL///< returns the plug for the \e main value. Value meaning depends on what kind of control you are talking to.
#define GETCONTROL_PLUG_MIN(name) NULL///< returns the plug for the min boundary. A control for scalar value has this.
#define GETCONTROL_PLUG_MAX(name) NULL///< returns the plug for the max boundary. A control for scalar value has this.
#define SETCTRL_VALUE(name, val) ///< sets the value of the main Control's \c Plug. \warning val is in () : (1.0) or (1,2,0,1)...
#define REGISTERPLUG(p) ///< Register a plug::Plug into Python scripting. So that you can use it in a script.
//@}


inline void _rtdummy(...) { }
inline bool _rtbdummy(...) { return true; }

inline void _rtsimplelog(int level, const char * fmt, ...)
{
  char *msglevel;
  va_list  vlist;
  va_start(vlist, fmt);
  switch(level)
  {
  case LOG_WARN:
    msglevel = "warning>>";
    break;
  case LOG_ERR:
    msglevel = "error>>";
    break;
  case LOG_STATE:
    msglevel = "state>>";
    break;
  case LOG_NET:
    msglevel = "net>>";
    break;
  case LOG_SND:
    msglevel = "sound>>";
    break;
  case LOG_EVENT:
    msglevel = "event>>";
    break;
  case LOG_FUNC:
    msglevel = "func>>";
    break;
  case LOG_MSG:
  default:
    msglevel = 0;
    break;
  }
  va_list va = vlist;
  char *args[10];
  for(int i=0; i<10; i++)
    args[i] = va_arg(va,char*);
  if(msglevel)
    fprintf(stderr, "%s : ", msglevel);
  fprintf(stderr, fmt, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
  fprintf(stderr, "\n");
  va_end(vlist);
  return;
}


#endif // NV_RTMODULES

#ifndef NO_PLUGS // USING PLUGS
/////////////////////////////////////////////////////////////////////////
//
// PLUG CREATION
//
#define CREATEPLUG_COMPOSITE(name, parentname) CreatePlugComposite(name, name, parentname)
// TODO...
//#define CREATEPLUG(parentname, ptr) CreatePlug(#ptr, parentname, &ptr)
#define CREATEPLUG_STRING(parentname, ptr) CreatePlug(#ptr, parentname, &ptr, plug::PLUG_STRING)
#define CREATEPLUG_FLOAT(parentname, ptr) CreatePlug(#ptr, parentname, &ptr, plug::PLUG_FLOAT)
#define CREATEPLUG_FLOATVEC3(parentname, ptr) CreatePlug(#ptr, parentname, &ptr, plug::PLUG_FLOATVEC3)
//
// FIND BACK A VARIABLE'S PLUG 
//
#define GET_PLUG_OF_PTR(ptr) FindPlugOfPtr(ptr)
//
// PLUG CONNECTION
//
inline void _connect_2_plugs(plug::IPlug *src,plug::IPlug *dst, bool twoways)
{
  if((!src)||(!dst))
  { LOGMSG(LOG_WARN, "_connect_2_plugs: 1 or 2 pointers are NULL"); return;}
  if(!src->connectTo(dst, twoways))
  { LOGMSG(LOG_WARN, "_connect_2_plugs: connection failed");}
}
#define CONNECT_PLUG_1WAY(src,dst) _connect_2_plugs(src,dst,false)
#define CONNECT_PLUG_2WAYS(src,dst) _connect_2_plugs(src,dst,true)
//
// FLUSHING THE PLUGS. PROPAGATE CHANGES
//
#define FLUSHPTR(ptr) FlushPtr((unsigned long)ptr) ///< Flush the related Plug of the variable passed as pointer. This helps to update all the connected Plugs. Call it when you changed the variable 'by hand' so that everybody knows it.
//
// CURVE CREATION MADE OF PLUGS
//
#ifndef NO_CURVES
#ifndef NO_RTMODULE
# define CV_INIT() mycurves.Init(spPython)
#else
# define CV_INIT() mycurves.Init(NULL)
#endif
#define CV_CREATE(n, dim) mycurves.newCV(n,dim)
#define CV_CREATE_FROM_FILE(n, fname) \
	if(!mycurves.newCVFromFile(n,fname))\
	{\
		LOGMSG(LOG_ERR,"error while reading curves from %s : %s", fname, mycurves.getLastError() );\
	}
#define CV_ADDKEY(n, frame, x,y,z,w)\
	{ ICurveVector *cv = mycurves.getCV(n);\
		if(cv) cv->addKey(frame, x,y,z,w); }
// floatptr MUST be a float variable. Nothing else : this is driving the curve, so we don't need anything else.
// ?? Is this clean ??
#define CV_CONNECT_IN(n, floatptr) cv_connect_in(n, floatptr, #floatptr)
//#	define CV_CONNECT_OUT(n, ptr, pt) cv_connect_out(n, (void *)ptr, pt)
#define CV_CONNECT_OUT1D(n, ptr) cv_connect_out(n, (void *)ptr, plug::PLUG_FLOAT, #ptr)
#define CV_CONNECT_OUT2D(n, ptr) cv_connect_out(n, (void *)ptr, plug::PLUG_FLOATVEC2, #ptr)
#define CV_CONNECT_OUT3D(n, ptr) cv_connect_out(n, (void *)ptr, plug::PLUG_FLOATVEC3, #ptr)
#define CV_CONNECT_OUT4D(n, ptr) cv_connect_out(n, (void *)ptr, plug::PLUG_FLOATVEC4, #ptr)

#define CV_DISCONNECT_IN(n, floatptr) cv_disconnect_in(n, floatptr, #floatptr)
#define CV_DISCONNECT_OUT(n, ptr) cv_disconnect_out(n, (void *)ptr, #ptr)

#define CV_DISCONNECT_ALL(n)\
	{	ICurveVector *cv = mycurves.getCV(n);\
		if(cv) {\
			plug::IPlug *pin = (plug::Plug*)cv->getPlugIn();\
			plug::IPlug *pout = (plug::Plug*)cv->getPlugOut();\
			if(pout) pin->disconnectAll();\
			if(pin) pin->disconnectAll();	} }
#endif // NO_CURVES
//
// some storage to simplify your use of plugs and containers.
//
#include <map>
typedef std::map<unsigned long, SmartPtr<plug::IPlug > > PlugMapType;
extern SmartPtr<plug::PlugContainer> plugrepository;
extern PlugMapType plugmap;

#ifndef NO_PLUGS
extern void FlushPtr(unsigned long ptr);
extern bool CreatePlugComposite(LPCSTR name, LPCSTR comptype, LPCSTR parentname);
extern plug::IPlug* CreatePlug(LPCSTR name, LPCSTR parentname, void * ptr, plug::PlugType type);
extern plug::IPlug* FindPlugOfPtr(void *ptrval);
extern plug::IPlug* CreatePlug_(const char *name, void *ptrval, plug::PlugType pt);
// TODO
//extern plug::IPlug* CreatePlug(const char *name, String *ptrval);
//extern plug::IPlug* CreatePlug(const char *name, float *ptrval);
extern bool cv_connect_in(LPCSTR cvname, float *variable, LPCSTR varname);
extern bool cv_connect_out(LPCSTR cvname, void *variable, plug::PlugType pt, LPCSTR varname);
extern bool cv_disconnect_in(LPCSTR cvname, float *variable, LPCSTR varname);
extern bool cv_disconnect_out(LPCSTR cvname, void *variable, LPCSTR varname);
#endif

//
// MORE ROOTS PLUG DECLARATION
//
#define DECL_PLUG(n) plug::Plug plug_##n;
#define DECL_PLUGCOMPOSITE(n) plug::Plug n;
#define DECL_PLUG_VEC3(n) plug::Plug plug_##n;\
	plug::Plug plug_x##n;\
	plug::Plug plug_y##n;\
	plug::Plug plug_z##n;
#define DECL_PLUG_VEC4(n) plug::Plug plug_##n;\
	plug::Plug plug_x##n;\
	plug::Plug plug_y##n;\
	plug::Plug plug_z##n;\
	plug::Plug plug_w##n;
#define DECL_PLUG_VEC2(n) plug::Plug plug_##n;\
	plug::Plug plug_x##n;\
	plug::Plug plug_y##n;
#define INIT_PLUG_FLOAT(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_FLOAT, &n, 1,cont,0,NULL,p,true);\
	if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_BYTE(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_BYTE, &n, 1,cont,0,NULL,p,true);\
		if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_INT(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_INT, &n, 1,cont,0,NULL,p,true);\
		if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_DWORD(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_DWORD, &n, 1,cont,0,NULL,p,true);\
		if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_BOOL(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_BOOL, &n, 1,cont,0,NULL,p,true);\
		if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_STRING(n,p,cont) plug_##n.Init(#n, 0, plug::PLUG_STRING, &n, 1,cont,0,NULL,p,true);\
		if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_VEC4(n,p,cont) \
	plug_##n.Init(#n, 0, plug::PLUG_FLOATVEC4, &n, 1,cont,0,NULL,p,true);\
	plug_x##n.Init("x", 0, plug::PLUG_FLOAT, &n[0], 1,cont,0,NULL,&plug_##n,true);\
	plug_y##n.Init("y", 0, plug::PLUG_FLOAT, &n[1], 1,cont,0,NULL,&plug_##n,true);\
	plug_z##n.Init("z", 0, plug::PLUG_FLOAT, &n[2], 1,cont,0,NULL,&plug_##n,true);\
	plug_w##n.Init("w", 0, plug::PLUG_FLOAT, &n[3], 1,cont,0,NULL,&plug_##n,true);\
	if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_VEC3(n,p,cont) \
	plug_##n.Init(#n, 0, plug::PLUG_FLOATVEC3, &n, 1,cont,0,NULL,p,true);\
	plug_x##n.Init("x", 0, plug::PLUG_FLOAT, &n[0], 1,cont,0,NULL,&plug_##n,true);\
	plug_y##n.Init("y", 0, plug::PLUG_FLOAT, &n[1], 1,cont,0,NULL,&plug_##n,true);\
	plug_z##n.Init("z", 0, plug::PLUG_FLOAT, &n[2], 1,cont,0,NULL,&plug_##n,true);\
	if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_VEC2(n,p,cont) \
	plug_##n.Init(#n, 0, plug::PLUG_FLOATVEC2, &n, 1,cont,0,NULL,p,true);\
	plug_x##n.Init("x", 0, plug::PLUG_FLOAT, &n[0], 1,cont,0,NULL,&plug_##n,true);\
	plug_y##n.Init("y", 0, plug::PLUG_FLOAT, &n[1], 1,cont,0,NULL,&plug_##n,true);\
	if((!p)&&(!cont)) { REGISTERPLUG(&plug_##n) }
#define INIT_PLUG_COMPOSITE(n, typestr, p,cont) \
	n.Init(#n, 0, plug::PLUG_COMPOSITE, NULL, 1,cont,0,#typestr, p, true);\
	if((!p)&&(!cont)) { REGISTERPLUG(&n) }
#define SETVALUE(n,v) plug_##n.setValue(v);  
#define SETVALUE2(n,v,v2) plug_##n.setValue(v,v2,0);
#define SETVALUE3(n,v,v2,v3) plug_##n.setValue(v,v2,v3,0); 
#define SETVALUE4(n,v,v2,v3,v4) plug_##n.setValue(v,v2,v3,v4,0); 
#define PLUGPTR(n) &plug_##n
#define FLUSH(n) plug_##n.flush(); 

#else // NOT USING PLUGS
/////////////////////////////////////////////////////////////////////////
//

/** \name plug::Plug class helpers.
Use these macros to easily create some plugs in your app.

DECL_PLUG... would be with other variables or as member of a class.

INIT_PLUG_... would be called at init time. 'n' is the plug object, 'p' is the variable ptr and 'cont' is the optional plug::PlugContainer

 **/
//@{
#define CREATEPLUG_COMPOSITE(name, parentname) ///< Create a Plug containing many other plugs
#define CREATEPLUG_STRING(parentname, ptr) ///< Create a String plug related to \c ptr as String
#define CREATEPLUG_FLOAT(parentname, ptr) ///< Create a Float Plug related to \c ptr as float*
#define CREATEPLUG_FLOATVEC3(parentname, ptr) ///< Create a 3D vector plug related to \c ptr as float[3]

#define GET_PLUG_OF_PTR(ptr) ///< Find back the Plug of a specific variable. \c ptr is the pointer of the variable

#define CONNECT_PLUG_1WAY(src,dst) ///< Connect src plug to dst plug in one single way
#define CONNECT_PLUG_2WAYS(src,dst) ///< Connect src plug to dst plug in both ways
//@}
/** \name Curve commands. 

	This helps to easily create some curves and keys. Then you would connect these curves to variable thanks to the \c PLUGS
**/
//@{
#ifndef NO_CURVES
#ifndef NO_RTMODULE
#	define CV_INIT() mycurves.Init(spPython) ///< Initialize the Curve management. This will prepare the curve repository.
#else
#	define CV_INIT() mycurves.Init(NULL)
#endif
#	define CV_CREATE(n, dim) mycurves.newCV(n,dim) ///< Create a curve named \c n of dimension \c dim. It can be 1D to 4D.
#	define CV_ADDKEY(n, frame, x,y,z,w) ///< add a key to the curve at a specific frame
/** Connect a float value to the entry of the curve. Most of the time it is 'time' (float).
	Floatptr MUST be a float variable. Nothing else : this is driving the curve, so we don't need anything else.

	Note that floatptr will get a Plug for itself (if not already available), then the curve will connect to it
**/
#	define CV_CONNECT_IN(n, floatptr)
//#	define CV_CONNECT_OUT(n, ptr)
#	define CV_CONNECT_OUT1D(n, ptr) ///< connect the 1D curve output to a variable. ptr will get a Plug so that the curve can connect to it.
#	define CV_CONNECT_OUT2D(n, ptr) ///< connect the 2D curve output to a variable. ptr will get a Plug so that the curve can connect to it.
#	define CV_CONNECT_OUT3D(n, ptr) ///< connect the 3D curve output to a variable. ptr will get a Plug so that the curve can connect to it.
#	define CV_CONNECT_OUT4D(n, ptr) ///< connect the 4D curve output to a variable. ptr will get a Plug so that the curve can connect to it.
#	define CV_DISCONNECT_IN(n, floatptr) ///< Disconnect the variable's Plug from the input of the curve. The plugs aren't discarded.
#	define CV_DISCONNECT_OUT(n, ptr) ///< Disconnect the variable's Plug from the output of the curve. The plugs aren't discarded.
#	define CV_DISCONNECT_ALL(n) ///< Isolate the curve by disconnecting every connected Plug.
#endif
//@}
/** \name Low level commands for Plugs

These defines are for low level Plug decalrations. You may not really need it.
**/
//@{
#	define DECL_PLUG(n)				///< declare a plug variable 'n'
#	define DECL_PLUG_VEC3(n)		///< declare a composite plug for float[3]
#	define DECL_PLUG_VEC4(n)		///< declare a composite plug for float[4]
#	define DECL_PLUG_VEC2(n)		///< declare a composite plug for float[2]
#	define INIT_PLUG_FLOAT(n,p,cont) ///< initialize the plug 'n' for a float variable 'p'. 'cont' is an optional plug::PlugContainer. C
#	define INIT_PLUG_BYTE(n,p,cont)		///< init a plug to connect with a BYTE pointed by 'p'
#	define INIT_PLUG_INT(n,p,cont)		///< init a plug to connect with a int pointed by 'p'
#	define INIT_PLUG_DWORD(n,p,cont)	///< init a plug to connect with a DWORD pointed by 'p'
#	define INIT_PLUG_BOOL(n,p,cont)		///< init a plug to connect with a bool pointed by 'p'
#	define INIT_PLUG_STRING(n,p,cont)	///< init a plug to connect with a sapi::String pointed by 'p'
#	define INIT_PLUG_VEC4(n,p,cont)		///< init a plug to connect with a float[4] pointed by 'p'
#	define INIT_PLUG_VEC3(n,p,cont)		///< init a plug to connect with a float[3] pointed by 'p'
#	define INIT_PLUG_VEC2(n,p,cont)		///< init a plug to connect with a float[2] pointed by 'p'
#	define SETVALUE(n,v)			n = v; ///< to assign a value to the plug. But will change...
#	define SETVALUE2(n,v,v2)		n[0] = v; n[1] = v2;
#	define SETVALUE3(n,v,v2,v3)		n[0] = v; n[1] = v2; n[2] = v3;
#	define SETVALUE4(n,v,v2,v3,v4)	n[0] = v; n[1] = v2; n[2] = v3; n[3] = v4; 
#	define PLUGPTR(n) NULL
#	define FLUSH(n)					///< Flush the plug: update all the connections
#	define String std::string
//@}
#endif

#endif

