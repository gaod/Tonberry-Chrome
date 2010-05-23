

#include <windows.h>

#include "WebBrowser.h"





// A running count of how many windows we have open that contain a browser object
unsigned char WindowCount = 0;

// The class name of our Window to host the browser. It can be anything of your choosing.
static const TCHAR	ClassName[] = "Browser";

// Where we store the pointers to CWebPage.dll's functions
EmbedBrowserObjectPtr		*lpEmbedBrowserObject;
UnEmbedBrowserObjectPtr		*lpUnEmbedBrowserObject;
DisplayHTMLPagePtr			*lpDisplayHTMLPage;
DisplayHTMLStrPtr			*lpDisplayHTMLStr;
ResizeBrowserPtr			*lpResizeBrowser;
DoPageActionPtr				*lpDoPageAction;





/****************************** WindowProc() ***************************
 * Our message handler for our window to host the browser.
 */

LRESULT CALLBACK WebBrowser::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			ShowWindow( hwnd, SW_HIDE );
			(*lpDoPageAction)(hwnd, WEBPAGE_STOP);
			(*lpDisplayHTMLPage)(hwnd, "about:blank");
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			return 0;
		}

		case WM_SIZE:
		{
			// Resize the browser object to fit the window
			if (lpResizeBrowser)
			{	
				(*lpResizeBrowser)(hwnd, LOWORD(lParam), HIWORD(lParam));
				return(0);
			}
		}

		case WM_CREATE:
		{
			// Embed the browser object into our host window. We need do this only
			// once. Note that the browser object will start calling some of our
			// IOleInPlaceFrame and IOleClientSite functions as soon as we start
			// calling browser object functions in EmbedBrowserObject().
			if ((*lpEmbedBrowserObject)(hwnd)) return(-1);

			// Another window created with an embedded browser object
			++WindowCount;

			return(0);
		}

		case WM_DESTROY:
		{
			// Detach the browser object from this window, and free resources.
			(*lpUnEmbedBrowserObject)(hwnd);

			// One less window
			--WindowCount;

			/*// If all the windows are now closed, quit this app
			if (!WindowCount) PostQuitMessage(0);*/

			return(TRUE);
		}

	}

	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}


WebBrowser::WebBrowser(HWND parent)
{
	WNDCLASSEX		wc;

	// Load our DLL containing the OLE/COM code. We do this once-only. It's named "cwebpage.dll"
	if ((m_cwebdll = LoadLibrary("cwebpage.dll")))
	{
		// Get pointers to the EmbedBrowserObject, DisplayHTMLPage, DisplayHTMLStr, and UnEmbedBrowserObject
		// functions, and store them in some globals.

		// Get the address of the EmbedBrowserObject() function. NOTE: Only Reginald has this one
		lpEmbedBrowserObject = (EmbedBrowserObjectPtr *)GetProcAddress((HINSTANCE)m_cwebdll, EMBEDBROWSEROBJECTNAME);

		// Get the address of the UnEmbedBrowserObject() function. NOTE: Only Reginald has this one
		lpUnEmbedBrowserObject = (UnEmbedBrowserObjectPtr *)GetProcAddress((HINSTANCE)m_cwebdll, UNEMBEDBROWSEROBJECTNAME);

		lpDisplayHTMLPage = (DisplayHTMLStrPtr *)GetProcAddress((HINSTANCE)m_cwebdll, DISPLAYHTMLPAGENAME);

		lpDisplayHTMLStr = (DisplayHTMLStrPtr *)GetProcAddress((HINSTANCE)m_cwebdll, DISPLAYHTMLSTRNAME);

		lpResizeBrowser = (ResizeBrowserPtr *)GetProcAddress((HINSTANCE)m_cwebdll, RESIZEBROWSERNAME);

		lpDoPageAction = (DoPageActionPtr *)GetProcAddress((HINSTANCE)m_cwebdll, DOPAGEACTIONNAME);


		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
		wc.lpfnWndProc = WebBrowser::WndProc;
		wc.lpszClassName = &ClassName[0];
		RegisterClassEx(&wc);

		HWND hwnd = CreateWindowEx( WS_EX_TOOLWINDOW, &ClassName[0], "Quick View", WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT, 600, 300,
							parent, NULL, (HINSTANCE)GetModuleHandle(NULL), 0) ;
		
		m_hWnd = hwnd;

		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, 0, 240, LWA_ALPHA);
		
		ShowWindow(hwnd, SW_HIDE);
		UpdateWindow(hwnd);

		m_Initialized = true;
	}
	else
		m_Initialized = false;

}

WebBrowser::~WebBrowser()
{
	if (m_Initialized)
		FreeLibrary(m_cwebdll);
}

void WebBrowser::DisplayHTMLPage(const char* url)
{
	(*lpDisplayHTMLPage)(m_hWnd, url);
}

void WebBrowser::DoPageAction(DWORD action)
{
	(*lpDoPageAction)(m_hWnd, action);
}

