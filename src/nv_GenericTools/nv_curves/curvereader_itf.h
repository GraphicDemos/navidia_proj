/**
This header is made of interfaces : abstract classes which are the basis for
some classes used to setup some curves.

The purpose of this abstraction is to allow some modules (i.e dll's or libs) to be able
to work with these objects without having the implementation : these interfaces will be
the only link between the module and the owner of these objects.

We really want the owner to be the implmentor of the methods.

For example, there is a scripting module as a dll which would need to wrap some Curve objects.
They would just need to store the pointer of these interfaces and let the original app to
implement the object.

 **/
#ifndef __CURVEREADERITF__
#define __CURVEREADERITF__

class ICurveReader;

//#if !defined(EtInfinityType)
enum EtInfinityType 
{
	kInfinityConstant,
	kInfinityLinear,
	kInfinityCycle,
	kInfinityCycleRelative,
	kInfinityOscillate
};
//#endif
enum EtTangentType 
{
	kTangentFixed,
	kTangentLinear,
	kTangentFlat,
	kTangentStep,
	kTangentSlow,
	kTangentFast,
	kTangentSmooth,
	kTangentClamped
};
typedef enum EtTangentType EtTangentType;

struct ReadKey
{
	float			time;
	float			value;
	EtTangentType	inTangentType;
	EtTangentType	outTangentType;
	float			inAngle;
	float			inWeight;
	float			outAngle;
	float			outWeight;
};
/**

  A curve vector is a set of curves params. Usually we are working with vectors. It is then more convenient to
  directly make this grouping available. Furthermore python scripting and plugs system will take advantage of this: manipulating
  and connecting is easier.

 **/
class ICurveVector
{
public:
	/// \name almost the same methods of Curve class but for a 'vector'
	//@{
	virtual bool find(float time, int *indexx, int *indexy, int *indexz, int *indexw) = 0;
	virtual float engineAnimEvaluate(float time, float *v1, float *v2, float *v3, float *v4) = 0;
	virtual float evaluateInfinities(float time, bool evalPre, float *v1, float *v2, float *v3, float *v4) = 0;
	virtual void addKey(float frame, float x, float y=0, float z=0, float w=0, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0) = 0;
	virtual void addKeyHere(EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0) = 0;
	//@}
	/// \name plug methods. using void * because maybe we don't want to use them
	//@{
	virtual void * getPlugIn() = 0; ///< here is the input plug : can be 'time' or other
	virtual void * getPlugOut(unsigned char comp=' ') = 0; ///< here is the plug output : either a scalar or a vector from 2..4 dim or a component of the vector ('x'...'w')
	virtual void disconnectPlugs(bool bIn=true, bool bOut=true) = 0; ///< to disconnect the plugs
	//@}
	virtual ICurveReader *getCurve(int n) = 0;
	virtual void clear(int n=-1) = 0;
};

/**

 This class is the main repository of all the curves we may need for an application.
 Here we store the curve vectors.

 **/
class ICurvePool
{
	virtual ICurveVector *newCV(const char * name, int dim, bool registertopython=true) = 0;
	virtual int getNumCV() = 0;
	virtual ICurveVector *getCV(const char *  name) = 0;
	virtual ICurveVector *newCVFromFile(char *  name, const char *  fname) = 0;
	virtual void clear() = 0;
};
/**

  ICurveReader is the basic curve data. It is the repository of the keys.

 **/
class ICurveReader
{
public:
	virtual void setName(const char * name) = 0;
	virtual void startKeySetup(bool inputIsTime=true, bool outputIsAngular=false, bool isWeighted=false,
						EtInfinityType preinftype=kInfinityConstant, EtInfinityType postinftype=kInfinityConstant) = 0;
	virtual void getKeySetup(bool &inputIsTime, bool &outputIsAngular, bool &isWeighted,
						EtInfinityType &preinftype, EtInfinityType &postinftype) = 0;
	virtual void addKey(float frame, float val, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0) = 0;
	virtual void endKeySetup() = 0;

	virtual int getNumKeys() = 0;
	virtual bool getKey(int n, ReadKey &k) = 0;
	virtual void clear() = 0;
	virtual bool delkey(int nkey) = 0;
};

#endif