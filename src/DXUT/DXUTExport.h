#if NV_DXUT_DLL
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined __MINGW32__
#  ifdef NV_DXUT_EXPORTS
#    define NV_DXUT_API __declspec(dllexport)
#  else
#    define NV_DXUT_API __declspec(dllimport)
#  endif
#endif
#else
#  define NV_DXUT_API
#endif