/**
\file curvereader.h

  This curve reader is made to work in conjunction with python scripting:
  we may be able to expose the curves to python and to edit them and play them.

  typical use:

	CurvePool curvepool;
	...
	curvepool.Init(spPython);

  the curves can be created by the python scripting in this way (through the maya nv_curves exporter):
  ---------------------------------------------
		e.newcv("rotation", 3)
		rotation.setup(1,1,0,"constant","constant")
		rotation[0].addkey(1.000000, 0.000000, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[0].addkey(81.000000, 50.421240, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[0].addkey(150.000000, 70, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[1].addkey(1.000000, 0.000000, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[1].addkey(81.000000, -54.414904, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[1].addkey(150.000000, -60, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[2].addkey(1.000000, 0.000000, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[2].addkey(81.000000, 30.000000, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation[2].addkey(150.000000, 10.000000, "smooth", "smooth", 1,1,0, 0,0, 0,0)
		rotation.compilekeys()
  ---------------------------------------------

**/ /******************************************************************************/
#ifndef __CURVEREADER__
#define __CURVEREADER__

//#pragma message("Note: including lib: nv_curves.lib\n")
//#pragma comment(lib,"nv_curves.lib")

#include "nv_curves/curvereader_itf.h"
#include "nv_curves/curveEngine.h"
#include "nv_rtmodules/nv_rtPythonItf.h"
#include "nv_plug/plug.h"
//#include "modhelper.h" ??

using namespace plug;
using namespace sapi;
using namespace std;

class CurvePool;
class CurveReader;
/*----------------------------------------------------------------------------------*/ /**

We are grouping curves so that we may have a vector from 1 to 4 dimensions.
This is interesting for connections with another plug which would be a vector

**/ //----------------------------------------------------------------------------------
class CurveVector : public ICurveVector, PlugContainer
{
protected:
	union 
	{
		CurveReader *m_cvs[4]; ///< the 1 to 4 curves
		struct 
		{
			CurveReader *m_cvx;
			CurveReader *m_cvy;
			CurveReader *m_cvz;
			CurveReader *m_cvw;
		};
	};
	int m_dim; ///< dimension of the 'vector'
	char * m_name; ///< the name. Pointed by CurvePool::CurveMapType
	/// \name plugs
	//@{
#	define CV_PLUG_IN 5
#	define CV_PLUG_OUT 0
#	define CV_PLUG_OUTX 1
#	define CV_PLUG_OUTY 2
#	define CV_PLUG_OUTZ 3
#	define CV_PLUG_OUTW 4
	plug::Plug plug_time;
	plug::Plug plug_cv;
	plug::Plug plug_cvx;
	plug::Plug plug_cvy;
	plug::Plug plug_cvz;
	plug::Plug plug_cvw;

	float m_time, m_cvvals[4];
	//@}
public:
	CurveVector(LPCSTR name, int dim);
	~CurveVector();
	/// \name almost the same methods of Curve class but for a 'vector'
	//@{
	bool find(float time, int *indexx, int *indexy, int *indexz, int *indexw);
	float engineAnimEvaluate(float time, float *v1, float *v2, float *v3, float *v4);
	float evaluateInfinities(float time, bool evalPre, float *v1, float *v2, float *v3, float *v4);
	void addKey(float frame, float x, float y, float z, float w, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	void addKeyHere(EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	//@}
	/// \name methods from PlugContainer
	//@{
	///bool		Incoming_Composite(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent=true);
	virtual		bool plug_IncomingData(IPlug *lpSrc, IPlug *lpDst, bool bSendEvent=true);
	virtual		void *plug_DataReadEvent(IPlug *lpPlug, int &idx);
	//@}
	/// \name plug methods. using void * because maybe we don't want to use them
	//@{
	virtual void * getPlugIn();
	virtual void * getPlugOut(unsigned char comp=' ');
	virtual void disconnectPlugs(bool bIn=true, bool bOut=true);
	//@}
	virtual ICurveReader *getCurve(int n);
	virtual void clear(int n=-1);

	friend CurvePool;
};
/*----------------------------------------------------------------------------------*/ /**

- Implementation of the external Python function for curve declaration
- contains a pool of allocated curves
.

**/ //----------------------------------------------------------------------------------
class CurvePool : public ICurvePool, IEventsPython
{
public:
	/// \name from IEventsPython
	//@{
	virtual LPCSTR ListOfAvailableCmds(IPython *);
	virtual bool OutputString(IPython *, LPCSTR) { return false; };
	virtual bool OutputError(IPython *, LPCSTR) { return false; };
	virtual void clear();
	virtual bool FuncCall(LPCSTR funcname, 
		int strresultsz, char *strresult, 
		int nstrings, const char * const *sptr,  
		int nfloats, const float *fptr);
	//@}
	ICurveVector *newCV(const char *  name, int dim, bool registertopython=true);
	ICurveVector *newCVFromFile(char *  name, const char *  fname);
	int getNumCV() {return (int)m_curves.size(); };
	ICurveVector *getCV(const char *  name);
	bool Init(IPython *iPython);
	LPCSTR getLastError() { return m_lasterr.c_str(); }
	~CurvePool();
private:
	struct ltstr
	{
	  bool operator()(LPCSTR s1, LPCSTR s2) const
	  {
		return strcmp(s1, s2) < 0;
	  }
	};
	typedef std::map<LPCSTR, CurveVector*, ltstr> CurveMapType;
	CurveMapType m_curves;

	SmartPtr<IPython> m_pythonitf;
	String	m_lasterr;
};


class CurveReader : public ICurveReader, public Curve
{
public:
	CurveReader();
	~CurveReader() {};
protected:
	//sapiString m_name;
	
	EtTangentType AsTangentType (String &str);
	bool assembleAnimCurve(vector<ReadKey> &keys, bool isWeighted, bool useOldSmooth);

	vector<ReadKey> m_keys; ///< keys which define the curve
	float			m_unitConversion;
	float			m_frameRate;
	
public:
	virtual void setName(const char *  name) {}
	virtual void startKeySetup(bool inputIsTime=true, bool outputIsAngular=false, bool isWeighted=false,
						EtInfinityType preinftype=kInfinityConstant, EtInfinityType postinftype=kInfinityConstant);
	virtual void getKeySetup(bool &inputIsTime, bool &outputIsAngular, bool &isWeighted,
						EtInfinityType &preinftype, EtInfinityType &postinftype);
	virtual void addKey(float frame, float val, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	virtual void endKeySetup();

	virtual int getNumKeys() { return (int)m_keys.size(); };
	bool getKey(int n, ReadKey &k);
	virtual void clear();
	virtual bool delkey(int nkey);

	friend CurveVector;
};
#endif
