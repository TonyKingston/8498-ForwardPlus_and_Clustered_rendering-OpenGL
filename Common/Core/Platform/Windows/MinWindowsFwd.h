#pragma once

/* Forward declarations for windows API types.
 Avoids having to include Windows.h in certain places. Similar to Unreal */

struct HWND__;
struct HKEY__;
struct HDC__;
struct HGLRC__;

#ifndef STRICT
#define STRICT
#endif


namespace Windows {
	typedef HWND__* HWND;
	typedef HKEY__* HKEY;
	typedef HDC__* HDC;
	typedef HGLRC__* HGLRC;
}