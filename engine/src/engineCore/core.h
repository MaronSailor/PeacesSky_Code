#pragma once

#ifdef CE_PLATFORM_WINDOWS
	#ifdef CE_BUILD_DLL
		#define CUSTOMENGINE_API __declspec(dllexport)
	#else
		#define CUSTOMENGINE_API __declspec(dllimport)
	#endif
#endif

#ifdef CE_PLATFORM_LINUX
	#ifdef CE_BUILD_DLL
		#define CUSTOMENGINE_API __attribute__((visibility("default")))
	#else
		#define CUSTOMENGINE_API 
	#endif
#endif