#pragma once

#define _WIN32_WINNT 0x0501

#pragma warning(disable: 4275) // non dll-interface class '...' used as base for dll-interface class '...'
#pragma warning(disable: 4251) // class '...' needs to have dll-interface to be used by clients of class '...'
#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4355) // 'this' used in base member initializer list
#pragma warning(disable: 4244) // conversion from int to float

#pragma warning(push)
#pragma warning(disable: 4267 4312)

#include <d3d9.h>
#include <windows.h>
#include <mmsystem.h>
#include <atlbase.h>
#include <d3d9types.h>
#include <streams.h>
#include <shlobj.h>
#include <dvdmedia.h>
#include <gdiplus.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <tchar.h>

#include <string>
#include <deque>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <set>
#include <algorithm>
#include <iomanip>
#include <utility>
#include <cassert>

#define GLEW_STATIC 1
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <GL/glew.h>
#include <GL/wglew.h>

#include <direct.h>

#pragma warning(pop)

#undef min
#undef max

#include <glh/glh_linear.h>

#define MATH2_NO_SIMD 1
//#include <math/matrix.tpp>

#include <vector>

using std::auto_ptr;
using std::make_pair;

// predefined components:
typedef glh::vec<1, float> t1f;
typedef glh::vec2f t2f;
typedef glh::vec3f t3f;
typedef glh::vec4f t4f;

typedef glh::vec3f n3f;    // normal component
typedef glh::vec<3, unsigned char> c3ub;    // color component
typedef glh::vec<4, unsigned char> c4ub;    // color component
typedef glh::vec3f c3f;             // float color component
typedef glh::vec4f c4f;             // float color component
typedef glh::vec<3, unsigned char> s3ub;    // 2ndary color component
typedef glh::vec<4, unsigned char> s4ub;    // 2ndary color component
typedef glh::vec3f s3f;             // float 2ndary color component
typedef glh::vec4f s4f;             // float 2ndary color component

typedef glh::vec<1, float> v1f; // for position
typedef glh::vec2f v2f; // for position
typedef glh::vec3f v3f; // for position
typedef glh::vec4f v4f; // for position

typedef glh::vec<2, bool> vec2b;
typedef glh::vec<3, bool> vec3b;
typedef glh::vec<4, bool> vec4b;

typedef glh::vec<2, int> vec2i;
typedef glh::vec<3, int> vec3i;
typedef glh::vec<4, int> vec4i;

typedef vec2i vec2;
typedef vec3i vec3;
typedef vec4i vec4;

//using namespace math;

class noncopyable
{
 protected:
    noncopyable() {}
    ~noncopyable() {}
 private:  // emphasize the following members are private
    noncopyable( const noncopyable& );
    const noncopyable& operator=( const noncopyable& );
};


extern const std::wstring default_ns;

inline void ShowFinalError(std::wstring err)
{
	MessageBoxW(0, err.c_str(), L"CoremagePBO Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);

    _exit(1);
}

inline void ShowAssertion(const char *msg, const char *file, long line, const char *sig)
{
	std::wostringstream ss;
	ss << "Mediocre has encountered a fatal error.\n\n"
		<< "Assertion failed: " << msg
		<< "\n\nFunction: " << sig
		<< "\nLocation: " << file << "(" << line << ")";
	ShowFinalError(ss.str());
}


#define rassert(x) do { if (!(x)) ShowAssertion(#x, __FILE__, __LINE__, __FUNCSIG__); } while (0)
#define xassert rassert

template<class Type>
struct rect {
	typedef std::vector<Type[2]> vec;
	vec pos, size;

	rect() : pos(0), size(0) {}
	rect(const vec &p, const vec &s) : pos(p), size(s) {}
	template<class T>
	explicit rect(const rect<T> &r)
		: pos(vec(r.pos)), size(vec(r.size)) {}

	bool operator==(const rect &r) const
		{ return pos == r.pos && size == r.size; }
	bool operator!=(const rect &r) const { return !(*this == r); }
};

std::wstring leafName(const std::wstring &path);
std::wstring extension(const std::wstring &path);
std::wstring tolower(std::wstring);
std::wstring indent(std::size_t n);
std::wstring escape_attribute(std::wstring);
void replace(std::wstring &, const std::wstring &from, const std::wstring &to);

extern std::wstring g_AppDir, g_StartDir;

inline int round(float x) { return int(x+0.5); }

inline int sqr(int x) { return x*x; }
inline float sqr(float x) { return x*x; }

inline int highbit(unsigned x)
{
	int n = -1;
	while (x) {
		x >>= 1;
		++n;
	}
	return n;
}

inline unsigned ispow2(unsigned x) { return (1 << highbit(x)) == x; }
inline unsigned nextpow2(unsigned x) { return ispow2(x) ? x : (1 << (highbit(x)+1)); }

inline long double refToFloat(long long time)
	{ return static_cast<long double>(time)/(1000*1000*10); }
inline long long floatToRef(long double time)
	{ return static_cast<long long>(time*1000*1000*10); }

inline long long sign(long long x) { if (!x) return 0; else return x>0 ? 1 : -1; }
