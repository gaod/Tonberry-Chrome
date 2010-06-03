// TelnetView.cpp: implementation of the CTelnetView class.
//
/////////////////////////////////////////////////////////////////////////////
// Name:        telnetcon.cpp
// Purpose:     Class dealing with telnet terminal screen, the central black view.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.16
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
  #pragma implementation "telnetview.h"
#endif

// for WM_MOUSEWHEEL support
#define _WIN32_WINNT  0x0500
#include <windows.h>
// ---

#pragma comment (lib, "Urlmon.lib" )

#include "telnetview.h"
#include "telnetcon.h"
#include "helperwnd.h"
#include "WebBrowser.h"
//#include "resource.h"

#include "stringutil.h"
#include <tchar.h>
#include <malloc.h>
#include "CommCtrl.h"	//for _TrackMouseEvent()

//extern WebBrowser* g_browser;

using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////




LRESULT CTelnetView::WndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch( msg )
	{

	case WM_COMMAND:
		if ( m_helperWnd )
				OnCommand( wp, lp );
		break;

	case WM_KILLFOCUS:
		{
			if ( m_helperWnd )
			{
				if ( wp != (int)this->m_hWnd ){
					if ( wp != (int)m_helperWnd->m_hWnd) 
						ShowWindow(m_helperWnd->m_hWnd, FALSE);
				}
			}
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		{
			if( OnSysKeyDown(int(wp), long(lp)) )
				return 0;
		}
		break;

	/*case WM_MOUSELEAVE:
		{
			OnMouseLeave(wp, lp);
		}
		break;*/

	/*case WM_SETFOCUS:
		{
			if( wp == (int)m_parent )
					SetFocus( m_hWnd );

		}
		break;*/

	case WM_INPUTLANGCHANGE:
		{
			this->m_imeProp = ImmGetProperty((HKL)lp, IGP_PROPERTY);
			return 0;
		}
	case WM_IME_COMPOSITION:
		{
			if (lp & GCS_RESULTSTR)
			{
				if (m_imeProp & IME_PROP_UNICODE)
					return _OnImeCompositionW(wp, lp);
				else
					return _OnImeCompositionA(wp, lp);
			}
		}
		break;
	case WM_IME_CHAR:
		{
			OnImeChar(wp, lp);
			return 0;
		}
	case WM_CHAR:
		OnChar(int(wp), long(lp));
		return 0;
	case WM_SOCKET:
		{
			int code = HIWORD(lp);
			switch( LOWORD(lp) )
			{
			case FD_READ:
				GetCon()->OnRecv(code);
				break;
			case FD_CONNECT:
				GetCon()->OnConnect(code);
				break;
			case FD_CLOSE:
				GetCon()->OnClose(code);
			};
		}
		return 0;
	case WM_KEYDOWN:
		if( OnKeyDown(int(wp), long(lp)) )
			return 0;
		break;
	case WM_CONTEXTMENU:
		if ( m_canPopup && !m_RbtnInUse )
			OnContextMenu(MAKEPOINTS(lp));
		return 0;
	case WM_SHOWWINDOW:
		if( wp )
			SetFocus( m_hWnd );
		break;
	case WM_GETHOSTBYNAME:
		GetCon()->OnGetHost(lp);
	}
	return CTermView::WndProc( msg, wp, lp );
}


CTelnetView::CTelnetView(HWND hWnd) : CTermView(hWnd)
{
	/*if ( GetACP() == 950 )
	{
		m_Popup = LoadMenu( (HINSTANCE)GetModuleHandle("nppcman.dll"), LPCTSTR(IDR_POPUP_CHT) );
		m_Popup = GetSubMenu( m_Popup, 0 );
		m_Popup_Nonsel = LoadMenu( (HINSTANCE)GetModuleHandle("nppcman.dll"), LPCTSTR(IDR_POPUP_NONSEL_CHT) );
		m_Popup_Nonsel = GetSubMenu( m_Popup_Nonsel, 0 );
	}else{
		m_Popup = LoadMenu( (HINSTANCE)GetModuleHandle("nppcman.dll"), LPCTSTR(IDR_POPUP) );
		m_Popup = GetSubMenu( m_Popup, 0 );
		m_Popup_Nonsel = LoadMenu( (HINSTANCE)GetModuleHandle("nppcman.dll"), LPCTSTR(IDR_POPUP_NONSEL) );
		m_Popup_Nonsel = GetSubMenu( m_Popup_Nonsel, 0 );
	}*/


	m_canPopup = true;
	m_bTrackLeave = false;

	m_initSetGesturesX = false;
	m_initUseHelperWnd = false;

	m_MouseGestures.Reset();


	m_helperWnd = NULL;

	//FIXME: below is temporarily
	m_keyFuncSwitch = false;
	m_RbtnInUse = false;
	m_MouseWheelInUse = false;
}

CTelnetView::~CTelnetView()
{
	if (m_helperWnd)
		delete m_helperWnd;
}


static bool IsURLSupported( string url ) {
	return ( 0 == strncmpi( url.c_str(), "http", 4) ||
			 0 == strncmpi( url.c_str(), "ftp:", 4) );
}

static void OpenWithMozilla( string url ) {
	ShellExecute( NULL, "open", url.c_str(), NULL, NULL, SW_SHOW );
}

void CTelnetView::OnChar(int ch, long data)
{
	char _ch = (char)ch;
    if( m_pTermData )
	{
		switch( ch )
		{
		case '\r':
			break;
		case '\b':
			break;
		default:
			((CTelnetCon*)m_pTermData)->Send(&_ch,1);
		}
	}
}

bool CTelnetView::OnKeyDown(int key, long data)
{
	if ( _onkeydown_chrome(key, data) )
		return true;

	else if ( _onkeydown(key, data) )
		return true;
	else
		return false;
}

bool CTelnetView::_onkeydown(int key, long data)
{
	if(!m_pTermData)
		return false;

	CTelnetCon* pTelnet = (CTelnetCon*)m_pTermData;

	CTermCharAttr* pAttr = m_pTermData->GetLineAttr(
			m_pTermData->m_Screen[m_pTermData->m_CaretPos.y] );
	int x = m_pTermData->m_CaretPos.x;

	switch(key)
	{
	case VK_RETURN:
		pTelnet->Send(pTelnet->m_Site.GetCRLF(),strlen(pTelnet->m_Site.GetCRLF()));
		break;
	case VK_LEFT:
//	case WXK_NUMPAD_LEFT:
		if( x > 0 && pAttr[x-1].GetCharSet() == CTermCharAttr::CS_MBCS2
			&& !(pTelnet->m_Site.m_AutoDbcsDetection & 0x2) )
			pTelnet->Send("\x1bOD\x1bOD",6);
		else
			pTelnet->Send("\x1bOD",3);
		break;
	case VK_RIGHT:
//	case WXK_NUMPAD_RIGHT:
		if( pAttr[x].GetCharSet() == CTermCharAttr::CS_MBCS1
			&& !(pTelnet->m_Site.m_AutoDbcsDetection & 0x2) )
			pTelnet->Send("\x1bOC\x1bOC",6);
		else
			pTelnet->Send("\x1bOC",3);
		break;
	case VK_UP:
//	case WXK_NUMPAD_UP:
		pTelnet->Send("\x1bOA",3);
		break;
	case VK_DOWN:
//	case WXK_NUMPAD_DOWN:
		pTelnet->Send("\x1bOB",3);
		break;
	case VK_BACK:
		if( x > 0 && pAttr[x-1].GetCharSet() == CTermCharAttr::CS_MBCS2
			&& !(pTelnet->m_Site.m_AutoDbcsDetection & 0x8) )
			pTelnet->Send("\b\b",2);
		else
			pTelnet->Send("\b",1);
		return true;
		break;
	case VK_DELETE:
		if( pAttr[x].GetCharSet() == CTermCharAttr::CS_MBCS1
			&& !(pTelnet->m_Site.m_AutoDbcsDetection & 0x4) )
			pTelnet->Send("\x1b[3~\x1b[3~",8);
		else
			pTelnet->Send("\x1b[3~",4);
		break;
	case VK_INSERT:
		pTelnet->Send("\x1b[2~",4);
		break;
	case VK_PRIOR:
//	case WXK_NUMPAD_PAGEUP:
		pTelnet->Send("\x1b[5~",4);
		break;
	case VK_NEXT:
//	case WXK_NUMPAD_PAGEDOWN:
		pTelnet->Send("\x1b[6~",4);
		break;
	case VK_END:
		pTelnet->Send("\x1b[4~",4);
		break;
	case VK_HOME:
//	case WXK_NUMPAD_HOME:
		pTelnet->Send("\x1b[1~",4);
		break;
	}
	return true;
}


/*!
    \fn CTelnetView::OnSendStringAsync(wxCommandEvent& evt)
	Process Async SendString request from CTelnetCon.
 */


bool CTelnetView::_onkeydown_chrome(int key, long data)
{
	if ( GetKeyState(VK_CONTROL) & 0x8000 )
	{
		// CTRL + SHIFT + ...
		if ( GetKeyState(VK_LSHIFT) & 0x8000 || GetKeyState(VK_RSHIFT) & 0x8000 ){
			switch (key){
				case VK_TAB:

				SetFocus( GetParent(GetParent(m_hWnd) ));
				SendMessage( GetParent(GetParent(m_parent)), WM_KEYDOWN, (WPARAM)key, (LPARAM)data);
				return true;
				break;
			}
		}

		// CTRL + ...
		switch (key)
		{

		//------------------------------------------------
		case VK_PRIOR:
		case VK_NEXT:
		case VK_TAB:
//		case 0x54: // 'T' key
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
		case VK_NUMPAD8:
		case VK_NUMPAD9:
//		case 0x57: // 'W' key
		case VK_F4:
		case VK_SUBTRACT:
		case VK_ADD:
		case 0x30:
//		case 0x52: // 'R' key
//		case 0x4c: // 'L' key

			SetFocus( GetParent(GetParent(m_hWnd) ));
			SendMessage( GetParent(GetParent(m_parent)), WM_KEYDOWN, (WPARAM)key, (LPARAM)data);
			return true;
			break;
		//------------------------------------------------

		case VK_RIGHT:		
			SendMessage( GetParent(GetParent(m_parent)), WM_KEYDOWN, (WPARAM)VK_NEXT, (LPARAM)data);
			return true;
			break;
		case VK_LEFT:
			SendMessage( GetParent(GetParent(m_parent)), WM_KEYDOWN, (WPARAM)VK_PRIOR, (LPARAM)data);
			return true;
			break;
		}
	}
	else
	{
		switch (key)
		{

		case VK_F11:
		case VK_F5:
		case VK_F6:

			SetFocus( GetParent(GetParent(m_hWnd) ));
			SendMessage( GetParent(GetParent(m_parent)), WM_KEYDOWN, (WPARAM)key, (LPARAM)data);
			return true;
			break;
		}
	}


	return false;
}



bool CTelnetView::OnSysKeyDown(int key, long data)
{
	if ( GetKeyState(VK_LMENU) & 0x8000 || GetKeyState(VK_RMENU) & 0x8000)
	{
		switch (key)
		{
			case 0x4f: // 'O' key
				CopySelectedText( false );
				break;

			case 0x50: // 'P' key
				OnPaste();
				break;							
		}
		return true;
	}
	return false;
}

void CTelnetView::OnSendStringAsync(string str)
{
//	((wxSocketClient*)evt.GetClientData())->Write(str.c_str(), str.length());
}

void CTelnetView::NewCon(CSite& si)
{
	if( !si.m_FontFace.empty() )
	{
		LOGFONT lf;
		GetObject( m_Font, sizeof(lf), &lf );
		strcpy( lf.lfFaceName, si.m_FontFace.c_str() );
		HFONT nf = CreateFontIndirect( &lf );
		if( nf )
		{
			DeleteObject( m_Font );
			m_Font = nf;
			RecalcCharExtent();
		}	
	}

	if( m_pTermData )
		delete m_pTermData;
	m_pTermData = new CTelnetCon( this, si );

	GetCon()->AllocScreenBuf( si.m_RowsPerPage, si.m_RowsPerPage, si.m_ColsPerPage );
	GetCon()->Connect();

	RECT rc;
	GetClientRect( m_hWnd, &rc );
	OnSize( 0, rc.right, rc.bottom );
}

static bool CompareVersion(string& new_ver, string& old_ver)
{
	const int max_len = 32;

	if ( new_ver.length() > max_len || old_ver.length() > max_len )
		return false;

	char chr_new [max_len];
	char chr_old [max_len];

	strcpy( chr_new, new_ver.c_str() );
	strcpy( chr_old, old_ver.c_str() );

	int tok_new[max_len] = {0};
	int tok_old[max_len] = {0};


	char* token = strtok( chr_new, ".");
	for ( int i = 0; token != NULL; i++ )
	{
		tok_new[i] = atoi( token );
		token = strtok( NULL, ".");
	}

	token = strtok( chr_old, ".");
	for ( int j = 0; token != NULL; j++ )
	{
		tok_old[j] = atoi( token );
		token = strtok( NULL, ".");
	}


	for ( int k = 0; k < max_len; k++ )
	{
		if ( tok_new[k] > tok_old[k] ){
			return true;
		}

		else if ( tok_new[k] < tok_old[k] ){
			return false;
		}
	}

	return false;
}

static bool GetRemoteVersionInfo(string& info)
{
	char buf[2048];
	char file_path[MAX_PATH];


	HRESULT ret = URLDownloadToCacheFile(NULL,
		"http://pcmanfx.openfoundry.org/app/ver.txt",
		file_path,
		MAX_PATH,
		0,
		NULL);

	if ( ret != S_OK )
		return false;

	FILE* pFile = fopen(file_path, "r");
	if ( !pFile )
		return false;

	int receive = fscanf(pFile, "%s", buf);
	fclose (pFile);

	if ( receive<=0 )
		return false;

	info = buf;
	return true;
}

static bool CheckUpdate(string& new_ver)
{
	if ( GetRemoteVersionInfo(new_ver) )
	{
		char buf[512];
		//-LoadString(GetModuleHandle("nppcman.dll"), IDS_APP_VERSION, buf, 512);
		string old_ver(buf);

		if ( CompareVersion( new_ver, old_ver ) )
			return true;
	}
	return false;
}

static HWND g_hDlg = NULL;
static const int ID_CHECK_TIMER = 99;

static DWORD WINAPI ThreadFunc(LPVOID p)
{
	/*string ver;
	string text;
	HWND hwnd;


		if ( g_hDlg )
			KillTimer( g_hDlg, ID_CHECK_TIMER );


		if ( CheckUpdate(ver) )
		{
			text = "有新版本 " + ver + " 可以安裝";

			if ( g_hDlg )
			{
				//-SetDlgItemText(g_hDlg, IDC_STATIC_MSG, text.c_str());
				hwnd = GetDlgItem( g_hDlg, IDC_BUTTON1 );
				EnableWindow(hwnd, true);
				ShowWindow(hwnd, true);
				//-SendDlgItemMessage( g_hDlg, IDC_STATIC_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(GetModuleHandle("nppcman.dll"), MAKEINTRESOURCE(IDI_ICON2)) );
			}
		}
		else
		{
			text = "目前已是最新版本";

			if ( g_hDlg )
			{
				SetDlgItemText(g_hDlg, IDC_STATIC_MSG, text.c_str());
				hwnd = GetDlgItem( g_hDlg, IDC_BUTTON1 );
				EnableWindow(hwnd, false);
				ShowWindow(hwnd, false);
				//-SendDlgItemMessage( g_hDlg, IDC_STATIC_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(GetModuleHandle("nppcman.dll"), MAKEINTRESOURCE(IDI_ICON1)) );
			}
		}*/
	return 0;
}

BOOL AboutDlgProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	HANDLE hThread = INVALID_HANDLE_VALUE;
	DWORD exit_code = 0;
	DWORD thread_id;

	if( msg == WM_COMMAND )
	{
		switch( LOWORD(wp) )
		{
			//-case IDC_BUTTON1:
				//-OpenWithMozilla("http://of.openfoundry.org/projects/941/download");
//				break;

			case IDOK:
			case IDCANCEL:
				if (hThread)
					CloseHandle(hThread);
				g_hDlg = NULL;
				EndDialog(hwnd, LOWORD(IDOK) );				
				break;
		}
	}

	else if( msg == WM_TIMER )
	{
		switch (wp)
		{

		case ID_CHECK_TIMER:
			hThread = CreateThread(NULL,
				0,
				ThreadFunc,
				(LPVOID)1,
				0,
				&thread_id);
			break;
		}
	}

	else if( msg == WM_INITDIALOG )
	{
		g_hDlg = hwnd;
		SetTimer( hwnd, ID_CHECK_TIMER, 50, NULL );

		return TRUE;
	}

	else if( msg == WM_DESTROY )
	{
		g_hDlg = NULL;
	}
	return 0;
}

void CTelnetView::NotifyOpenWithMozilla( string url, bool background )
{

/*	long lh = long(m_hWnd);
	char buf[16];
	ltoa(lh, buf, 10);
	wchar_t wh[16];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wh, 16, buf, _TRUNCATE);


	nsCString nsStr;
	NS_CStringSetData( nsStr, url.c_str() );
	m_nsSupportsCString_1->SetData( nsStr );


	if ( !background )
		m_nsObserverService->NotifyObservers( m_nsSupportsCString_1, "onLoadNewTab", wh );
	else
		m_nsObserverService->NotifyObservers( m_nsSupportsCString_1, "onLoadNewTabBk", wh );*/
}

void CTelnetView::OnContextMenu(POINTS pt)
{
	/*if (GetCon()->m_Site.m_UseMouseBrowsing){
		CheckMenuItem(m_Popup, ID_USE_MOUSE_BROWSING, MF_CHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_USE_MOUSE_BROWSING, MF_CHECKED);
	}else{
		CheckMenuItem(m_Popup, ID_USE_MOUSE_BROWSING, MF_UNCHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_USE_MOUSE_BROWSING, MF_UNCHECKED);
	}

	if (GetCon()->m_Site.m_UseTextDragDrop){
		CheckMenuItem(m_Popup, ID_USE_TEXT_DRAG_DROP, MF_CHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_USE_TEXT_DRAG_DROP, MF_CHECKED);
	}else{
		CheckMenuItem(m_Popup, ID_USE_TEXT_DRAG_DROP, MF_UNCHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_USE_TEXT_DRAG_DROP, MF_UNCHECKED);
	}

	if ( this->m_keyFuncSwitch ){
		CheckMenuItem(m_Popup, ID_SWITH_KEY_FUNC, MF_CHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_SWITH_KEY_FUNC, MF_CHECKED);
	}else{
		CheckMenuItem(m_Popup, ID_SWITH_KEY_FUNC, MF_UNCHECKED);
		CheckMenuItem(m_Popup_Nonsel, ID_SWITH_KEY_FUNC, MF_UNCHECKED);
	}

	UINT cmd;

	bool textSelected = false;
	if (m_pTermData){
		textSelected = !( m_pTermData->m_SelStart.x == m_pTermData->m_SelEnd.x && m_pTermData->m_SelStart.y == m_pTermData->m_SelEnd.y );
	}


	if (textSelected)
		cmd = TrackPopupMenu( m_Popup, TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL );
	else
		cmd = TrackPopupMenu( m_Popup_Nonsel, TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL );

	

	//cmd = TrackPopupMenu( m_Popup, TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL );
	switch( cmd )
	{
	case ID_COPY:
		CopySelectedText( false );
		break;
	case ID_COPYANSI:
		CopySelectedText( true );
		break;
	case ID_PASTE:
		OnPaste();
		break;
	case ID_COPYPASTE:
		CopySelectedText( false );
		OnPaste();
		break;
	case ID_COPYLINK:
		CopyLinkLocation(pt);
		break;
	case ID_SELECTALL:
		if(!m_pTermData)
			return;
		m_pTermData->m_SelStart.x = 0;
		m_pTermData->m_SelStart.y = m_pTermData->m_FirstLine;
		m_pTermData->m_SelEnd.x = m_pTermData->m_ColsPerPage;
		m_pTermData->m_SelEnd.y = m_pTermData->m_RowCount-1;
		InvalidateRect( m_hWnd, NULL, false );
		break;

	case ID_OPENSEARCH1:
		this->m_searchInfo_1.search_terms = this->m_pTermData->GetSelectedText();
		OpenWithSearchEngine(&m_searchInfo_1);
		break;
	case ID_OPENSEARCH2:
		this->m_searchInfo_2.search_terms = this->m_pTermData->GetSelectedText();
		OpenWithSearchEngine(&m_searchInfo_2);		
		break;

	case ID_ABOUT:
		//-DialogBox( (HINSTANCE)GetModuleHandle("nppcman.dll"), LPCTSTR(IDD_ABOUT), 
			//-m_hWnd, (DLGPROC)AboutDlgProc );
		break;

	case ID_USE_MOUSE_BROWSING:
		GetCon()->m_Site.m_UseMouseBrowsing = !GetCon()->m_Site.m_UseMouseBrowsing;
		break;

	case ID_USE_TEXT_DRAG_DROP:
		if ( ! (GetCon()->m_Site.m_UseTextDragDrop = !GetCon()->m_Site.m_UseTextDragDrop))
			m_beginDrag = false;
		break;

	case ID_SWITH_KEY_FUNC:
		this->m_keyFuncSwitch = !m_keyFuncSwitch;
		break;

	case ID_SET_ENCODING_DEFAULT:
		SetCodePage( ::GetACP() );
		InvalidateRect( m_hWnd, NULL, false );
		break;
	case ID_SET_ENCODING_CP950:
		SetCodePage(950);
		InvalidateRect( m_hWnd, NULL, false );
		break;
	case ID_SET_ENCODING_CP936:
		SetCodePage(936);
		InvalidateRect( m_hWnd, NULL, false );
		break;
	}*/
	SetFocus( m_hWnd );
}

void CTelnetView::SendString(string str)
{
	if( !GetCon() || GetCon()->m_State & CTelnetCon::TS_CLOSED )
		return;

	// Check if there are ESC control sequences in the string which have to be replaced.
	// by corresponding substutes such as double ESCs or Ctrl + U.
	string esc = UnEscapeStr( GetCon()->m_Site.m_ESCConv.c_str() );
	esc += '[';
	bool hasControlChar = str_replace( str, "\x1b[", esc.c_str() );

	// Only when no control character is in this string can autowrap be enabled
	unsigned int len = 0, max_len = GetCon()->m_Site.m_AutoWrapOnPaste;
	if( !hasControlChar && GetCon()->m_Site.m_AutoWrapOnPaste > 0 )
	{
		string str2;
		const char* pstr = str.c_str();
		for( ; *pstr; pstr++ )
		{
			size_t word_len = 1;
			const char* pword = pstr;
			if( ((unsigned char)*pstr) < 128 )		// This is a ASCII character
			{
				if( *pstr == '\n' )
					len = 0;
				else
				{
					while( *pstr &&  ((unsigned char)*(pstr+1)) < 128  && !strchr(" \t\n\r", *pstr) )
						pstr++;
					word_len = (pstr - pword) + 1;
				}
			}
			else
			{
				pstr++;
				word_len = ( *pstr ? 2 : 1 );
			}

			if( (len + word_len) > max_len )
			{
				len = 0;
				str2 += '\n';
			}

			len += word_len;
			while( pword <= pstr )
			{
				str2 += *pword;
				pword ++;
			}
		}
		str = str2;
	}

	/* Replace CRLFs to '\n' or 'r' according to keyboard mapping. */
	// 'str' should be ConvertFromCRLF before passed into this function.
//	str.Replace("\n", pCon->m_Site.GetCRLF() );

	GetCon()->SendString( str );
}

void CTelnetView::OnCommand(WPARAM wp, LPARAM lp)
{
	int id = LOWORD(wp); 
	int event = HIWORD(wp);


	switch (id)
	{
	case ID_HELPER_COPY:
		CopySelectedText( false );
		CancelSelectedBlock();
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		break;

	case ID_HELPER_SEARCH:
		//-NS_openWithSearchEngine(-1);
		this->m_searchInfo_1.url = "http://www.google.com/search";
		this->m_searchInfo_1.param_name = "q";
		this->m_searchInfo_1.search_terms = this->m_pTermData->GetSelectedText();
		this->OpenWithSearchEngine(&m_searchInfo_1);
//		CancelSelectedBlock();
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		break;

	case ID_HELPER_SURL_0RZ:
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		OpenShorterUrl(string("http://0rz.tw"));
		break;

	case ID_HELPER_SURL_TINYURL:
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		OpenShorterUrl(string("http://tinyurl.com"));
		break;

	case ID_HELPER_COPYPASTE:
		CopySelectedText( false );
		OnPaste();
		CancelSelectedBlock();
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		break;

	case ID_HELPER_COPYANSI:
		CopySelectedText( true );
		CancelSelectedBlock();
		ShowWindow(m_helperWnd->m_hWnd, FALSE);
		break;

	case ID_HELPER_SELECTALL:
		if(!m_pTermData)
			return;
		m_pTermData->m_SelStart.x = 0;
		m_pTermData->m_SelStart.y = m_pTermData->m_FirstLine;
		m_pTermData->m_SelEnd.x = m_pTermData->m_ColsPerPage;
		m_pTermData->m_SelEnd.y = m_pTermData->m_RowCount-1;
		InvalidateRect( m_hWnd, NULL, false );
		break;


	case ID_HELPER_CLOSE:
		delete this->m_helperWnd;
		m_helperWnd = NULL;
		break;
	}

}

void CTelnetView::OpenShorterUrl(string url)
{
	string terms = m_pTermData->GetSelectedText();
	str_replace(terms, " ", "+");
	string post = url + "/" + terms;

	ShellExecute( NULL, "open", post.c_str(), NULL, NULL, SW_SHOW );
}

void CTelnetView::OnLButtonDown(UINT flags, POINTS& pt)
{
	SetFocus(m_hWnd);

	if( !m_pTermData )
		return;
	SetCapture(m_hWnd);
	m_IsSelecting = true;
	int x = pt.x;
	int y = pt.y;
	this->PointToLineCol( &x, &y, true );

	this->m_textSelected = !( m_pTermData->m_SelStart.x == m_pTermData->m_SelEnd.x && m_pTermData->m_SelStart.y == m_pTermData->m_SelEnd.y );

	// 2008.07.03 Added by neopro@ptt.cc
	// Execute commands by using drag and drop gestures.
	if ( GetCon()->m_Site.m_UseTextDragDrop )
	{

		if ( (m_pTermData->m_SelStart.x != m_pTermData->m_SelEnd.x)
			&& (x >= m_pTermData->m_SelStart.x && x <=m_pTermData->m_SelEnd.x)
			&& (y >= m_pTermData->m_SelStart.y && y <=m_pTermData->m_SelEnd.y)
			)
		{
			m_oldDragCursor = GetCursor();
			SetCursor( m_DragtextCursor );
			m_beginDrag = true;
			m_IsSelecting = false;
			return;
		}
	}
	//	end of the function. (drag and drop gestures)

	bool need_refresh = ( m_pTermData->m_SelStart.x != m_pTermData->m_SelEnd.x
						|| m_pTermData->m_SelStart.y != m_pTermData->m_SelEnd.y );
	m_pTermData->m_SelStart.x = m_pTermData->m_SelEnd.x = x;
	m_pTermData->m_SelStart.y = m_pTermData->m_SelEnd.y = y;
	if( need_refresh )
		InvalidateRect( m_hWnd, NULL, false );
//	m_pTermData->m_SelBlock = evt.ShiftDown();
	m_pTermData->m_SelBlock = ((flags&MK_CONTROL)||(flags&MK_SHIFT));
}


void CTelnetView::OnMouseWheel(WPARAM wp, LPARAM lp)
{

	if ( GET_KEYSTATE_WPARAM(wp) & MK_RBUTTON )
	{
		if ( m_keyFuncSwitch )
		{

			if ( GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA > 0){
				SetFocus( GetParent(GetParent(m_hWnd) ));
				keybd_event(VK_CONTROL,0,0,0);
				keybd_event(VK_PRIOR,0,0,0);
				keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);
				keybd_event(VK_PRIOR,0,KEYEVENTF_KEYUP,0);
			}else if(GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA < 0){
				SetFocus( GetParent(GetParent(m_hWnd) ));
				keybd_event(VK_CONTROL,0,0,0);
				keybd_event(VK_NEXT,0,0,0);
				keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);
				keybd_event(VK_NEXT,0,KEYEVENTF_KEYUP,0);
			}
		}else
		{
			if ( !m_pTermData )
				return;

			m_RbtnInUse = true;
			m_MouseWheelInUse = true;

			if ( GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA > 0 )
				((CTelnetCon*)m_pTermData)->Send("\x1b[5~",4);
			else if(GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA < 0 )
				((CTelnetCon*)m_pTermData)->Send("\x1b[6~",4);			
		}
	}

	else
	{
		if ( GetCon()->m_Site.m_UseMouseBrowsing )
		{

			int ps = ((CTelnetCon*)m_pTermData)->GetPageState();

			//if ( !_duplicate /*|| ps==1*/ )
				CTermView::OnMouseWheel( wp, lp );
		}
	}

}

void CTelnetView::OnLButtonUp(UINT flags, POINTS& pt)
{
	if( !m_pTermData )
		return;

	ReleaseCapture();
	m_IsSelecting = false;

	int x = pt.x;
	int y = pt.y;
	this->PointToLineCol( &x, &y );

	// 2008.07.03 Added by neopro@ptt.cc
	// Execute commands by using drag and drop gestures.
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	bool inside = (x >= m_pTermData->m_SelStart.x && x <=m_pTermData->m_SelEnd.x) && (y >= m_pTermData->m_SelStart.y && y <=m_pTermData->m_SelEnd.y);
	bool largeBlock = ( (m_pTermData->m_SelStart.x == 0) && (m_pTermData->m_SelEnd.y == (m_pTermData->m_RowCount-1)) )||
		((m_pTermData->m_SelStart.x == m_pTermData->m_ColsPerPage) && (m_pTermData->m_SelEnd.y == 0) );

	if ( m_beginDrag && largeBlock )
	{
		m_pTermData->m_SelStart.x = m_pTermData->m_SelStart.y = m_pTermData->m_SelEnd.x = m_pTermData->m_SelEnd.y = 0;
		InvalidateRect( m_hWnd, NULL, false );
		m_beginDrag = false;
	}

	if ( m_beginDrag &&  !inside )
	{			
		if ( (x <= m_pTermData->m_SelStart.x || x >=m_pTermData->m_SelEnd.x)
			|| (y <= m_pTermData->m_SelStart.y || y >=m_pTermData->m_SelEnd.y) )
		{
			if ((y > m_pTermData->m_SelEnd.y) || (pt.y >= rc.bottom))
				OnTextDragDrop(0);
			else if ((y < m_pTermData->m_SelStart.y) || (pt.y <= rc.top))
				OnTextDragDrop(1);
			else if ((x >= m_pTermData->m_SelEnd.x) || (pt.x >= rc.right))
				OnTextDragDrop(2);
			else if ((x <= m_pTermData->m_SelStart.x) || (pt.x <= rc.left))
				OnTextDragDrop(3);
		}
	}
	SetCursor( m_oldDragCursor );
	m_beginDrag = false;
	//	end of drag and drop gestures support.

	bool loadInBk = GetCon()->m_Site.m_LoadUrlInBackGround;

	//	2004.08.07 Added by PCMan.  Hyperlink detection.
	//	If no text is selected, consider hyperlink.
	if( m_pTermData->m_SelStart.x == m_pTermData->m_SelEnd.x 
		&& m_pTermData->m_SelStart.y == m_pTermData->m_SelEnd.y)
	{
		if ( flags & MK_CONTROL )
			loadInBk = !loadInBk;				

		// *beginning of the Hyperlink detection.
		char* pline = m_pTermData->m_Screen[y];
		CTermCharAttr* pattr = m_pTermData->GetLineAttr(pline);
		if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
		{
			int start, end;
			for( start = x-1; start > 0 && pattr[start].IsHyperLink(); start-- );
			if( !pattr[start].IsHyperLink() )
				start++;
			for( end = x+1; end < m_pTermData->m_ColsPerPage && pattr[end].IsHyperLink(); end++ );
			string URL( (pline+start), (int)(end-start) );
			if( IsURLSupported(URL) )
			{
				OpenWithMozilla(URL);
//				NotifyOpenWithMozilla( URL, loadInBk );
			}
			else{
				ShellExecute( m_hWnd, "open", URL.c_str(), NULL, NULL, SW_SHOW );
			}
		}
		// *end of the Hyperlink detection.

		// 2008.07.01 Added by neopro@ptt.cc (porting form PCMan X pure GTK+ 2)
		// beginning of mouse support
		// -----------------------
		if ( GetCon()->m_Site.m_UseMouseBrowsing && !this->m_textSelected)
		{

			int start, end;
			// Don't send mouse action when the user click on hyperlinks
			if( HyperLinkHitTest( x, y, &start, &end ) )
							return;

			int cur = m_CursorState;
			int ps = ((CTelnetCon*)m_pTermData)->GetPageState();
    
			if ( cur == 2 ) // mouse on entering mode
			{
				switch (ps)
				{
				case 1: // list
					{        
					int n = y - m_pTermData->m_CaretPos.y; 
					if ( n>0 )
						while(n)
						{
							((CTelnetCon*)m_pTermData)->Send("\x1bOB",3);
							n--;
						}
					if ( n<0 )
						{
						n=-n;   
						while(n)
							{
							((CTelnetCon*)m_pTermData)->Send("\x1bOA",3); 
							n--;
							}
						}                                      
					((CTelnetCon*)m_pTermData)->Send("\r",1); //return key
					break;
					}
				case 0: // menu
					{
					char cMenu = ((CTelnetCon*)m_pTermData)->GetMenuChar(y);
					((CTelnetCon*)m_pTermData)->Send( &cMenu, 1 );
					((CTelnetCon*)m_pTermData)->Send( "\r", 1 );    
					break;      
					} 
				case -1: // normal
					((CTelnetCon*)m_pTermData)->Send( "\r", 1 );   
					break;                       
				default:
					break;      
				}     
			}
			else if (cur == 1)
			((CTelnetCon*)m_pTermData)->Send("\x1bOD",3); //exiting mode
			else if (cur == 6)
			((CTelnetCon*)m_pTermData)->Send("\x1b[1~",4); //home
			else if (cur == 5)
			((CTelnetCon*)m_pTermData)->Send("\x1b[4~",4); //end
			else if (cur == 4)
			((CTelnetCon*)m_pTermData)->Send("\x1b[5~",4); //pageup
			else if (cur == 3)
			((CTelnetCon*)m_pTermData)->Send("\x1b[6~",4); //pagedown
			else
			((CTelnetCon*)m_pTermData)->Send( "\r", 1 );
			// end of mouse support
		}



	}

//	Correct the selected area when users stop selecting.
//  If the user selected from bottom to top or from right to left, we should swap the
//	coordinates to make them correct.
	CorrectSelPos(m_pTermData->m_SelStart.x, m_pTermData->m_SelStart.y, 
		m_pTermData->m_SelEnd.x, m_pTermData->m_SelEnd.y);


	if ( GetCon()->m_Site.m_UseHelperWnd )
	{
		if ( !m_initUseHelperWnd )
		{			
			m_helperWnd = new CHelperWnd(m_hWnd);
			m_initUseHelperWnd = true;
		}

		if ( m_helperWnd )
		{
			if (!( m_pTermData->m_SelStart.x == m_pTermData->m_SelEnd.x && m_pTermData->m_SelStart.y == m_pTermData->m_SelEnd.y ))
			{
				POINT _pt;
				_pt.x = pt.x;
				_pt.y = pt.y;
				ClientToScreen(m_hWnd, &_pt);
				const int h_helper = 40; //the helper-window's height

				SetWindowPos(m_helperWnd->m_hWnd, NULL, _pt.x, _pt.y - h_helper, 0,0,SWP_NOZORDER | SWP_NOSIZE );
				ShowWindow(m_helperWnd->m_hWnd, SW_SHOW);
				SetFocus( m_hWnd );
			}else{
				ShowWindow(m_helperWnd->m_hWnd, FALSE);
			}
		}

	}// end if (m_UseHelperWnd)

}

/*LPARAM CTelnetView::OnMouseLeave(WPARAM wp, LPARAM lp)
{
	m_bTrackLeave = FALSE;
	//在這裡添加處理鼠標離開的代碼 ：
	//
    return 0;
}*/

void CTelnetView::OnMouseMove(UINT flags, POINTS& pt)
{
	if( !m_pTermData )
		return;

	/*
	//*hack for NPAPI plugins*
    //滑鼠第一次移入窗口時， 請求一個WM_MOUSELEAVE 消息(在窗口內移動時，不請求)
	if ( !m_bTrackLeave )
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		_TrackMouseEvent(&tme);

		SetFocus(m_hWnd);
		m_bTrackLeave = TRUE;
	}
	//*/


	int x = pt.x;
	int y = pt.y;
	this->PointToLineCol( &x, &y );

	static int prev_y = -1;
	bool is_new_posy = (prev_y - y) != 0;


	// 2009.01.20 Added
	//支援中文的座標計算
	int cx = 0;
	cx = (pt.x / m_CharW);

	char* tmpstr = m_pTermData->m_Screen[y];
	if (cx > 0 && IsBig5(tmpstr, cx - 1))	//如果選擇中文後半段，就選取下一個字
		cx++;
	else if (!IsBig5(tmpstr, cx) && (pt.x % m_CharW)*2 > m_CharW)	//如果也不是中文前半，才是英文
		cx++;

	if ( cx > m_pTermData->m_ColsPerPage )
		cx = m_pTermData->m_ColsPerPage;

	if ( pt.x < m_CharW )
		cx = 0;


	if( m_IsSelecting )	//	Selecting text.
	{
		int oldx = m_pTermData->m_SelEnd.x;
		int oldy = m_pTermData->m_SelEnd.y;
		//	Store new SelPos
		m_pTermData->m_SelEnd.x = cx;
		m_pTermData->m_SelEnd.y = y;
		//	Update Display
		RedrawSel( oldx, oldy, x, y );

		SetCursor(m_ArrowCursor);
		m_CursorState=0;
	}
	else	//	Consider hyperlink detection. / mouse browsing support
	{
		CTermCharAttr* pattr = m_pTermData->GetLineAttr(m_pTermData->m_Screen[ y ]);


		// FIXME:  modified for mouse support.
		//
		/* Todo: hyperlink detection. */

		bool isHyperlink = x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink();

		if ( !GetCon()->m_Site.m_UseMouseBrowsing )
		{
			if( isHyperlink )
				SetCursor(m_HandCursor);
			else
				SetCursor(m_ArrowCursor);
			m_CursorState = 0;
		}

		// 2009.01.20 Added by neopro@ptt.cc
		/*if ( isHyperlink ){ //2010.03.21
			EnableMenuItem(m_Popup, ID_COPYLINK, MF_ENABLED);
			EnableMenuItem(m_Popup_Nonsel, ID_COPYLINK, MF_ENABLED);
		}else{
			EnableMenuItem(m_Popup, ID_COPYLINK, MF_GRAYED);
			EnableMenuItem(m_Popup_Nonsel, ID_COPYLINK, MF_GRAYED);
		}*/

		// 2008.07.01 Added by neopro@ptt.cc (porting form PCMan X pure GTK+ 2)
		// Mouse browsing support
		if ( GetCon()->m_Site.m_UseMouseBrowsing )
		{
			int option = GetCon()->m_Site.m_MouseOverEffect;
			if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
			{ SetCursor(m_HandCursor);m_CursorState=-1; }
			else
			{
				switch( ((CTelnetCon*)m_pTermData)->GetPageState() )
				{
				case -1: //NORMAL
					SetCursor(m_ArrowCursor);
					m_CursorState = 0;
					break;
				case 1: //LIST
					if ( y>2 && y < m_pTermData->m_RowsPerPage-1 )
					{
					if ( x <= 6 )
					{SetCursor(m_ExitCursor);m_CursorState=1;}
					else if ( x >= m_pTermData->m_ColsPerPage-16 )
					{
						if ( y > m_pTermData->m_RowsPerPage /2 )
						{SetCursor(m_PageDownCursor);m_CursorState=3;}
						else
						{SetCursor(m_PageUpCursor);m_CursorState=4;}
					}					
					else
						{
							SetCursor(m_BullsEyeCursor);m_CursorState=2;
							if ( is_new_posy )
							{
								if (option==0)
									break;

								if (m_pTermData->IsLineEmpty(y))
									break;

								CTermCharAttr* pold_attr =  new CTermCharAttr [ m_pTermData->m_ColsPerPage ];
								for( int col = 0; col < m_pTermData->m_ColsPerPage; col ++ ){

									//-------------save old attr-------------
									if(option==1) // Effect type 1
										pold_attr[col].SetBackground( pattr[col].GetBackground() );
									else if(option==2){ // Effect type 2
										pold_attr[col].SetBright( pattr[col].IsBright() );
										pold_attr[col].SetUnderLine( pattr[col].IsUnderLine() );
									}
									else if(option==3) // Effect type 3
										pold_attr[col].SetInverse( pattr[col].IsInverse() );

									//-------------set with new attr-------------
									if(option==1) // Effect type 1
										pattr[col].SetBackground( 2 );
									else if(option==2){ // Effect type 2
										pattr[col].SetBright( true );
										pattr[col].SetUnderLine( true );
									}
									else if(option==3) // Effect type 3
										pattr[col].SetInverse( true );
									pattr[col].SetNeedUpdate(true);
								}

								m_pTermData->UpdateDisplay();
							
								for( int col = 0; col < m_pTermData->m_ColsPerPage; col ++ ){
									//-------------restore-------------
									if(option==1)
										pattr[col].SetBackground( pold_attr[col].GetBackground() );
									else if(option==2){
										pattr[col].SetBright( pold_attr[col].IsBright() );
										pattr[col].SetUnderLine( pold_attr[col].IsUnderLine() );
									}
									else if(option==3)
										pattr[col].SetInverse( pold_attr[col].IsInverse() );
									pattr[col].SetNeedUpdate(true);
								}
								delete [] pold_attr;
							}
						}
					}      
					else if ( y==1 || y==2 )
					{SetCursor(m_PageUpCursor);m_CursorState=4;}
					else if ( y==0 ) 
					{SetCursor(m_HomeCursor);m_CursorState=6;}
					else //if ( y = m_pTermData->m_RowsPerPage-1) 
					{SetCursor(m_EndCursor);m_CursorState=5;}
					break;
				case 2: //READING
					if ( y == m_pTermData->m_RowsPerPage-1)
					{SetCursor(m_EndCursor);m_CursorState=5;}
					else if ( x<7 )
					{SetCursor(m_ExitCursor);m_CursorState=1;}
					else if ( y < (m_pTermData->m_RowsPerPage-1)/2 )
					{SetCursor(m_PageUpCursor);m_CursorState=4;}
					else
					{SetCursor(m_PageDownCursor);m_CursorState=3;}
					break;  
				case 0: //MENU
					if ( y>0 && y < m_pTermData->m_RowsPerPage-1 )
					{
					if (x>7)
					{SetCursor(m_BullsEyeCursor);m_CursorState=2;}
					else
					{SetCursor(m_ExitCursor);m_CursorState=1;}
					}      
					else    
					{SetCursor(m_ArrowCursor);m_CursorState=0;}
					break;
				default:
					break;
				}
			}
		}

		// 2008.07.03 Added by neopro@ptt.cc  *Drag and drop gestures
		if ( this->m_beginDrag )
			SetCursor( m_DragtextCursor );

		// 2008.07.31 Added by neopro@ptt.cc
		// Mouse Gestures support (FIXME: dirty & poor)
		//if ( m_MouseGestures.beginAction )
		if ( flags & MK_RBUTTON && GetCon()->m_Site.m_UseMouseBrowsing)
		{
			int currAction = m_MouseGestures.DoAction(pt);
			if ((currAction != m_MouseGestures.prev_action) && (currAction!=-1))
			{
				if (currAction != m_MouseGestures.prev_added)
				{
					m_MouseGestures.NewAction(currAction);
					m_MouseGestures.prev_added = currAction;
				}
			}
			m_MouseGestures.prev_action = currAction;						
		}
		// end of Mouse Gestures support

		}

		prev_y = y;
}
void CTelnetView::OnRButtonDown(UINT flags, POINTS& pt)
{
	m_RbtnInUse = false;
	m_MouseWheelInUse = false;

	if ( !GetCon()->m_Site.m_UseMouseBrowsing || !m_pTermData)
		return;

	if ( !m_initSetGesturesX ){
		m_MouseGestures.SetSensitivityX( ((CTelnetCon*)m_pTermData)->m_Site.m_GesturesSensitivityX );
		m_initSetGesturesX = true;
	}
}

void CTelnetView::OnRButtonUp(UINT flags, POINTS& pt)
{
	if ( !GetCon()->m_Site.m_UseMouseBrowsing )
		return;

	bool useXAxisOnly = GetCon()->m_Site.m_GesturesUseXAxisOnly;

	/*string str = m_MouseGestures.GetActions();
	MessageBox( m_hWnd, str.c_str(), "Error",  MB_ICONSTOP|MB_OK );*/

	int act0 = m_MouseGestures.GetActions(0);
	int act1 = m_MouseGestures.GetActions(1);
	int act2 = m_MouseGestures.GetActions(2);

	if (act0==3 && act1==-1)
	{
		m_canPopup = false;
		GetCon()->Send("\x1bOD",3);
	}

	else if (act0==2 && act1==-1)
	{
		m_canPopup = false;
		GetCon()->Send("\r",1);
	}

	else if (act0==2 && act1==1 /*&& act2==-1*/ && !useXAxisOnly)
	{
		m_canPopup = false;
		GetCon()->Send("\x1b[1~",4);
	}

	else if (act0==2 && act1==0 /*&& act2==-1*/ && !useXAxisOnly)
	{
		m_canPopup = false;
		GetCon()->Send("\x1b[4~",4);
	}

	else if (act0==0 && act1==-1/* && act2==-1*/ && !useXAxisOnly)
	{
		m_canPopup = false;
		GetCon()->Send("+",1);
	}

	else if (act0==1 && act1==-1/* && act2==-1*/ && !useXAxisOnly)
	{
		m_canPopup = false;
		GetCon()->Send("-",1);
	}

	else if (act0==2 && act1==3 && act2==2)
	{
		m_canPopup = false;
		GetCon()->Send("X",1);
	}

	// no gestures
	else{
		m_canPopup = true;

		//-------------------------------------------------------------------------------------------------------
		/*if ( g_browser && !m_MouseWheelInUse )
		{
			if( !m_pTermData )
				return;

			int x = pt.x;
			int y = pt.y;
			this->PointToLineCol( &x, &y );

			// Hyperlink detection.
			char* pline = m_pTermData->m_Screen[y];
			CTermCharAttr* pattr = m_pTermData->GetLineAttr(pline);
			if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
			{		
				int start, end;
				for( start = x-1; start > 0 && pattr[start].IsHyperLink(); start-- );
				if( !pattr[start].IsHyperLink() )
					start++;
				for( end = x+1; end < m_pTermData->m_ColsPerPage && pattr[end].IsHyperLink(); end++ );
				string URL( (pline+start), (int)(end-start) );

				if( IsURLSupported(URL) )
				{
					POINT _pt;
					_pt.x = pt.x;
					_pt.y = pt.y;
					ClientToScreen(m_hWnd, &_pt);
					const int h = 300; //window's height
					SetWindowPos(g_browser->m_hWnd, NULL, _pt.x, _pt.y - h, 0,0,SWP_NOZORDER | SWP_NOSIZE );
					ShowWindow( g_browser->m_hWnd, SW_SHOW );
					g_browser->DisplayHTMLPage( URL.c_str() );
				}
			}else{
				ShowWindow( g_browser->m_hWnd, SW_HIDE );
				g_browser->DoPageAction(WEBPAGE_STOP);
				g_browser->DisplayHTMLPage( "about:blank" );
				SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			}
		}*/
		//-------------------------------------------------------------------------------------------------------
	}//

	m_MouseGestures.Reset();
}

void CTelnetView::OnMButtonUp(UINT flags, POINTS& pt)
{
	if( !m_pTermData )
		return;

	int x = pt.x;
	int y = pt.y;
	this->PointToLineCol( &x, &y );

	// *Hyperlink detection.
	char* pline = m_pTermData->m_Screen[y];
	CTermCharAttr* pattr = m_pTermData->GetLineAttr(pline);
	if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
	{
		
		int start, end;
		for( start = x-1; start > 0 && pattr[start].IsHyperLink(); start-- );
		if( !pattr[start].IsHyperLink() )
			start++;
		for( end = x+1; end < m_pTermData->m_ColsPerPage && pattr[end].IsHyperLink(); end++ );
		string URL( (pline+start), (int)(end-start) );
		if( IsURLSupported(URL) ){
			OpenWithMozilla(URL);
//			NotifyOpenWithMozilla( URL, !(GetCon()->m_Site.m_LoadUrlInBackGround) );
		}
		else{
			ShellExecute( m_hWnd, "open", URL.c_str(), NULL, NULL, SW_SHOW );
		}
	}		
}

void CTelnetView::OnPaste() 
{
			if( !OpenClipboard(m_hWnd) )
				return;
			HGLOBAL hmem = (HGLOBAL)GetClipboardData( CF_UNICODETEXT );
			if( !hmem )
				return;

			const wchar_t* pwtext=(const wchar_t*)GlobalLock( hmem );
			int len = wcslen(pwtext)*sizeof(wchar_t) + 1;//in bytes
			char* ptext = new char[ len ];
			memset(ptext, 0, len);

			if (this->GetCodePage() == 950)
				m_ucs2conv.Ucs22Big5(pwtext, ptext);
			else
				::WideCharToMultiByte(GetCodePage(), 0, pwtext, -1,ptext, len, NULL, NULL);
			string text = ptext;
			delete [] ptext;
			GlobalUnlock( hmem );
			CloseClipboard();
	//		ConvertFromCRLF(text);	//Replace all line break with a single '\n'
			text+='\n';
			SendString( text );	// All control characters are processed in SendString().
	//		MessageBox( m_hWnd, "Not Completed yet.", "Error",  MB_ICONSTOP|MB_OK );
}

LRESULT CTelnetView::_OnImeCompositionW(WPARAM wparam, LPARAM lparam)
{
		HIMC hIMC = ImmGetContext(m_hWnd);
		if(!hIMC)
			return 0;

		DWORD size =
			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0); 

		if ( size < 1 || size > 1024 )
			return 0;

		size += sizeof(wchar_t); 

		wchar_t* pwcsData = NULL;
		pwcsData = (wchar_t*)_alloca(size);

		unsigned char* pszData = NULL;
		pszData = (unsigned char*)_alloca(size);

		if ( pwcsData && pszData )
		{
			memset(pwcsData, 0, size);
			memset(pszData, 0, size);

			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, pwcsData, size);
//			ImmSetCompositionStringW(hIMC, SCS_SETSTR, L"", NULL, L"", NULL);

			if (*pwcsData)
			{
				UINT cp_id = GetCodePage();
				if (cp_id == 950){
					m_ucs2conv.Ucs22Big5(pwcsData, (char*)pszData);
				}
				else{
					::WideCharToMultiByte(cp_id, 0, pwcsData, -1,
						(char*)pszData, size, NULL, NULL);
				}

				for ( ; *pszData; ++pszData )
				{
					UINT in = 0;
					unsigned char ch1 = BYTE(*pszData);
					if (ch1 > 0x7e){
						in = ch1 <<8 | (*(pszData+1));
						++pszData;
					}
					else{
						in = ch1;
					}
					this->OnImeChar(in, 0);
				}
			}
		}
	ImmReleaseContext(m_hWnd, hIMC);
	return 0;
}

LRESULT CTelnetView::_OnImeCompositionA(WPARAM wparam, LPARAM lparam)
{
		HIMC hIMC = ImmGetContext(m_hWnd);
		if(!hIMC)
			return 0;

		DWORD size =
			ImmGetCompositionStringA(hIMC, GCS_RESULTSTR, NULL, 0); 

		if ( size < 1 || size > 1024 )
			return 0;

		size += sizeof(char);

		unsigned char* pszData = NULL;
		pszData = (unsigned char*)_alloca(size);

		if ( pszData )
		{
			memset(pszData, 0, size);

			ImmGetCompositionStringA(hIMC, GCS_RESULTSTR, pszData, size);
//			ImmSetCompositionStringA(hIMC, SCS_SETSTR, "", NULL, "", NULL);			

			if (*pszData)
			{
				for ( ; *pszData; ++pszData )
				{
					UINT in = 0;
					unsigned char ch1 = BYTE(*pszData);
					if (ch1 > 0x7e){
						in = ch1 <<8 | (*(pszData+1));
						++pszData;
					}
					else{
						in = ch1;
					}
					this->OnImeChar(in, 0);
				}
			}
		}
	ImmReleaseContext(m_hWnd, hIMC);
	return 0;
}

void CTelnetView::OnImeChar(WPARAM wp, LPARAM lp)
{
	char db[3] = {0};
	db[0] = (BYTE)(wp >> 8);
	db[1] = (BYTE)wp;
	if( m_pTermData ) {
		if( db[0] ) {
			((CTelnetCon*)m_pTermData)->Send( db, 2 );
		}
		else{
			((CTelnetCon*)m_pTermData)->Send( &db[1], 1 );
		}
	}
}

void CTelnetView::CopyLinkLocation(POINTS& pt)
{
	if( !m_pTermData )
		return;

	POINT _pt;
	_pt.x = pt.x;
	_pt.y = pt.y;
	ScreenToClient(m_hWnd, &_pt);

	int x = _pt.x;
	int y = _pt.y;
	this->PointToLineCol( &x, &y );

	char* pline = m_pTermData->m_Screen[y];
	CTermCharAttr* pattr = m_pTermData->GetLineAttr(pline);
	if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
	{
		int start, end;
		for( start = x-1; start > 0 && pattr[start].IsHyperLink(); start-- );
		if( !pattr[start].IsHyperLink() )
			start++;
		for( end = x+1; end < m_pTermData->m_ColsPerPage && pattr[end].IsHyperLink(); end++ );
		string URL( (pline+start), (int)(end-start) );
		if( IsURLSupported(URL) )
		{

			if( OpenClipboard( m_hWnd ) )
			{
				int TEXT_LEN = URL.length();
		
				EmptyClipboard();
				if ( GetVersion() < 0x80000000 )
				{
					wchar_t* pwtmp = new wchar_t[ TEXT_LEN+1 ];
					memset(pwtmp, 0, (TEXT_LEN+1)*sizeof(wchar_t));

					if (this->GetCodePage() == 950)
						m_ucs2conv.Big52Ucs2(URL.c_str(), pwtmp, TEXT_LEN);
					else
						::MultiByteToWideChar(this->GetCodePage(), 0, URL.c_str(), TEXT_LEN, pwtmp, TEXT_LEN);

					HANDLE hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, (TEXT_LEN+4)*sizeof(wchar_t));
					wchar_t* wbuf=(wchar_t*)GlobalLock(hmem);
					memcpy( wbuf, pwtmp, (TEXT_LEN+4)*sizeof(wchar_t));
					delete [] pwtmp;
					GlobalUnlock(hmem);
					SetClipboardData(CF_UNICODETEXT,hmem);
				}
				else{
					HGLOBAL hmem = GlobalAlloc( GMEM_MOVEABLE, TEXT_LEN+1);
					char* buf = (char*)GlobalLock(hmem);
					memcpy( buf, URL.c_str(), TEXT_LEN+1);
					GlobalUnlock(hmem);
					SetClipboardData( CF_TEXT, hmem );
				}
				CloseClipboard();
			}			
		}//end of IsURLSupported()
	}		
}

void CTelnetView::CancelSelectedBlock()
{
	if (m_pTermData){
		m_pTermData->m_SelStart.x = m_pTermData->m_SelStart.y = m_pTermData->m_SelEnd.x = m_pTermData->m_SelEnd.y = 0;
		InvalidateRect( m_hWnd, NULL, false );
	}
}


// 2007.11. Added by neopro@ptt.cc
void CTelnetView::OpenWithSearchEngine( _SearchInfo* info ){

	string terms = info->search_terms.c_str();
	str_replace(terms, " ", "+");
	string post;

	if ( !info->param_name.empty() )
		post = info->url + "?" + info->param_name + "=" + terms;
	else{
		post = info->url + "/" + terms;
	}

	ShellExecute( NULL, "open", post.c_str(), NULL, NULL, SW_SHOW );
}

// 2008.06.27 Added by neopro@ptt.cc
void CTelnetView::SetSearchEngineData( string url, string param_name, int num )
{
	if (num == 0){
		m_searchInfo_1.url = url;
		m_searchInfo_1.param_name = param_name;
		m_searchInfo_1.search_terms = "";
	}
	if (num == 1){
		m_searchInfo_2.url = url;
		m_searchInfo_2.param_name = param_name;
		m_searchInfo_2.search_terms = "";
	}
}

// 2008.07.04 Added by neopro@ptt.cc
void CTelnetView::OnTextDragDrop( int nAction )
{
	switch ( nAction )
	{
		case 0:
//			MessageBox( m_hWnd, "-y", "Error",  MB_ICONSTOP|MB_OK );
			//-NS_openWithSearchEngine(-1);
			//-break;
		case 1:
//			MessageBox( m_hWnd, "+y", "Error",  MB_ICONSTOP|MB_OK );
			//-NS_openWithSearchEngine(1);
			//-break;
		case 2:
//			MessageBox( m_hWnd, "+x", "Error",  MB_ICONSTOP|MB_OK );
			//-NS_openWithSearchEngine(2);
			//-break;
		case 3:
//			MessageBox( m_hWnd, "-x", "Error",  MB_ICONSTOP|MB_OK );
			//-this->m_searchInfo_1.search_terms = this->m_pTermData->GetSelectedText();
			//-OpenWithSearchEngine(&m_searchInfo_1);
			//-break;

		//tmp
		this->m_searchInfo_1.url = "http://www.google.com/search";
		this->m_searchInfo_1.param_name = "q";
		this->m_searchInfo_1.search_terms = this->m_pTermData->GetSelectedText();
		this->OpenWithSearchEngine(&m_searchInfo_1);
	}
}

wchar_t* CTelnetView::AnsiToUTF16(const string& ansi, unsigned int codepageId)
{
	wchar_t* pwbuf = new wchar_t[ ansi.size()+1 ];
	memset(pwbuf, 0, (ansi.size()+1)*sizeof(wchar_t));

	int count = ansi.size();


	if ( codepageId != 950 )
	{
		::MultiByteToWideChar( codepageId, 0, ansi.c_str(), count, pwbuf, count+1 );
	}
	else // is big5
	{
		m_ucs2conv.Big52Ucs2( ansi.c_str(), pwbuf, ansi.size() );
	}


	return pwbuf;
}
