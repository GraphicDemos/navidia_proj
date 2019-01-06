#ifndef NVIDIA_PNG_DEC
#define NVIDIA_PNG_DEC
 

#if NV_PNG_DLL
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined __MINGW32__
#  ifdef NV_PNG_EXPORTS
#    define NV_PNG_API __declspec(dllexport)
#  else
#    define NV_PNG_API __declspec(dllimport)
#  endif
#endif
#else
#  define NV_PNG_API
#endif
 

#endif
