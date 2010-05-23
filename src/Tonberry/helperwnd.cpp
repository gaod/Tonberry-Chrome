#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#include "helperwnd.h"
static const char szClassName[] = "HelperWnd_Class";


/*#define LWA_COLORKEY 0x00000001
#define LWA_ALPHA    0x00000002
#define WS_EX_LAYERED 0x00080000

typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;*/

static const unsigned int ALPHA_BLEND = 180;


LRESULT CALLBACK CHelperWnd::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int alpha = ALPHA_BLEND;

    switch(msg)
    {
 		/*case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = (HDC)BeginPaint( hwnd, &ps );
				EndPaint( hwnd, &ps );
				return 0;
			}
			break;*/

		case WM_SHOWWINDOW:
			{
//				if ( SetLayeredWindowAttributes )
//				{					
					if (!wp)
					{
						for ( int nPercent=100; nPercent >= 0 ;nPercent-- )
						{
							if(alpha<=0)
								break;
							alpha = ALPHA_BLEND * nPercent/100;
							SetLayeredWindowAttributes( hwnd, 0, alpha, LWA_ALPHA );
							SuperSleep(0.001);
						}
					}else{
						SetLayeredWindowAttributes( hwnd, 0, ALPHA_BLEND, LWA_ALPHA);
					}
					return 0;
//				}
			}
			break;


		case WM_COMMAND:
			{
				SendMessage((HWND)GetWindowLong(hwnd, GWL_USERDATA), WM_COMMAND, wp, lp);
			}
			break;

		case WM_NOTIFY: 
			switch (((LPNMHDR) lp)->code) 
			{ 
			case TTN_GETDISPINFO: 
				{ 
					LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lp; 
            
					UINT_PTR idButton = lpttt->hdr.idFrom; 
					switch (idButton) 
					{ 
					case ID_HELPER_COPY: 
						lpttt->lpszText = "Copy Selected Text"; 
						break; 
					case ID_HELPER_SEARCH: 
						lpttt->lpszText = "Open with Search Engine"; 
						break; 
					case ID_HELPER_SURL_0RZ:
						lpttt->lpszText = "0rz.tw"; 
						break;
					case ID_HELPER_SURL_TINYURL:
						lpttt->lpszText = "Tinyurl.com"; 
						break;
					case ID_HELPER_COPYPASTE:
						lpttt->lpszText = "Copy and Paste";
						break;
					case ID_HELPER_COPYANSI:
						lpttt->lpszText = "Copy Text with ANSI Color";
						break;
					case ID_HELPER_SELECTALL:
						lpttt->lpszText = "Select All";
						break;
						
					case ID_HELPER_CLOSE: 
						lpttt->lpszText = "Kill!"; 
						break; 
					} 
				} 
			}
			break;			

    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

CHelperWnd::~CHelperWnd()
{
	Destroy();
}

void CHelperWnd::Destroy()
{
	DestroyWindow(m_hWnd);
}

CHelperWnd::CHelperWnd(HWND parent)
{
    HWND hwnd;

	WNDCLASS wc = {0};
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wc.lpfnWndProc = (WNDPROC)CHelperWnd::WndProc;
	wc.lpszClassName = szClassName;
	wc.style = 0;

    RegisterClass(&wc);

	const int w = 164;//140
	const int h = 40;

    hwnd = CreateWindowEx(
        (WS_EX_TOPMOST | WS_EX_TOOLWINDOW),
        szClassName,
        "HelperWnd",
        WS_POPUP , 
        CW_USEDEFAULT, CW_USEDEFAULT, w, h,
        parent, NULL, wc.hInstance, NULL);

	m_hWnd = hwnd;
	SetWindowLong(hwnd, GWL_USERDATA, (long)parent);

    // Create the toolbar.
    const int ImageListID = 0;
    const int numButtons = 8;//
    const DWORD buttonStyles = BTNS_AUTOSIZE;
    const int bitmapSize = 16;


    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
        WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        m_hWnd, NULL, wc.hInstance, NULL);


    // Create the imagelist.
    HIMAGELIST hImageList = ImageList_Create(
        bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
        ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
        numButtons, 0);

    SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ImageListID, 
        (LPARAM)hImageList);

    SendMessage(hWndToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, 
        (LPARAM)HINST_COMMCTRL);

    // Initialize button info.
    TBBUTTON tbButtons[numButtons] = 
    {
        { MAKELONG(STD_COPY, ImageListID), ID_HELPER_COPY, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"Copy" },

        { MAKELONG(STD_FIND, ImageListID), ID_HELPER_SEARCH, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"Search"},

        { MAKELONG(STD_FILENEW, ImageListID), ID_HELPER_SURL_0RZ, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"O"},

        { MAKELONG(STD_FILENEW, ImageListID), ID_HELPER_SURL_TINYURL, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"T"},

        { MAKELONG(STD_PASTE, ImageListID), ID_HELPER_COPYPASTE, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"P"},

        { MAKELONG(STD_FILENEW, ImageListID), ID_HELPER_COPYANSI, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"A"},

        { MAKELONG(STD_FILENEW, ImageListID), ID_HELPER_SELECTALL, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"L"},


        { MAKELONG(STD_DELETE, ImageListID), ID_HELPER_CLOSE, TBSTATE_ENABLED, 
          buttonStyles, {0}, 0, (INT_PTR)L"X"}
    };

    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, 
        (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, 
        (LPARAM)&tbButtons);

	HRGN hrgn =  ::CreateRoundRectRgn(0, 0, w+1, h+1, 16, 16 );
	SetWindowRgn( hwnd, hrgn, true);
	DeleteObject (hrgn);

//	HMODULE hUser32 = GetModuleHandle("user32.dll");
//	SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)
//	GetProcAddress(hUser32,"SetLayeredWindowAttributes");
//	if (SetLayeredWindowAttributes)
//	{
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, 0, ALPHA_BLEND, LWA_ALPHA);
//	}
//	FreeLibrary(hUser32);    

	//LONG curStyle=GetWindowLong (hwnd,GWL_STYLE);
	//SetWindowLong(hwnd, GWL_STYLE, curStyle&(~WS_CAPTION));

    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0); 
    ShowWindow(hWndToolbar, TRUE);


	ShowWindow(hwnd, FALSE);
	UpdateWindow(hwnd);

}
