#ifndef NVPARSE_H
#define NVPARSE_H

#define NVPARSE 1

#include <stdio.h>

#if NVPARSE_SHARED

#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined __MINGW32__
#  ifdef NVPARSE_EXPORTS
#    define NVPARSE_API __declspec(dllexport)
#  else
#    define NVPARSE_API __declspec(dllimport)
#  endif
#endif
#else
#    define NVPARSE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
	NVPARSE_API void nvparse(const char * input_string, ...);
	NVPARSE_API char * const * const nvparse_get_errors();
	NVPARSE_API char * const * const nvparse_print_errors(FILE *fp);
	NVPARSE_API const int* nvparse_get_info(const char* input_string, int* pcount);

#ifdef __cplusplus
}
#endif

#endif