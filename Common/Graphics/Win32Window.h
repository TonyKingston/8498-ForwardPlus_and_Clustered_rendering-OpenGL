/******************************************************************************
Class:Win32Window
Implements:Window
Author:Rich Davison
Description:TODO

-_-_-_-_-_-_-_,------,
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Window.h"

#ifdef _WIN32
#include "Core/Platform/Windows/MinWindows.h"
#include <io.h>
#include <stdio.h>
#include <fcntl.h>

#include "Input/Win32Mouse.h"
#include "Input/Win32Keyboard.h"

namespace NCL {
	namespace Win32Code {
#define WINDOWCLASS "WindowClass"

		class Win32Mouse;
		class Win32Keyboard;

		class Win32Window : public Window {
		public:
			friend class Window;
			void	LockMouseToWindow(bool lock)		override;
			void	ShowOSPointer(bool show)			override;
			void	SetConsolePosition(int x, int y)	override;
			void	ShowConsole(bool state)				override;
			void	SetFullScreen(bool state)			override;
			void	SetWindowPosition(int x, int y)		override;

			HWND		GetHandle()			const { return windowHandle; }
			HINSTANCE	GetInstance()		const { return windowInstance; }

		protected:
			Win32Window(const std::string& title, int sizeX, int sizeY, bool fullScreen, int offsetX, int offsetY);
			virtual ~Win32Window(void);

			bool	InternalUpdate()	override;
			void	UpdateTitle()		override;

			void					CheckMessages(MSG &msg);
			static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			HWND			windowHandle;
			HINSTANCE		windowInstance;

			bool			forceQuit;

			bool			active;
			bool			fullScreen;
			bool			lockMouse;
			bool			showMouse;
			bool			mouseLeftWindow;
			bool			maximised;
			Win32Mouse*		winMouse;
			Win32Keyboard*  winKeyboard;
		};
	}
}
#endif //_WIN32