#ifndef WebBrowserH
#define WebBrowserH

#include "CWebPage.h"	/* Declarations of the functions in DLL.c */


class WebBrowser
{
	public:
		WebBrowser(HWND parent);
		~WebBrowser();
		void DisplayHTMLPage(const char* url);
		void DoPageAction(DWORD action);


		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		HWND m_hWnd;
		HINSTANCE		m_cwebdll;
		bool m_Initialized;
};


#endif