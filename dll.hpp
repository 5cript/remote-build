#pragma once

#if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN32__)
#	include <windows.h>
#	ifdef BUILD_DLL
#		define DLL_EXPORT __declspec(dllexport)
#	elif defined(DYNAMIC_MARKUP_LIBRARY)
#		define DLL_EXPORT __declspec(dllimport)
#	else
#		define DLL_EXPORT
#	endif
#elif defined(__GNUC__)
#	define DLL_EXPORT __attribute__((visibility("default")))
#else
#	define DLL_EXPORT
#endif