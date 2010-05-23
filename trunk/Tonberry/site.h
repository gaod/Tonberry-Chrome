// site.h: interface for the CSite class.
/////////////////////////////////////////////////////////////////////////////
// Name:        site.h
// Purpose:     Site Settings
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.07.15
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_SITE_H__F305F1AE_B980_4FF3_A11B_F18CFAEC5F2B__INCLUDED_)
#define AFX_SITE_H__F305F1AE_B980_4FF3_A11B_F18CFAEC5F2B__INCLUDED_

#ifdef __GNUG__
  #pragma interface "site.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

class CSite
{
public:
	string m_FontFace, m_FontFaceEn;
	int m_FontSize;
	bool m_Startup;
	void SaveToFile(FILE* fo);
	// Name of site
	string m_Name;

	// IP : port
	string m_URL;

	// Time duration in seconds during which should we reconnect
	// automatically when disconnected from server, and 0 means disabled.
	unsigned int m_AutoReconnect;

	// We send this string, m_AntiIdleStr, to the server every 'm_AntiIdle'
	// seconds to prevent being kicked by the server.
	string m_AntiIdleStr;	// empty string means '\0'
	unsigned int m_AntiIdle;	// 0 means disabled

	// Terminal settings
	// Rows per page
	unsigned int m_RowsPerPage;
	// Cols per page
	unsigned int m_ColsPerPage;

	// When pasting long articles, especially those from webpages, wrap lines 
	// automatically when there are more than 'm_AutoWrapOnPaste' characters per line.
	unsigned int m_AutoWrapOnPaste;	// 0 means disabled.

	// Convert ESC characters in ANSI color to m_ESCConv
	string m_ESCConv;

	// Terminal type
	string m_TermType;

	int m_CRLF;
	// Send CR, LF, or CRLF when Enter is pressed
	inline const char* GetCRLF()
	{
		const char* crlf[3] = { "\r", "\n", "\r\n" };
		return (m_CRLF > 3 ? "\r" : crlf[m_CRLF]);
	}

	// Automatic detection of DBCS character (when keydown)
	unsigned int m_AutoDbcsDetection;


	// Mouse browsing support
	bool m_UseMouseBrowsing;
	// search selected text by using drag and drop gestures.
	bool m_UseTextDragDrop;

	bool m_LoadUrlInBackGround;

	int m_GesturesSensitivityX;
	bool m_GesturesUseXAxisOnly;

	bool m_UseHelperWnd;

	int m_MouseOverEffect;

	int m_GdippDraw;

	bool m_UseIniFiles;


	CSite(const char* Name = "");
	~CSite();

    string& GetPasswd(){	return m_Passwd;	}
    void SetPasswd( string passwd ){	m_Passwd = passwd;	}
    
    string& GetPasswdPrompt(){	return m_PasswdPrompt;	}
    void SetPasswdPrompt( string passwd_prompt ){	m_PasswdPrompt = passwd_prompt;	}
    
	string& GetLogin(){	return m_Login;	}
    void SetLogin( string login ){	m_Login = login;	}

	string& GetLoginPrompt(){	return m_LoginPrompt;	}
    void SetLoginPrompt( string login_prompt ){	m_LoginPrompt = login_prompt;	}

    string& GetPreLogin(){	return m_PreLogin;	}
    void SetPreLogin(string prelogin){	m_PreLogin = prelogin;	}
    
	string& GetPreLoginPrompt(){	return m_PreLoginPrompt;	}
    void SetPreLoginPrompt( string prelogin_prompt ){	m_PreLoginPrompt = prelogin_prompt;	}

	string& GetPostLogin(){	return m_PostLogin;	}
    void SetPostLogin(string postlogin){	m_PostLogin = postlogin;	}

protected:
    string m_Passwd;
    string m_Login;
    string m_LoginPrompt;
    string m_PasswdPrompt;
    string m_PreLogin;
    string m_PreLoginPrompt;
	string m_PostLogin;
};

#endif // !defined(AFX_SITE_H__F305F1AE_B980_4FF3_A11B_F18CFAEC5F2B__INCLUDED_)
