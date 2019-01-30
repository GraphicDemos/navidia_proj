/** \file nv_rtPythonItf.h 
 **
 **/ /***********************************************************************/
#ifndef __Python_H__
#define __Python_H__

#define PYTHONITF_VERSION "1.15"

#include "sapi_smartptr_itf.h"

using namespace sapi;

class IPython;
/*************************************************************************/ /**
 **
 ** must be implemented by the client if it wants to get back events
 ** 
 **/ /*************************************************************************/
class IEventsPython
{
public:
	/// put your list of external cmds : e.###(...) etc
	virtual LPCSTR ListOfAvailableCmds(IPython *) { return NULL; };
	virtual bool OutputString(IPython *, LPCSTR) { return false; };
	virtual bool OutputError(IPython *, LPCSTR) { return false; };
	virtual bool FuncCall(LPCSTR funcname, 
		int strresultsz, char *strresult, 
		int nstrings, const char * const *sptr,  
		int nfloats, const float *fptr) 
		{ /*strcpy(strresult,"not implemented !");*/ return false; };
};
typedef enum
{
	PYSTDIN,
	PYSTDOUT,
	PYSTDERR
} PyPlugType;
/**
 **	Interface for Python system embedded into a rt module
 **/
class IPython
{
public:
	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	//virtual bool CompileString(LPCSTR fmt, ...) = 0; ///< compile a simple string
	virtual bool ExecuteString(LPCSTR fmt, ...) = 0; ///< execute a simple string
	virtual bool ExecuteString(bool scriptable, LPCSTR fmt, ...) = 0; ///< execute a simple string
	virtual bool ExecuteStringFromFile(LPCSTR fname, bool scriptable=true) = 0; ///< execute a simple string from a file
	virtual LPCSTR GetStringResult() = 0; ///< returns the last string generated by the script

	//virtual LPCSTR GetLastError() const = 0;
	//virtual LPCSTR GetLastResult() const = 0;
	//virtual void GetLastResultf1(float *pfloat) = 0;
	//virtual void GetLastResultf2(float *pfloat) = 0;
	//virtual void GetLastResultf3(float *pfloat) = 0;
	//virtual void GetLastResultf4(float *pfloat) = 0;

	virtual LPCSTR GetScriptForSetup() = 0;
	virtual bool SaveScriptForSetup(LPCSTR fname) = 0;

	virtual void Register(IEventsPython *pClient) = 0;
	virtual void UnRegister(IEventsPython *pClient=NULL) = 0;

	virtual void *GetPlug(PyPlugType) = 0;
	virtual void *GetPlug(LPCSTR objname) = 0; ///< to retrieve a plug of an object in Python
	virtual bool RegisterPlug(void *iplug, char * prefix=NULL) = 0;
	//virtual bool UnregisterPlug(void *iplug) = 0;

	virtual bool RegisterIWindowInterface(void *ptriwin, LPCSTR name) = 0; ///< using void to avoid dependency with IWindow.
	virtual void * GetIWindowInterface(LPCSTR windowname) = 0; ///< using void to avoid dependency with IWindow.
	virtual bool RegisterILoggingInterface(void *ptrilog, LPCSTR name) = 0;
	virtual void * GetILoggingInterface(LPCSTR loggingname) = 0;
	virtual bool RegisterICurveVector(void *ptricv, LPCSTR name) = 0; ///< register a ICurveVector* interface. ptricv is ICurveVector*
	virtual void * GetICurveVector(LPCSTR curvename) = 0; ///< return ICurveVector* interface
	virtual bool RegisterIPlugContainerInterface(void *ptr, LPCSTR name) = 0;
	virtual void * GetIPlugContainer(LPCSTR name) = 0;

  virtual LPCSTR GetIWindowsVersion() = 0; ///< important : to see if we can work without crashing ! Versions MUST match
};

#endif