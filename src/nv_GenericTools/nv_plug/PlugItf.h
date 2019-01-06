/**
\file PlugItf.h Plug Interface system

	This include is defining the interface (abstract class) for a plug::Plug

**/ /******************************************************************************/
#ifndef __PlugInterfaces__
#define __PlugInterfaces__

#include "nv_rtmodules/sapi_smartptr_itf.h"

using namespace sapi;
#include <vector>
#include <string>

#ifdef WIN32
  #include <windows.h>
#else
	#define DWORD unsigned long
	#define ULONG unsigned long
	#define BYTE unsigned char
	#define LPCSTR const char *
	#include <malloc.h>
#endif

/**
	this version of the STL string is using the allocator HeapAlloc.
	The purpose is to allow one dll to allocate the strings and another to release it.
	Normally, we cannot share heaps between dll's. In this case, 
	we make the allocation relative to the whole process.
**/
class myallocator {
	public:
		typedef _SIZT size_type;
		typedef _PDFT difference_type;
		typedef char _FARQ *pointer;
		typedef const char _FARQ *const_pointer;
		typedef char _FARQ& reference;
		typedef const char _FARQ& const_reference;
		typedef char value_type;
		template<class _Other>
			struct rebind
			{	// convert an allocator<_Ty> to myallocator
				typedef myallocator other;
			};
// For .NET
#if 1
	pointer address(reference _Val) const
		{	// return address of mutable _Val
		return (&_Val);
		}

	const_pointer address(const_reference _Val) const
		{	// return address of nonmutable _Val
		return (&_Val);
		}

  	myallocator()
		{	// construct default allocator (do nothing)
		}

		myallocator(const myallocator&)
		{	// construct from a related allocator (do nothing)
		}

    myallocator& operator=(const myallocator&)
		{	// assign from a related allocator (do nothing)
		return (*this);
		}

	void deallocate(pointer _Ptr, size_type)
		{	// deallocate object at _Ptr, ignore size
		#ifdef WIN32
					HeapFree(GetProcessHeap(), 0, _Ptr);
		#else
					free(_Ptr);
		#endif
		}

	pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
		#ifdef WIN32
				return (pointer)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _Count);
		#else
				return (pointer)malloc(_Count);
		#endif
		}

	pointer allocate(size_type _Count, const void _FARQ *)
		{	// allocate array of _Count elements, ignore hint
		#ifdef WIN32
				return (pointer)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _Count);
		#else
				return (pointer)malloc(_Count);
		#endif
		}

	void construct(pointer _Ptr, const char& _Val)
		{	// construct object at _Ptr with value _Val
		#ifdef WIN32
				pointer p = (pointer)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1);
		#else
				pointer p = (pointer)malloc(1);
		#endif
    p[0] = _Val;
		}

	void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
		#ifdef WIN32
					HeapFree(GetProcessHeap(), 0, _Ptr);
		#else
					free(_P);
		#endif
		}
// Old : for VC6
#else
    pointer address(reference _X) const
			{return (&_X); }
		const_pointer address(const_reference _X) const
			{return (&_X); }
		pointer allocate(size_type _N, const void *)
		{
		#ifdef WIN32
				return (pointer)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _N);
		#else
				return (pointer)malloc(_N);
		#endif
		}
		char _FARQ *_Charalloc(size_type _N)
		{
		#ifdef WIN32
				return (pointer)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _N);
		#else
				return (pointer)malloc(_N);
		#endif
		}
		void deallocate(void _FARQ *_P, size_type)
		{
		#ifdef WIN32
					HeapFree(GetProcessHeap(), 0, _P);
		#else
					free(_P);
		#endif
		}
		void construct(pointer _P, const char& _V)
		{
		}
		void destroy(pointer _P)
		{
		#ifdef WIN32
					HeapFree(GetProcessHeap(), 0, _P);
		#else
					free(_P);
		#endif
		}
#endif
		_SIZT max_size() const {_SIZT _N = (_SIZT)(-1) / sizeof (char);	return (0 < _N ? _N : 1); }
		char *_ptr;
};

inline bool operator==(const myallocator&, const myallocator&) {return (true); }
inline bool operator!=(const myallocator&, const myallocator&) {return (false); }

typedef std::basic_string<char, std::char_traits<char>, myallocator > String;

namespace plug
{
/// \name flags used by the plug.
//@{
#define PLUG_READ	1 ///< bit to tell the Plug can be read
#define PLUG_WRITE	2 ///< bit to tell the Plug can be written
#define PLUG_RW		3 ///< bit to tell the Plug can be read/write

#define PLUG_IN		4 ///< to flag a connection of a plug as an input
#define PLUG_OUT	8 ///< to flag a connection of a plug as an output

#define PLUG_CHILDREN 16 ///< for example in Plug::getNumConnections() : to take children into account
//@}
/**
these are the available types of Plug we can create : Byte, Int, Float, String etc.

Note that PLUG_COMPOSITE is a special case when the plug contains other plugs. In this case we can see
such a Plug as a specific structure. We'll see that a Composite Plug will have to provide a custom id for
its custom type it defines.

PLUG_CUSTOM is another type we may use when we are downclassing Plug class into a specific Plug.
**/
typedef enum
{
	PLUG_CUSTOM=0,
	PLUG_BYTE,
	PLUG_INT,
	//PLUG_SHORT, TODO, please
	PLUG_DWORD,
	PLUG_FLOAT,
	PLUG_BOOL,
	PLUG_STRING,
	PLUG_FLOATVEC2,
	PLUG_FLOATVEC3,
	PLUG_FLOATVEC4,
	PLUG_BYTEVEC2,
	PLUG_BYTEVEC3,
	PLUG_BYTEVEC4,
	PLUG_COMPOSITE
} PlugType;

class IPlug;
/**
Interface for a container plug::PlugContainer (See it for more details)

a container of Plug can be seen as a 'blackbox' doing some specific computation. Plugs will be exposed from this blackbox so that you can create so data
stream going in and out of this blackbox.

Conceptually, it is important to considere the plug::PlugContainer as the brain of the data processing, while plugs are simply the way to connect and access data of
this blackbox.

However, you'll see that in order to simplify the way to handle data, plugs can be linked to variable pointers. There is a default implementation that allow to
update variable linked with plugs.

Another special case : you can create some plugs without having PlugContainers. Therefore, plugs are orphans and will have to be handled by callbacks (see plug:IPlug).
The best case is when the plug is linked to a variable of known type : you don't even have to care about callbacks since there is a defaut implementation.
Considere this special case of not having PlugContainer class as having the whole application acting as a PlugContainer class equivalent...

 **/
class IPlugContainer
{
public:
	/// type for a group of plugs
	typedef std::vector<IPlug*> Plugmap; 
	/// iterator to walk through the group of plugs
	typedef Plugmap::const_iterator ItPlug; 
	/// Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations
	DECLSMARTREF();
	/** create a new Plug object and returns its interface. This plug will be registered to the PlugContainer class.
	\arg name : name of the plug. Used to help to find it back (in scripting, for example)
	\arg id : will allow to identify it (when you'll be in a callback event...). But there is no check to make sure this id is unique
	\arg pt : type of Plug plug::PlugType gives the whole list
	\arg lpData : an easy way to associate a variable (pointed by pt) with the Plug. \e Warning : there is no check on the consistency of the plug type and pt type.
	\arg sz : array size of the plug. 1 is just for a single variable (float, vector 2d..3d, bool...)
	\customsize : if the type that the plug deals with isn't part of plug::PlugType, then it is a custom type. So you'll have to give the size of the data in bytes.
	\customtypeid : if the plug is a custom plug (undefined type), pass a specific id to be able to recognize this custom type. \e Warning : no check if this id is unique.
	**/
	virtual IPlug*		plug_new(LPCSTR name, unsigned short id, PlugType pt, void *lpdata=NULL, int sz=1, int customsize=0, LPCSTR customtypeid=NULL) = 0;
	/** Create and register a composite Plug  : a plug that can contain children plugs.
	For example you could have a structure made of one float, 4 4D vector and a boolean. the composite plug allow you to work with connection on the top of this group.
	This can be usefull to easily connect complex structures of data together.
	**/
	virtual IPlug*		plug_newComposite(LPCSTR name, unsigned short id, LPCSTR comptype) = 0;
	/** register an existing plug to the container. This can be used if you created the plug without the help of plug::PlugContainer::plug_new or plug::PlugContainer::plug_newComposite
	\return true if it registered correctly.
	**/
	virtual bool		plug_Register(IPlug *p) = 0;
	/// unregister a plug that is part of this container.
	virtual void		plug_unregister(IPlug *p) = 0;
	/// delete and unregister a plug from its name.
	virtual bool		plug_Delete(LPCSTR name) = 0;
	/// delete and unregister a plug from its pointer
	virtual bool		plug_Delete(IPlug *p) = 0;
	/// returns the iterator of the first plug in this container. Use it to walk through the repository.
	virtual ItPlug		plug_begin() const = 0;
	/// returns the end iterator. Use it to see if you reached the end. typical STL way to do...
	virtual ItPlug		plug_end() const = 0;
	/// look for a named plug. returns the pointer if it succeeded.
	virtual IPlug*		plug_Find(LPCSTR name) = 0;
	/** performs a plug connection between 2 plugs.
	This connection is performed between a plug which is in this container, with a plug which is in another container (pCont argument). We use the string name to fetch them.
	\arg sFrom : the plug name that is in this container
	\arg pCont : the PlugContainer class pointer which owns the second plug names 'sTo'
	\arg sTo : the plug name that is in the container pCont

	This command can be used to connect plugs at runtime. From a script, for example.
	**/
	virtual bool plug_Connect(LPCSTR sFrom, IPlugContainer *pCont, LPCSTR sTo) = 0;
	/** disconnects 2 plugs.
	This operation is performed between a plug which is in this container, with a plug which is in another container (pCont argument). We use the string name to fetch them.
	\arg sFrom : the plug name that is in this container
	\arg pCont : the PlugContainer class pointer which owns the second plug names 'sTo'
	\arg sTo : the plug name that is in the container pCont

	This command can be used to connect plugs at runtime. From a script, for example.
	**/
	virtual bool plug_Disconnect(LPCSTR sFrom, IPlugContainer *pCont, LPCSTR sTo) = 0;
	/** Plugs allow complex inter-connections of plugs. So, data can flow in many directions. Use this method to make sure the data got updated and propagated through connections.
	The container will walk through its plugs and flush them all. This is something you want to do 
	- when you turned off \e immediate \e Flush : plug_setImmediateFlush(false). Since the connected plugs didn't get updated, you'll have to ask for it thanks to this method.
	- when you changed a variable that is related to a plug (you gave its pointer). So the plug need to get this
	new value and propagate it to the connected ones.
	**/
	virtual void plug_updateFlushes() = 0;
	/** When you change the value of a plug, you have two strategies :
	- change the value and propagate it to the connected plugs (bYes=true). This is the default case.
	- change the value but don't propagate changes. You'll have to ask for flushing data by calling PlugContainer::plug_updateFlushes() : all the connected plugs will get updated.
	.
	solution 2 could be interesting to optimize the process when working frame by frame, for example. 
	If the connections of frame F depend on frame F-1, then you can postpone the connection update to the end of the frame job.
	**/
	virtual void plug_setImmediateFlush(bool bYes) = 0;
	/** When deriving your class from plug::PlugContainer, you may want to do specific work when a dedicated plug belonging to this container got changed.
	Implement this method to take the control over the way that the plugs get written.
	\arg lpSrc : the connected plug that is willing to change your plug
	\arg lpDst : the plug from this container that is about to be changed.
	\arg bSendEvent : ...to explain...

	This callback is interesting when some plugs have 'dynamic' information. A simple example: if a plug is representing distant value through the network,
	you may want to access the data via a protocol to write the new value. This would be done here...
	**/
	virtual		bool plug_IncomingData(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent=true) = 0;
	/** When deriving your class from plug::PlugContainer, you may want to do specific work when a dedicated plug belonging to this container gets accessed for reading purpose.
	Implement this method to take the control over the way that the plugs will give its value to the one that asked for it.
	\arg lpPlug : the plug that is being accessed for reading purpose.
	\arg idx : tell you what element in the array we want you to give back. Most of the time, this is 0 (no array).
	\arg arraysize : if not NULL, fill it with the sice of the array of the data exposed by the plug.

	This callback is interesting when some plugs have 'dynamic' information. A simple example: if a plug is representing distant value through the network,
	you may want to access the data via a protocol when the data is being requested.
	**/
	virtual		void *plug_DataReadEvent(IPlug *lpPlug, int &idx, int *arraysize) = 0;
};

/**
	Interface for the plug::Plug class.

Plugs allow the 'blackbox' plug::PlugContainer to be connected with the outside world.

Conceptually, it is important to considere the plug::PlugContainer as the brain of the data processing, while plugs are simply the way to connect and access data of
this blackbox.

However, you'll see that in order to simplify the way to handle data, plugs can be linked to variable pointers. There is a default implementation that allow to
update variable linked with plugs.

Another special case : you can create some plugs without having PlugContainers. Therefore, plugs are orphans and will have to be handled by callbacks (see plug:IPlug).
The best case is when the plug is linked to a variable of known type : you don't even have to care about callbacks since there is a defaut implementation.
Considere this special case of not having PlugContainer class as having the whole application acting as a PlugContainer class equivalent (bu with no class)...

 **/
class IPlug
{
public:
	/// callback prototype when we want to track write ops
	typedef bool (*IncomingDataCB)(IPlug *lpFrom, IPlug *lpTo, unsigned long ul1, unsigned long ul2); 
	/// callback prototype when we want to track read ops
	typedef void *(*DataReadEventCB)(IPlug *lpPlug, int &idx, int *arraysize, unsigned long ul1, unsigned long ul2); 
	/// type of the array for in/out connected plugs
	typedef std::vector<IPlug* >						Plugs;	
	/// iterator of plugs to walk into the arrays
	typedef Plugs::const_iterator					ItPlugs;
	/// group of plugs
	//typedef std::map<String, IPlug*, ltstr>				Plugmap;
	typedef Plugs Plugmap;
	/// iterator of plugmap
	typedef Plugmap::const_iterator						ItPlugchildren;

	DECLSMARTREF(); ///< Macro for GetRef(), AddRef(), Release() , QueryInterface() implementations

	/// \name Connection and state of the IPlug
	//@{
	/// give back the container which owns this plug
	virtual IPlugContainer *getParentContainer() = 0;
	/// assign a container to this plug. Same as PlugContainer::plug_Register
	virtual void setParentContainer(IPlugContainer *parent) = 0;
	/// set the parent of this plug : parent is a composite plug.
	virtual void setParentPlug(IPlug *parent) = 0;
	/// add a plug as a child. This means that this plug is a composite plug.
	virtual void addChildPlug(IPlug *newchild) = 0;
	/// gives back the parent composite plug
	virtual IPlug *getParentPlug() = 0;
	/// return the id of the custom type, if it has.
	virtual unsigned long getCustomType() const = 0;
	/// return the the internal flags of the plug : PLUG_READ PLUG_WRITE...
	virtual BYTE		getMode()  const = 0;
	/// set the internal flags of the plug : PLUG_READ PLUG_WRITE...
	virtual void		setMode(BYTE m) = 0;
	/// return the type of the plug
	virtual PlugType	getType()  const = 0;
	/// check if this plug is compatible with 'pt'. If true, then you can connect this plug with another of 'pt' type.
	virtual bool		checkCompatibleType(PlugType pt)  const = 0;
	/// return the Name of the Plug
	virtual String		getName()  const = 0;
	/// return the Full name of the Plug. \todo more details, here
	virtual String		getFullName()  const = 0;
	/// return a bief description of this plug.
	virtual String		getDesc()  const = 0;
	/// return the id of the Plug.
	virtual unsigned short getId()  const = 0;
	 /** connect \b from \b this to the plug interface passed as ptr. One direction \b only if \c bothsides=false.

	  \e Note that the default value will be set to the one from 'this' object. Thus IPlug * argument will be set to this value... **/
	virtual bool		connectTo(IPlug *, bool bothsides=false) = 0;
	/// disconnect this plug from the one passed as ptr.
	virtual bool		disconnect(IPlug *) = 0;
	/// disconnect all the plugs connected to this plug.
	virtual bool		disconnectAll() = 0;
	/// returns the first iterator of the source plugs : the ones that can bring new values to this plug.
	virtual ItPlugs		srcBegin() const = 0;
	/// end iterator : use it as in STL, to check if you reached the end.
	virtual ItPlugs		srcEnd() const = 0;
	/// returns the first iterator of the destination plugs : the ones that can receive new values from this plug.
	virtual ItPlugs		dstBegin() const = 0;
	/// end iterator : use it as in STL, to check if you reached the end.
	virtual ItPlugs		dstEnd() const = 0;
	/// look for a plug that is connected to this one. You can choose to look for it in destination and/or source plugs.
	virtual IPlug*	FindPlug(LPCSTR name, bool dst=true, bool src=true) const = 0;
	/// get the plug source (connected to the input of this plug) number 'num'.
	virtual IPlug*	getPlugIn(int num=0) const = 0;
	/// get the plug destination (connected to the output of this plug) number 'num'.
	virtual IPlug*	getPlugOut(int num=0) const = 0;
	/// return the amount of connected plugs. PLUG_OUT|PLUG_IN flags to choos what to count.
	virtual int			getNumConnections(BYTE rw=PLUG_OUT|PLUG_IN) = 0;
	/// \name Children Plugs
	//@{
	/// first iterator of the children
	virtual ItPlugchildren		childBegin() const = 0;
	/// end iterator to check like in STL
	virtual ItPlugchildren		childEnd() const = 0;
	/// if the plug has children, returns > 0
	virtual int			hasChildren() const = 0;
	/// search for a specific child, with its name.
	virtual IPlug*		plug_Find(LPCSTR name) = 0;
	/// delete a child plug with its pointer
	virtual bool		plug_Delete(IPlug *plug) = 0;
	/// delete a child plug with its name
	virtual bool		plug_Delete(LPCSTR name) = 0;
	/** perform a connection between 2 plugs by using the names.
	\arg sFrom is the child plug
	\arg pPlugParent is the [composite] plug where you can find sTo named plug
	\arg sTo is the name of the plug in pPlugParent
	**/
	virtual bool		plug_Connect(LPCSTR sFrom, IPlug *pPlugParent, LPCSTR sTo) = 0;
	/** disconnect 2 plugs by using the names.
	\arg sFrom is the child plug
	\arg pPlugParent is the [composite] plug where you can find sTo named plug
	\arg sTo is the name of the plug in pPlugParent
	**/
	virtual bool		plug_Disconnect(LPCSTR sFrom, IPlug *pPlugParent, LPCSTR sTo) = 0;
	/** create a new child plug
	\arg name : name of the plug. Used to help to find it back (in scripting, for example)
	\arg id : will allow to identify it (when you'll be in a callback event...). But there is no check to make sure this id is unique
	\arg pt : type of Plug plug::PlugType gives the whole list
	\arg lpData : an easy way to associate a variable (pointed by pt) with the Plug. \e Warning : there is no check on the consistency of the plug type and pt type.
	\arg sz : array size of the plug. 1 is just for a single variable (float, vector 2d..3d, bool...)
	\customsize : if the type that the plug deals with isn't part of plug::PlugType, then it is a custom type. So you'll have to give the size of the data in bytes.
	\customtypeid : if the plug is a custom plug (undefined type), pass a specific id to be able to recognize this custom type. \e Warning : no check if this id is unique.
	**/
	virtual IPlug*		plug_new(LPCSTR name, unsigned short id, PlugType pt, void *lpdata=NULL, int sz=1, int customsize=0, LPCSTR customtypeid=NULL) = 0;
	/// create a new child and composite plug.
	virtual IPlug*		plug_newComposite(LPCSTR name, unsigned short id, LPCSTR comptype) = 0;
	//@}
	/// size of the array of data that the plug is representing
	virtual int			&arraySize() = 0;
	/// return a raw pointer to the data that the plug is representing. Note that it can be NULL : some plug aren't especially connected to a real variable in memory.
	virtual void *		&lpData() = 0;
	/// check if the plug is of type 'pt'
	virtual bool		isTypeOfData(PlugType pt) const = 0;
	/// \todo explain what it is...
	virtual bool		getValidFlags(const BYTE **validarray, unsigned long * fields, unsigned long *fields2) const = 0;
	//@}
	/// \name Operators
	//@{
	virtual operator const PlugType() const = 0;
	virtual operator const String() const = 0;
	virtual operator const unsigned short() const = 0;
	//@}
	/// \name management for R/W Events of in the IPlug.
	//@{
	/// tell that the plug has changed somewhere.
	virtual void		setAsDirty(bool bRecurseChildren=true) = 0;
	/// check if the plug is changed
	virtual bool		isDirty() = 0;
	/// flush the plug value to the connected ones, to the parent and to the children
	virtual void		flush(bool bRecurseChildren=true, bool bRecurseParents=true) = 0;
	/** Plugs allow complex inter-connections of plugs. So, data can flow in many directions. Use this method to make sure the data got updated and propagated through connections.
	This is something you want to do 
	- when you turned off \e immediate \e Flush : plug_setImmediateFlush(false). Since the connected plugs didn't get updated, you'll have to ask for it thanks to this method.
	- when you changed a variable that is related to a plug (you gave its pointer). So the plug need to get this
	new value and propagate it to the connected ones.
	**/
	virtual void		updateFlushes(bool bRecurseChildren=true, bool bRecurseParents=true) = 0;
	/** When you change the value of a plug, you have two strategies :
	- change the value and propagate it to the connected plugs (bYes=true). This is the default case.
	- change the value but don't propagate changes. You'll have to ask for flushing data by calling PlugContainer::plug_updateFlushes() : all the connected plugs will get updated.
	.
	solution 2 could be interesting to optimize the process when working frame by frame, for example. 
	If the connections of frame F depend on frame F-1, then you can postpone the connection update to the end of the frame job.
	**/
	virtual void		setImmediateFlush(bool bYes) = 0;
	/// check the mode for flushing
	virtual bool		isFlushingImmediate() = 0;
	/// you can set additional callback to handle incoming data for this plug. This is equivalent to handling data with PlugContainer::plug_IncomingData
	virtual void		setIncomingDataCB(IncomingDataCB lpFn) = 0;
	/// you can set additional callback to handle outgoing data from this plug. This is equivalent to handling data with PlugContainer::plug_DataReadEvent
	virtual void		setDataReadEventCB(DataReadEventCB lpFn) = 0;
	/// you can attach 2 unsigned long values to a plug for any purpose
	virtual void		setUserLong(unsigned long ul1, unsigned long ul2 = 0) = 0;
	/// get back the user data
	virtual void		getUserLong(unsigned long *ul1, unsigned long *ul2 = NULL) = 0;

	virtual bool		plug_IncomingData(IPlug *lpSrc) = 0;
	//@}
	virtual bool operator==(LPCSTR name) const = 0; ///< easy comparison of the plug name with a string
	/** \name Methods for reading values
	
	Use these methods to read the value(s) of a plug. These methods will perform data conversion if needed (reading a float plug as a string, for example)
	**/
	//@{
	virtual void*		getPtr(int idx=0) = 0; ///< when you want a structure that you KNOW how it is made !!
	virtual bool		getValues(void *val, int maxitems=0) = 0;
	virtual bool		getValue(void *val, int idx=0) = 0;
	virtual bool		getValue(BYTE &val, int idx=0) = 0;
	virtual bool		getValue(int &val, int idx=0) = 0;
	virtual bool		getValue(DWORD &val, int idx=0) = 0;
	virtual bool		getValue(bool &val, int idx=0) = 0;
	virtual bool		getValue(String &val, int idx=0) = 0;
	//virtual bool		getValue(const float val[], int idx=0) = 0;
	virtual bool		getValue(const BYTE val[], int idx=0) = 0;
	virtual bool		getValue(float &val, int idx=0) = 0;
	virtual bool		getValue(float &x, float &y, int idx=0) = 0;
	virtual bool		getValue(float &x, float &y, float &z, int idx=0) = 0;
	virtual bool		getValue(float &x, float &y, float &z, float &w, int idx=0) = 0;
	//@}
	/** \name Methods for writing values.

	safe ways to write values to a plug. Note that if the plug is attached to a variable pointer, you could set the attached variable and flush the plug (plug::IPlug::flush).
	But these methods do it for you and perform conversion if needed (setting a float plug with a boolean value, for example).

	\warning : these methods shouldn't be called frome plug::PlugContainer::plug_IncomingData. Infinite loop may appear...
	 **/
	//@{
	virtual bool		setValue(void* ptr, int idx=0) = 0;
	virtual bool		setValue(BYTE val, int idx=0) = 0;
	virtual bool		setValue(int val, int idx=0) = 0;
	virtual bool		setValue(DWORD val, int idx=0) = 0;
	virtual bool		setValue(float val, int idx=0) = 0;
	virtual bool		setValue(bool val, int idx=0) = 0;
	virtual bool		setValue(String val, int idx=0) = 0;
	virtual bool		setValue(LPCSTR pstr, int idx=0) = 0;
	virtual bool		setValue(const float val[], int idx=0) = 0;
	virtual bool		setValue(const BYTE val[], int idx=0) = 0;
	virtual bool		setValue(float x, float y, int idx=0) = 0;
	virtual bool		setValue(float x, float y, float z, int idx=0) = 0;
	virtual bool		setValue(float x, float y, float z, float w, int idx=0) = 0;
	//@}
	/// \name connection
	//@{
	/// connect pSrc to this plug as a source
	virtual void connectAsSource(IPlug *pSrc) = 0;
	/// connect pDst to this plug as a destination plug
	virtual void connectAsDest(IPlug *pDst) = 0;
	/// disconnect plug to this plug as a source
	virtual bool disconnectAsDest(IPlug *plug) = 0;
	/// disconnect plug to this plug as a destination plug
	virtual bool disconnectAsSource(IPlug *plug) = 0;
	//@}
	//virtual ItPlugchildren findPlugChild(LPCSTR name) = 0;
};

/**
	CGraphvis is a class that will write the PlugContainer and Plug classes into a
	graphical description from Graphvis project.

	Later this description would allow Graphvis (dot, neato) to render it in ps format.
 **/
class ICGraphvis
{
public:
	virtual void beginGraph(int mode=0, bool bHideUnusedPlugs=false) = 0;
	virtual void endGraph() = 0;
	virtual void beginSubGraph(LPCSTR label) = 0;
	virtual void endSubGraph() = 0;
	virtual void beginRank() = 0;
	virtual void endRank() = 0;
	virtual bool addNode(IPlugContainer *pnode, LPCSTR pname) = 0;

	bool beginNode(LPCSTR pname);
	bool addPlug(IPlug *pplug);
	bool endNode();

	virtual bool writeToFile(LPCSTR fname) = 0;
};

} //namespace plug
#endif
