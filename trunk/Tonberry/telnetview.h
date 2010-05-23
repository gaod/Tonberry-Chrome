// TelnetView.h: interface for the CTelnetView class.
//
/////////////////////////////////////////////////////////////////////////////
// Name:        telnetcon.h
// Purpose:     Class dealing with telnet terminal screen, the central black view.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.16
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////


#ifndef _TELNETVIEW_H_
#define _TELNETVIEW_H_

#ifdef __GNUG__
  #pragma interface "telnetview.h"
#endif

#include <string>
#include "termview.h"
#include "MouseGestures.h"



using namespace std;


struct _SearchInfo
{
	std::string		url;
	std::string		param_name;
	std::string		search_terms;
};

class CHelperWnd;
class CSite;
class CTelnetCon;
class CTelnetView : public CTermView
{
public:
	void SendString(string str);
	void OnContextMenu(POINTS pt);
	enum {WM_SOCKET = WM_APP + 100, WM_GETHOSTBYNAME = WM_APP+102};

	CTelnetCon* GetCon(){return (CTelnetCon*)m_pTermData;}
	void NewCon(CSite& si);
	LRESULT WndProc(UINT msg, WPARAM wp, LPARAM lp);
	CTelnetView(HWND hWnd);
	virtual ~CTelnetView();

	CTelnetCon* GetCurCon(){	return (CTelnetCon*)m_pTermData;	}



	void OnTextDragDrop( int nAction );
	void OnPaste();
	void CopyLinkLocation(POINTS& pt);
	void CancelSelectedBlock();
	void OpenShorterUrl(string url);

	void NotifyOpenWithMozilla( string url, bool background );
	void NS_openWithSearchEngine(int action);

	wchar_t* AnsiToUTF16(const string& ansi, unsigned int codepageId);

	void SetSearchEngineData( std::string url, std::string param_name, int num );
	void OpenWithSearchEngine( _SearchInfo* info );
	_SearchInfo m_searchInfo_1, m_searchInfo_2;

	DWORD m_imeProp;
	bool m_bTrackLeave;
	CMouseGestures m_MouseGestures;
	CHelperWnd* m_helperWnd;

protected:
	void OnChar(int ch, long data);
	bool OnKeyDown(int key, long data);
	bool _onkeydown(int key, long data);
	bool _onkeydown_chrome(int key, long data);
    void OnSendStringAsync(string str);

	void OnImeChar(WPARAM wp, LPARAM lp);
	bool OnSysKeyDown(int key, long data);
	LPARAM OnMouseLeave(WPARAM wp, LPARAM lp);

	LRESULT OnImeComposition(WPARAM wparam, LPARAM lparam);
	LRESULT _OnImeCompositionW(WPARAM wparam, LPARAM lparam);
	LRESULT _OnImeCompositionA(WPARAM wparam, LPARAM lparam);
	// virtual function
	void OnMouseMove(UINT flags, POINTS& pt);
	void OnMouseWheel(WPARAM wp, LPARAM lp);
	void OnLButtonDown(UINT flags, POINTS& pt);
	void OnLButtonUp(UINT flags, POINTS& pt);
	void OnRButtonDown(UINT flags, POINTS& pt);
	void OnRButtonUp(UINT flags, POINTS& pt);
	void OnMButtonUp(UINT flags, POINTS& pt);
	void OnCommand(WPARAM wp, LPARAM lp);


	// property	
	bool m_textSelected;
	bool m_canPopup;

	bool m_initSetGesturesX;
	bool m_initUseHelperWnd;

	bool m_keyFuncSwitch;
	bool m_RbtnInUse;
	bool m_MouseWheelInUse;

private:

};


#endif
