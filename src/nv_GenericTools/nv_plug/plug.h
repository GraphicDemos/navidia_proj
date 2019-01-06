/**
\file Plug.h Plug system

	This include is dealing with the Plug concept : an overlay that would allow us
	to make connections between various variables of objects.

	This Plug system is also working with Properties : it would be a layer dynamicaly
	allocated on top of a property when we decided to connect this property to another
	way.

**/ /******************************************************************************/
#ifndef __PLUG_H
#define __PLUG_H

#pragma warning(disable: 4305)
#pragma warning(disable: 4786)
#include <map>
#pragma warning(disable: 4786)
#include <list>
#pragma warning(disable: 4786)
#include <vector>
#include <assert.h>
#include <string.h>

//#include "sapi_smartptr.h"
//using namespace sapi;
#include "PlugItf.h"

/**
used to delete a plug in PlugContainer : disconnect all connected plugs and then delete the plug
**/
#define DELETE_PLUG(p) \
	if(p)\
	{\
		p->disconnectAll();\
		assert(p->GetRef() == 1);\
		plug_Delete(p);\
		p = NULL;\
	}

namespace plug
{
	
class Property;
class EnumProperty;
class PropertySet;
class VariableSet;
#ifndef NO_SMARTPTR_USE
class SmartPropertySet;
class SmartVariableSet;
#endif

#ifndef OBJECT_MEMBER
/// This is used to compute the offset to access variables from PropertySet to your up-classed object
#define OBJECT_MEMBER(member) ((BYTE*)(&member) - (BYTE*)this)
#endif


class Plug;

struct ltstr
{
  bool operator()(const String s1, const String s2) const
  {
	return strcmp(s1.c_str(), s2.c_str()) < 0;
  }
};

/*******************************************************/ /**
The basic idea is to provide such a container as a basic class to inherite from 
and can provide some services for these 'registered' plugs.

A Container of Plug can be used to create some \e operators : a block with Plugs for input data and for output.
For example a Container may help to define a math operation receiving 3 vectors through 3 Plugs and sending 
the Barycentric vector as an Output. Such a container is a function represented as a block we can connect thanks
to the Plugs. In this example, In/Out Plugs would be created by the container at init time.

When creating a special container, we must care about the following methods we may want to overload in order to
get a more specific behaviour than the original one :
PlugContainer::SRDelete, PlugContainer::plug_new, PlugContainer::plug_newComposite, 
PlugContainer::plug_Find, PlugContainer::plug_Connect, PlugContainer::plug_Disconnect, 
PlugContainer::plug_updateFlushes, PlugContainer::plug_setImmediateFlush,
PlugContainer::plug_IncomingData, PlugContainer::plug_DataReadEvent

- PlugContainer::plug_new & PlugContainer::plug_newComposite & PlugContainer::plug_Delete to create/delete new plugs, composite or note. 
A composite plug can host other plugs
- PlugContainer::plug_Register & PlugContainer::plug_unregister to attach or detach the Plug in the container
- PlugContainer::plug_begin & PlugContainer::plug_end to walk through the plugs of this container through an iterator : PlugContainer::ItPlug
- PlugContainer::plug_Find to get a specific plugs from its name
- PlugContainer::plug_Connect & PlugContainer::plug_Disconnect to connect/disconnect 2 Plugs from its names
- PlugContainer::plug_updateFlushes & PlugContainer::plug_setImmediateFlush to update the data of all the plugs through their connections.
This will propagate the update process through the connections. This plug_updateFlushes should be used when
PlugContainer::plug_setImmediateFlush(false)
- PlugContainer::plug_IncomingData is the method which will be used when new data is arriving for a specific registered Plug.
We may want to overload this method if ever we have a specific way of updating the plugs of a container.
- PlugContainer::plug_DataReadEvent is the method we may want to overload when we need to know when a Plug is being read. 
This may be used when we must provide a value coming from a special source.


.


 **/
class PlugContainer : public IPlugContainer, public ISmartRef
{
protected:
	Plugmap	m_plugmap;
	typedef Plug *Papou[3];
	Papou ppppp;
public:
	/// \name methods for Smart reference
	//@{
	void SRDelete() { delete this; }
	BEGINIMPLSMARTREF();
		QIMAPITF(IPlugContainer)
	ENDIMPLSMARTREF();
	//@}
	PlugContainer();
	~PlugContainer();
	/// \name methods from IPlugContainer
	//@{
	/** create a new Plug object and returns its interface. This plug will be registered to the PlugContainer class.
	\arg name : name of the plug. Used to help to find it back (in scripting, for example)
	\arg id : will allow to identify it (when you'll be in a callback event...). But there is no check to make sure this id is unique
	\arg pt : type of Plug plug::PlugType gives the whole list
	\arg lpData : an easy way to associate a variable (pointed by pt) with the Plug. \e Warning : there is no check on the consistency of the plug type and pt type.
	\arg sz : array size of the plug. 1 is just for a single variable (float, vector 2d..3d, bool...)
	\customsize : if the type that the plug deals with isn't part of plug::PlugType, then it is a custom type. So you'll have to give the size of the data in bytes.
	\customtypeid : if the plug is a custom plug (undefined type), pass a specific id to be able to recognize this custom type. \e Warning : no check if this id is unique.
	**/
	virtual IPlug*		plug_new(LPCSTR name, unsigned short id, PlugType pt, void *lpdata=NULL, int sz=1, int customsize=0, LPCSTR customtypeid=NULL);
	/** Create and register a composite Plug  : a plug that can contain children plugs.
	For example you could have a structure made of one float, 4 4D vector and a boolean. the composite plug allow you to work with connection on the top of this group.
	This can be usefull to easily connect complex structures of data together.
	**/
	virtual IPlug*		plug_newComposite(LPCSTR name, unsigned short id, LPCSTR comptype);
	/** register an existing plug to the container. This can be used if you created the plug without the help of plug::PlugContainer::plug_new or plug::PlugContainer::plug_newComposite
	\return true if it registered correctly.
	**/
	virtual bool		plug_Register(IPlug *p);
	/// unregister a plug that is part of this container.
	virtual void		plug_unregister(IPlug *p);
	/// delete and unregister a plug from its name.
	virtual bool		plug_Delete(LPCSTR name);
	/// delete and unregister a plug from its pointer
	virtual bool		plug_Delete(IPlug *p);
	/// returns the iterator of the first plug in this container. Use it to walk through the repository.
	virtual ItPlug		plug_begin() const;
	/// returns the end iterator. Use it to see if you reached the end. typical STL way to do...
	virtual ItPlug		plug_end() const;
	/// look for a named plug. returns the pointer if it succeeded.
	virtual IPlug*		plug_Find(LPCSTR name);
	/** performs a plug connection between 2 plugs.
	This connection is performed between a plug which is in this container, with a plug which is in another container (pCont argument). We use the string name to fetch them.
	\arg sFrom : the plug name that is in this container
	\arg pCont : the PlugContainer class pointer which owns the second plug names 'sTo'
	\arg sTo : the plug name that is in the container pCont

	This command can be used to connect plugs at runtime. From a script, for example.
	**/
	virtual bool plug_Connect(LPCSTR sFrom, IPlugContainer *pCont, LPCSTR sTo);
	/** disconnects 2 plugs.
	This operation is performed between a plug which is in this container, with a plug which is in another container (pCont argument). We use the string name to fetch them.
	\arg sFrom : the plug name that is in this container
	\arg pCont : the PlugContainer class pointer which owns the second plug names 'sTo'
	\arg sTo : the plug name that is in the container pCont

	This command can be used to connect plugs at runtime. From a script, for example.
	**/
	virtual bool plug_Disconnect(LPCSTR sFrom, IPlugContainer *pCont, LPCSTR sTo);
	/** Plugs allow complex inter-connections of plugs. So, data can flow in many directions. Use this method to make sure the data got updated and propagated through connections.
	The container will walk through its plugs and flush them all. This is something you want to do 
	- when you turned off \e immediate \e Flush : plug_setImmediateFlush(false). Since the connected plugs didn't get updated, you'll have to ask for it thanks to this method.
	- when you changed a variable that is related to a plug (you gave its pointer). So the plug need to get this
	new value and propagate it to the connected ones.
	**/
	virtual void plug_updateFlushes();
	/** When you change the value of a plug, you have two strategies :
	- change the value and propagate it to the connected plugs (bYes=true). This is the default case.
	- change the value but don't propagate changes. You'll have to ask for flushing data by calling PlugContainer::plug_updateFlushes() : all the connected plugs will get updated.
	.
	solution 2 could be interesting to optimize the process when working frame by frame, for example. 
	If the connections of frame F depend on frame F-1, then you can postpone the connection update to the end of the frame job.
	**/
	virtual void plug_setImmediateFlush(bool bYes);
	//@}
	bool		Incoming_Composite(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent=true);
	/** When deriving your class from plug::PlugContainer, you may want to do specific work when a dedicated plug belonging to this container got changed.
	Implement this method to take the control over the way that the plugs get written.
	\arg lpSrc : the connected plug that is willing to change your plug
	\arg lpDst : the plug from this container that is about to be changed.
	\arg bSendEvent : ...to explain...

	This callback is interesting when some plugs have 'dynamic' information. A simple example: if a plug is representing distant value through the network,
	you may want to access the data via a protocol to write the new value. This would be done here...
	**/
	virtual		bool plug_IncomingData(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent=true);
	/** When deriving your class from plug::PlugContainer, you may want to do specific work when a dedicated plug belonging to this container gets accessed for reading purpose.
	Implement this method to take the control over the way that the plugs will give its value to the one that asked for it.
	\arg lpPlug : the plug that is being accessed for reading purpose.
	\arg idx : tell you what element in the array we want you to give back. Most of the time, this is 0 (no array).
	\arg arraysize : if not NULL, fill it with the sice of the array of the data exposed by the plug.

	This callback is interesting when some plugs have 'dynamic' information. A simple example: if a plug is representing distant value through the network,
	you may want to access the data via a protocol when the data is being requested.
	**/
	virtual		void *plug_DataReadEvent(IPlug *lpPlug, int &idx, int *arraysize);
};


/*******************************************************/ /**
A Plug is an object which wraps an arbitrary data : either a real variable or even an abstract variable
like a variable created 'on demand' (value generated when being read...) and allows other Plugs to connect to
it.

Plugs allow us to connect them to each other so that we may get some complex connections : when some values are
changed, the entire connection will be updated depending on the connections.

The basic class is made to provide basic types like vectors, scalar, integer, strings etc. But if we
downclass the Plug into a specific one (PLUG_CUSTOM), then we may be able to create special Plugs for
various unknown types.

A Plug can either be created in a static way, dynamic way or through the PlugContainer::plug_new, PlugContainer::plug_newContainer or
Plug::plug_new & Plug::plug_newContainer

If we create by ourselves the instance of a Plug Class, we'll have to provide some data to the constructor, 
like in PlugContainer::plug_new for example.



\todo Plug containers : Plugs can contain other plugs... for complex linkage
\todo IncomingDataCB & DataReadEventCB should work with class callbacks, too
 
 **/
class Plug : public IPlug, public ISmartRef
{
protected:
	Plugs			m_plugins,		///< plugs connected for incoming data
					m_plugouts;		///< plugs connected for out going data
	Plugmap			m_plugchildren;	
	IPlug*			m_lpParentPlug;	///< when a IPlug is owned by another
	IPlugContainer*	m_lpParentNode;	///< parent container
	BYTE			m_mode;			///< mode is READ, WRITE, RW
	BYTE			m_avoidrecurse; ///< to avoid re-entrancy in events (plug_IncomingData)
	String			m_name;			///< String of the name
	String			m_desc;			///< short description of this plug
	unsigned short	m_id;			///< id to identify quickly the plug in the container
	int				m_arraysize,	///< number of items (>1 <=> array)
					m_customdatasize; ///< custom size for custom data
	PlugType		m_type;			///< Type of the m_lpData
	unsigned long	m_customtypeid;	///< 4 chars to identify the custom & composite type ("TRAN", "QUAT"...)
	void*			m_lpData;		///< data ptr to the owner of this IPlug
	IncomingDataCB	m_IncomingDataCB; ///< optional pointer to a function which wants to handle incoming data like plug_IncomingData()
	DataReadEventCB m_DataReadEventCB; ///< optional pointer to a function which wants to handle reading events like DataReadEvent()
	unsigned long	m_userlong1, m_userlong2; ///< optional user data for DataReadEventCB & IncomingDataCB
	bool			m_bIsDirty;		///< true when the IPlug is dirty and needs to send flush
	bool			m_bImmediateFlush; ///< true if we want to propagate changes immediately. if false, we'll wait for updateflush
protected:
	void connectAsSource(IPlug *pSrc);
	void connectAsDest(IPlug *pSrc);
	bool disconnectAsDest(IPlug *plug);
	bool disconnectAsSource(IPlug *plug);
	//ItPlugchildren findPlugChild(LPCSTR name);

	static bool default_Incoming_Composite(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent);
	static bool default_IncomingData(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent); ///< used when no container available for this plug
	static void *default_DataReadEvent(IPlug *lpPlug, int &idx, int *arraysize); ///< used when no container available for this plug
public:
	/// \name methods for Smart reference
	//@{
	void SRDelete() { delete this; }
	BEGINIMPLSMARTREF();
		QIMAPITF(IPlug)
	ENDIMPLSMARTREF();
	//@}
/**
Constructor.

\arg name of the Plug
\arg id of the Plug
\arg type of the Plug
\arg pointer to the data
\arg size (number of items) of the data if it is an array
\arg parent PlugContainer
\arg customsize size of the custom type
\arg allocated is true if the plug class is static variable : then prevent from being freed by Release() method

\todo see if we would be better to use OBJECT_MEMBER() define i.e. an offset...
 **/
	Plug(LPCSTR name=NULL, 	
		unsigned short id=0, 
		PlugType pt=PLUG_CUSTOM, 
		void *lpdata=NULL, 
		int sz=1, 
		IPlugContainer* parent=NULL, 
		int customsize=0, 
		LPCSTR customtype = NULL, 
		IPlug* parentPlug=NULL,
		bool allocated=true);
/**
	Constructor for composite Plug.
	\arg name of the Plug
	\arg id of the Plug
	\arg composite type name : 4 chars
	\arg parentCont : ptr to the parent Container
	\arg parentPlug : ptr to the parent Plug
	.
 **/
	Plug(LPCSTR name, 
		unsigned short id, 
		LPCSTR comptype, 
		IPlugContainer* parentCont, 
		IPlug* parentPlug,
		bool allocated=true);
	~Plug();
	void Init(LPCSTR name=NULL, 	
		unsigned short id=0, 
		PlugType pt=PLUG_CUSTOM, 
		void *lpdata=NULL, 
		int sz=1, 
		IPlugContainer* parent=NULL, 
		int customsize=0, 
		LPCSTR customtype = NULL, 
		IPlug* parentPlug=NULL,
		bool allocated=true);
	/// \name Connection and state of the Plug
	//@{
	/// give back the container which owns this plug
	IPlugContainer *getParentContainer() { return m_lpParentNode; };
	/// assign a container to this plug. Same as PlugContainer::plug_Register
	void setParentContainer(IPlugContainer *parent) { m_lpParentNode = parent; };
	/// set the parent of this plug : parent is a composite plug.
	void setParentPlug(IPlug *parent);
	/// add a plug as a child. This means that this plug is a composite plug.
	void addChildPlug(IPlug *newchild);
	/// gives back the parent composite plug
	IPlug *getParentPlug() { return m_lpParentPlug; };
	/// return the id of the custom type, if it has.
	unsigned long getCustomType() const;
	/// return the the internal flags of the plug : PLUG_READ PLUG_WRITE...
	BYTE		getMode()  const;
	/// set the internal flags of the plug : PLUG_READ PLUG_WRITE...
	void		setMode(BYTE m){ m_mode = m; };
	/// return the type of the plug
	PlugType	getType()  const;
	/// check if this plug is compatible with 'pt'. If true, then you can connect this plug with another of 'pt' type.
	bool		checkCompatibleType(PlugType pt)  const;
	/// return the Name of the Plug
	String		getName()  const;
	/// return the Full name of the Plug. \todo more details, here
	String		getFullName()  const;
	/// return a bief description of this plug.
	String		getDesc()  const;
	void		setDesc(LPCSTR desc);
	/// return the id of the Plug.
	unsigned short getId()  const;
	 /** connect \b from \b this to the plug interface passed as ptr. One direction \b only if \c bothsides=false.

	  \e Note that the default value will be set to the one from 'this' object. Thus IPlug * argument will be set to this value... **/
	bool		connectTo(IPlug *, bool bothsides);
	/// disconnect this plug from the one passed as ptr.
	bool		disconnect(IPlug *);
	/// disconnect all the plugs connected to this plug.
	bool		disconnectAll() { return disconnect(NULL);};
	/// returns the first iterator of the source plugs : the ones that can bring new values to this plug.
	ItPlugs		srcBegin() const;
	/// end iterator : use it as in STL, to check if you reached the end.
	ItPlugs		srcEnd() const;
	/// returns the first iterator of the destination plugs : the ones that can receive new values from this plug.
	ItPlugs		dstBegin() const;
	/// end iterator : use it as in STL, to check if you reached the end.
	ItPlugs		dstEnd() const;
	/// look for a plug that is connected to this one. You can choose to look for it in destination and/or source plugs.
	IPlug*	FindPlug(LPCSTR name, bool dst=true, bool src=true) const;
	/// get the plug source (connected to the input of this plug) number 'num'.
	IPlug*	getPlugIn(int num=0) const;
	/// get the plug destination (connected to the output of this plug) number 'num'.
	IPlug*	getPlugOut(int num=0) const;
	/// return the amount of connected plugs. PLUG_OUT|PLUG_IN flags to choos what to count.
	int			getNumConnections(BYTE rw=PLUG_RW);
	/// \name Children Plugs
	//@{
	/// first iterator of the children
	ItPlugchildren		childBegin() const;
	/// end iterator to check like in STL
	ItPlugchildren		childEnd() const;
	/// if the plug has children, returns > 0
	int			hasChildren() const;
	/// search for a specific child, with its name.
	IPlug*		plug_Find(LPCSTR name);
	/// delete a child plug with its pointer
	bool		plug_Delete(IPlug *plug);
	/// delete a child plug with its name
	bool		plug_Delete(LPCSTR name);
	/** perform a connection between 2 plugs by using the names.
	\arg sFrom is the child plug
	\arg pPlugParent is the [composite] plug where you can find sTo named plug
	\arg sTo is the name of the plug in pPlugParent
	**/
	bool		plug_Connect(LPCSTR sFrom, IPlug *pPlugParent, LPCSTR sTo);
	/** disconnect 2 plugs by using the names.
	\arg sFrom is the child plug
	\arg pPlugParent is the [composite] plug where you can find sTo named plug
	\arg sTo is the name of the plug in pPlugParent
	**/
	bool		plug_Disconnect(LPCSTR sFrom, IPlug *pPlugParent, LPCSTR sTo);
	/** create a new child plug
	\arg name : name of the plug. Used to help to find it back (in scripting, for example)
	\arg id : will allow to identify it (when you'll be in a callback event...). But there is no check to make sure this id is unique
	\arg pt : type of Plug plug::PlugType gives the whole list
	\arg lpData : an easy way to associate a variable (pointed by pt) with the Plug. \e Warning : there is no check on the consistency of the plug type and pt type.
	\arg sz : array size of the plug. 1 is just for a single variable (float, vector 2d..3d, bool...)
	\customsize : if the type that the plug deals with isn't part of plug::PlugType, then it is a custom type. So you'll have to give the size of the data in bytes.
	\customtypeid : if the plug is a custom plug (undefined type), pass a specific id to be able to recognize this custom type. \e Warning : no check if this id is unique.
	**/
	virtual IPlug*		plug_new(LPCSTR name, unsigned short id, PlugType pt, void *lpdata=NULL, int sz=1, int customsize=0, LPCSTR customtypeid=NULL);
	/// create a new child and composite plug.
	virtual IPlug*		plug_newComposite(LPCSTR name, unsigned short id, LPCSTR comptype);
	//@}
	/// size of the array of data that the plug is representing
	int			&arraySize();
	/// return a raw pointer to the data that the plug is representing. Note that it can be NULL : some plug aren't especially connected to a real variable in memory.
	virtual void * &lpData();
	/// check if the plug is of type 'pt'
	bool		isTypeOfData(PlugType pt) const;
	/// \todo explain what it is...
	bool		getValidFlags(const BYTE **validarray, unsigned long * fields, unsigned long *fields2) const;
	//@}
	/// \name Operators
	//@{
	operator const PlugType() const;
	operator const String() const;
	operator const unsigned short() const;
	//@}
	/// \name management for R/W Events of in the IPlug.
	//@{
	/// tell that the plug has changed somewhere.
	void		setAsDirty(bool bRecurseChildren=true);
	/// check if the plug is changed
	bool		isDirty() { return m_bIsDirty; };
	/// flush the plug value to the connected ones, to the parent and to the children
	void		flush(bool bRecurseChildren=true, bool bRecurseParents=true);
	/** Plugs allow complex inter-connections of plugs. So, data can flow in many directions. Use this method to make sure the data got updated and propagated through connections.
	This is something you want to do 
	- when you turned off \e immediate \e Flush : plug_setImmediateFlush(false). Since the connected plugs didn't get updated, you'll have to ask for it thanks to this method.
	- when you changed a variable that is related to a plug (you gave its pointer). So the plug need to get this
	new value and propagate it to the connected ones.
	**/
	void		updateFlushes(bool bRecurseChildren=true, bool bRecurseParents=true);
	/** When you change the value of a plug, you have two strategies :
	- change the value and propagate it to the connected plugs (bYes=true). This is the default case.
	- change the value but don't propagate changes. You'll have to ask for flushing data by calling PlugContainer::plug_updateFlushes() : all the connected plugs will get updated.
	.
	solution 2 could be interesting to optimize the process when working frame by frame, for example. 
	If the connections of frame F depend on frame F-1, then you can postpone the connection update to the end of the frame job.
	**/
	void		setImmediateFlush(bool bYes);
	/// check the mode for flushing
	bool		isFlushingImmediate() { return m_bImmediateFlush; }
	/// you can set additional callback to handle incoming data for this plug. This is equivalent to handling data with PlugContainer::plug_IncomingData
	void		setIncomingDataCB(IncomingDataCB lpFn) { m_IncomingDataCB = lpFn; };
	/// you can set additional callback to handle outgoing data from this plug. This is equivalent to handling data with PlugContainer::plug_DataReadEvent
	void		setDataReadEventCB(DataReadEventCB lpFn) { m_DataReadEventCB = lpFn; };
	/// you can attach 2 unsigned long values to a plug for any purpose
	void		setUserLong(unsigned long ul1, unsigned long ul2 = 0) { m_userlong1 = ul1; m_userlong2 = ul2; }
	/// get back the user data
	void		getUserLong(unsigned long *ul1, unsigned long *ul2 = NULL) { if(ul1) *ul1 = m_userlong1; if(ul2) *ul2 = m_userlong2; }
	bool		plug_IncomingData(IPlug *lpSrc);
private:
	void*		DataReadEvent(int &idx, int *arraysize);
	//@}
public:
	bool operator==(LPCSTR name) const
	{
		if(!name) return false;
		return m_name == String(name);
	}
	/** \name Methods for reading values
	
	Use these methods to read the value(s) of a plug. These methods will perform data conversion if needed (reading a float plug as a string, for example)
	**/
	//@{
	void*		getPtr(int idx=0); ///< when you want a structure that you KNOW how it is made !!
	bool		getValues(void *val, int maxitems=0);
	bool		getValue(void *val, int idx=0);
	bool		getValue(BYTE &val, int idx=0);
	bool		getValue(int &val, int idx=0);
	bool		getValue(DWORD &val, int idx=0);
	bool		getValue(bool &val, int idx=0);
	bool		getValue(String &val, int idx=0);
	//bool		getValue(const float val[], int idx=0);
	bool		getValue(const BYTE val[], int idx=0);
	bool		getValue(float &val, int idx=0);
	bool		getValue(float &x, float &y, int idx=0);
	bool		getValue(float &x, float &y, float &z, int idx=0);
	bool		getValue(float &x, float &y, float &z, float &w, int idx=0);
	//@}
	/** \name Methods for writing values.

	safe ways to write values to a plug. Note that if the plug is attached to a variable pointer, you could set the attached variable and flush the plug (plug::IPlug::flush).
	But these methods do it for you and perform conversion if needed (setting a float plug with a boolean value, for example).

	\warning : these methods shouldn't be called frome plug::PlugContainer::plug_IncomingData. Infinite loop may appear...
	 **/
	//@{
	bool		setValue(void* ptr, int idx=0);
	bool		setValue(BYTE val, int idx=0);
	bool		setValue(int val, int idx=0);
	bool		setValue(DWORD val, int idx=0);
	bool		setValue(float val, int idx=0);
	bool		setValue(bool val, int idx=0);
	bool		setValue(String val, int idx=0);
	bool		setValue(LPCSTR pstr, int idx=0);
	bool		setValue(const float val[], int idx=0);
	bool		setValue(const BYTE val[], int idx=0);
	bool		setValue(float x, float y, int idx=0);
	bool		setValue(float x, float y, float z, int idx=0);
	bool		setValue(float x, float y, float z, float w, int idx=0);
	//@}
	friend PlugContainer;
};

} //namespace plug

#endif //__PLUG_H





