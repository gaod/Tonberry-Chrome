// site.cpp: implementation of the CSite class.
//////////////////////////////////////////////////////////////////////
// Name:        site.cpp
// Purpose:     Site Settings
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.07.15
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
  #pragma implementation "site.h"
#endif

#include "site.h"

//	#include "blowfish/blowfish.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSite::CSite(const char* Name) : m_Name(Name)
{
	// Time duration in seconds during which should we reconnect 
	// automatically when disconnected from server, and 0 means disabled.
	m_AutoReconnect = 20;

	// We send this string, m_AntiIdleStr, to the server every 'm_AntiIdle' 
	// seconds to prevent being kicked by the server.
	m_AntiIdleStr = "^[OB";	// empty string means '\0'
	m_AntiIdle = 180;	// 0 means disabled

	// Terminal settings
	// Rows per page
	m_RowsPerPage = 24;
	// Cols per page
	m_ColsPerPage = 80;

	// When pasting long articles, especially those from webpages, wrap lines 
	// automatically when there are more than 'm_AutoWrapOnPaste' characters per line.
	m_AutoWrapOnPaste = 78;	// 0 means disabled.

	// Terminal type
	m_TermType = "vt100";

	// Convert ESC characters in ANSI color to m_ESCConv
	m_ESCConv = "^U";

	// Send CR or CRLF when Enter is pressed.
	m_CRLF = 0;	// 0 = CR, 1 = LF, 2 = CRLF

	// Connect automatically when startup.
	m_Startup = false;

	// 2008.06.26 Added by neopro@ptt.cc
	// Automatic detection of DBCS character (when keydown)
	// which the value of the Bit is TRUE means the detection will be disabled.
	// ----------------------
	// 0000 enables all
	// 0001 disables mouse		,FIXME not implemented yet
	// 0010 disables arrow		,0x2
	// 0100 disables delete		,0x4
	// 1000 disables backspace	,0x8
	// 1111 disables all
	m_AutoDbcsDetection = 0;

	// 2008.07.04 Added by neopro@ptt.cc
	// Mouse browsing support
	m_UseMouseBrowsing = true;

	// 2008.07.04 Added by neopro@ptt.cc
	// search selected text by using drag and drop gestures.
	m_UseTextDragDrop = false;

	// 2008. Added by neopro@ptt.cc
	m_LoadUrlInBackGround = false;

	// 2009.01.23 Added by neopro@ptt.cc
	m_GesturesSensitivityX = 0;
	m_GesturesUseXAxisOnly = false;

	m_UseHelperWnd = true;

	m_MouseOverEffect = 1; //  0 means close

	m_GdippDraw = 2;

	m_FontSize = 0;

	m_FontFace = "MINGLIU";	m_FontFaceEn = "MINGLIU";

	m_UseIniFiles = 0;
}

CSite::~CSite()
{
}


void CSite::SaveToFile(FILE *fo)
{
	fprintf( fo, "[%s]\n", m_Name.c_str() );
	fprintf( fo, "URL=%s\n", m_URL.c_str() );
	fprintf( fo, "AutoReconnect=%d\n", m_AutoReconnect );
	fprintf( fo, "AntiIdle=%d\n", m_AntiIdle );
	fprintf( fo, "AntiIdleStr=%s\n", m_AntiIdleStr.c_str() );
	fprintf( fo, "Rows=%d\n", m_RowsPerPage );
	fprintf( fo, "Cols=%d\n", m_ColsPerPage );
	fprintf( fo, "TermType=%s\n", m_TermType.c_str() );
	fprintf( fo, "ESCConv=%s\n", m_ESCConv.c_str() );
	fprintf( fo, "CRLF=%d\n", m_CRLF );
	/*fprintf( fo, "Startup=%d\n", m_Startup );
	fprintf( fo, "PreLoginPrompt=%s\n", m_PreLoginPrompt.c_str() );
	fprintf( fo, "PreLogin=%s\n", m_PreLogin.c_str() );
	fprintf( fo, "PostLogin=%s\n", m_PostLogin.c_str() );
	fprintf( fo, "LoginPrompt=%s\n", m_LoginPrompt.c_str() );
	fprintf( fo, "Login=%s\n", m_Login.c_str() );
	fprintf( fo, "PasswdPrompt=%s\n", m_PasswdPrompt.c_str() );*/

	fprintf( fo, "AutoDbcsDetection=%d\n", m_AutoDbcsDetection );
	fprintf( fo, "UseMouseBrowsing=%d\n", m_UseMouseBrowsing );
	fprintf( fo, "UseTextDragDrop=%d\n", m_UseTextDragDrop );
	fprintf( fo, "GesturesSensitivityX=%d\n", m_GesturesSensitivityX );
	fprintf( fo, "GesturesUseXAxisOnly=%d\n", m_GesturesUseXAxisOnly );
//	fprintf( fo, "UseHelperWnd=%s\n", m_UseHelperWnd );
	fprintf( fo, "MouseOverEffect=%d\n", m_MouseOverEffect );
	fprintf( fo, "GdippDraw=%d\n", m_GdippDraw );
	fprintf( fo, "FontSize=%d\n", m_FontSize );
	fprintf( fo, "FontFace=%s\n", m_FontFace.c_str() );
	fprintf( fo, "FontFaceEn=%s\n", m_FontFaceEn.c_str() );
	fprintf( fo, "UseIniFiles=%d\n", m_UseIniFiles );

/*	if( m_Passwd.length() && AppConfig.IsLoggedIn() )
	{
		BLOWFISH_CTX *bfc = AppConfig.GetBlowfish();
		if( bfc )
		{
			unsigned long l, r;
			memcpy(&l, m_Passwd.c_str(), 4 );
			memcpy(&r, m_Passwd.c_str() + 4, 4 );
			Blowfish_Encrypt( bfc, &l, &r );
			fprintf( fo, "Passwd=%X,%X\n", l, r );
			AppConfig.ReleaseBlowfish();
		}
	}
	else
		fprintf( fo, "Passwd=%s\n", m_Passwd.c_str() );
*/
}
