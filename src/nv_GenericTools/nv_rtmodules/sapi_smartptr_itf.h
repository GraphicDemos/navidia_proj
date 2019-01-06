#ifndef __SMARTPTRITF_H__
#define __SMARTPTRITF_H__

#ifndef LPCSTR
	typedef const char *LPCSTR;
#endif
#ifndef NULL
#	define NULL 0
#endif

/*
 *	sapi encloses all definitions
 */
namespace sapi
{
/********************************************************************/ /**
 ** \file sapi_smartptr_itf.h 
 **
 ** Smart Reference Interface : when we need total abstraction.
 ** 
 ** \b Note : We added here QueryInterface() method to allow us to query for
 ** Other interfaces. This method is not so strict than the one we can see
 ** in Microsoft api. No needs : it should be enough.
 ** 
 **/
class ISmartRef
{
public:
	virtual int GetRef(void) = 0;
	virtual void AddRef(void) = 0;
	virtual void Release(void) = 0;
	virtual void SRDelete() = 0;

	virtual void *QueryInterface(LPCSTR ItfName=NULL) = 0;
};

#ifndef SMARTPTRDEFINED
#define SMARTPTRDEFINED
/** 
 ** Smart pointer class Template for declaration of a class derived from SmartRef or ISmartRef.
 ** 
 ** We can considere this template like the ComQIPtr<> : 
 ** it wraps the pointer deriving from ISmartRef or SmartRef and thus takes care on reference counters.
 **
 ** \todo Create a SmartQIPtr : the same but with interface query.
 **/
template <class T> class SmartPtr {
protected:
public:
    T* lpClass;
    SmartPtr()
	{ 
		lpClass=NULL;
	}
	/// Constructor used mainly by STL classes (!)
    SmartPtr(T* lpClass_)
	{ 
		if(lpClass_)
			lpClass_->AddRef();
		lpClass = lpClass_;
	}
	// We must be careful with this cast
    SmartPtr(const void* lpClass_)
	{ 
		if(lpClass_)
			((T*)lpClass_)->AddRef();
		lpClass = (T*)lpClass_;
	}
    SmartPtr(SmartPtr const &smart)
	{ 
		lpClass = smart.lpClass; 
		if(lpClass)
			lpClass->AddRef(); 
	}
    ~SmartPtr(void) 
	{
		if(lpClass)
			lpClass->Release(); 
	}
    //SmartPtr& operator[](int idx);
    inline operator T*(void) 
	{ 
		return lpClass; 
	}
    /*inline operator SmartRef*(void) 
	{ 
		return (SmartRef*)lpClass; 
	}*/
	// Ce cas de recopie du contenu ne devrait pas arriver
    //inline const T& operator[](int idx) const
	//{
	//	return lpClass[idx];
	//}
    //inline T& operator*(void) 
	//{ 
	//	return *lpClass; 
	//}
    inline T* operator[](int idx)
	{
		return &(lpClass[idx]);
	}
    T* operator->(void) 
	{ 
		return lpClass; 
	}
	/**
	 **	This helps to cast from nothing (void*) to T...
	 ** Caution : may cause troubles if cast has no sense !!
	 **/
    void AssignCast(const void * lpClass_, bool bAddref=false) 
	{
		if(lpClass == (T*)lpClass_)
			return;
		if(lpClass)
			lpClass->Release(); 
		lpClass = (T*)lpClass_; 
		if(lpClass && bAddref)
		{
			lpClass->AddRef(); 
		}
    }
    SmartPtr& operator=(T* lpClass_) 
	{
		if(lpClass == lpClass_)
			return *this;
		if(lpClass)
			lpClass->Release(); 
		lpClass = lpClass_; 
		if(lpClass)
		{
			lpClass->AddRef(); 
		}
		return *this;
    }
    SmartPtr& operator=(SmartPtr const &smart) 
	{
		if(lpClass == smart.lpClass) 
			return *this;
		if(lpClass)
			lpClass->Release(); 
		lpClass = smart.lpClass; 
		if(lpClass)
			lpClass->AddRef(); 
		return *this;
    }
	inline int operator == ( T* ptr)
		{ return ptr == lpClass; }
	inline int operator != ( T* ptr)
		{ return ptr != lpClass; }
};
#endif
/**
 ** Generic factory that returns void * pointer \b that \b must \b be \b cast to the right type :
 ** any objects that should be available and that would derive from SmartRef
 **
 ** Note that this factory doesn't work with interfaces : a simple version.
 ** No CLSID are used. Instead we prefere using simple strings. Enough...
 **
 ** The name has "RT" because we don't use Factory system in the strict way :
 ** <LI> the returned pointer is not an Unknown or SmartRef : it is already cast (!)
 ** <LI> the reference to the object is made through a string (not a GUID)
 ** <LI> the basis of returned interface is ISmartRef, which is not really an interface basis...
 **
 **/
class IRTFactory
{
public:
	/// instanciation of an object (like ... = new ...;). \arg \c lparam1 & \c lparam2 depend on the user
	virtual const void* CreateInstance(LPCSTR lpInstTypeName, unsigned long lparam1=0, unsigned long lparam2=0) = 0; ///< like "new" token
	/// returns a singleton. A singleton is something that is unique and that may already exist.
	virtual const void* GetSingletonOf(LPCSTR lpInstTypeName, unsigned long lparam1=0, unsigned long lparam2=0) = 0; ///< if we want a singleton...
	/// last error
	virtual LPCSTR GetLastErrorMsg() = 0;
	/// total amount of objects. categories is a table of strings
	virtual int GetNumberOfObjects(LPCSTR filtercat=NULL) = 0;
	/// total amount of singletons. categories is a table of strings
	virtual int GetNumberOfSingletons(LPCSTR filtercat=NULL) = 0;
	/// the brings the first object in the list. \retval returns != 0 if success.
	virtual int GetFirstObjectInfo(LPCSTR &objname, LPCSTR &objdesc, LPCSTR **categories=NULL, LPCSTR filtercat=NULL) = 0;
	/// the brings the next object in the list. \retval returns != 0 if success.
	virtual int GetNextObjectInfo(LPCSTR &objname, LPCSTR &objdesc, LPCSTR **categories=NULL, LPCSTR filtercat=NULL) = 0;
	/// the brings the first singleton in the list. \retval returns != 0 if success.
	virtual int GetFirstSingletonInfo(LPCSTR &objname, LPCSTR &objdesc, LPCSTR **categories=NULL, LPCSTR filtercat=NULL) = 0;
	/// the brings the next singleton in the list. \retval returns != 0 if success.
	virtual int GetNextSingletonInfo(LPCSTR &objname, LPCSTR &objdesc, LPCSTR **categories=NULL, LPCSTR filtercat=NULL) = 0;
	/// \name brings picture handles (HBITMAP under windows) for various parts. can help for plugins and maintenance.
	//@{
	/// \arg \c smod is used in case of aggregation of multiple modules.
	virtual unsigned long GetPictureForModule(LPCSTR smod=NULL) { return 0; };
	virtual unsigned long GetPictureForCategory(LPCSTR scat) { return 0; };
	virtual unsigned long GetPictureForObjectType(LPCSTR sobjtype) { return 0; };
	//@}
	virtual void GetModuleInfos(LPCSTR &desc, LPCSTR &revision, LPCSTR *CPPheader=NULL) = 0;
};

} // namespace sapi

/**
 **	typedef of the function that brings us the factory
 **/
typedef sapi::IRTFactory* (*LPGetRTFactory)();
/**
 **	define that helps to get the factory
 **/
#define GETRTFACTORY(f, lib)\
{\
	HINSTANCE hLib = LoadLibrary(lib);\
	if(hLib)\
	{\
		LPGetRTFactory pgetf = (LPGetRTFactory)GetProcAddress(hLib, "GetRTFactory");\
		if(pgetf)\
		{\
			f = (*pgetf)();\
		}\
	}\
}
/**
 **	define that helps to release the factory
 **/
#define RELEASERTFACTORY(f, lib)\
{\
	HINSTANCE hLib = LoadLibrary(lib);\
	if(hLib)\
	{\
		BOOL b=TRUE;\
		while(b == TRUE)\
		{\
			b=FreeLibrary(hLib);\
		}\
		f = NULL;\
	}\
}


/**
 **	Implementation of the smartref methods. 
 ** Use this define when you derive your object from ISmartRef.
 ** 
 ** These defines allow us to provide other interfaces through QueryInterface().
 **/
#define BEGINIMPLSMARTREF()\
	int crefs;\
	int GetRef(void) \
	{ \
		return crefs; \
	}\
	void AddRef(void) \
	{ \
		++crefs; \
		return;\
	}\
	void Release(void) \
	{ \
		if (--crefs <= 0) \
		{\
			SRDelete();\
		}\
	}\
	void *QueryInterface(LPCSTR ItfName)\
	{\
	if(!ItfName) return NULL;\
	if(ItfName == NULL)\
		return static_cast<ISmartRef*>(this);
/// To add new query of an interface
#define QIMAPITF(itf) if(!strcmp(ItfName, #itf))\
		return static_cast<itf*>(this);
#define QIMAPITF2(itf, name) if(!strcmp(ItfName, name))\
		return static_cast<itf*>(this);
/// Terminate SmartRef implementation
#define ENDIMPLSMARTREF() return NULL; }
/**
 **	Declaration of the smartref methods. Used in the declaration of new interfaces
 **/
#define DECLSMARTREF()\
	virtual int GetRef(void) = 0; \
	virtual void AddRef(void) = 0; \
	virtual void Release(void) = 0; \
	virtual void *QueryInterface(LPCSTR ItfName) = 0;
/**
 ** Put it in Constructor of the class that must implement the ISmartRef.
 **/
#define IMPLSMARTREFINIT\
	crefs = 0;

#endif