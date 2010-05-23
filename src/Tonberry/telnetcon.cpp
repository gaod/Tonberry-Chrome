/////////////////////////////////////////////////////////////////////////////
// Name:        telnetcon.cpp
// Purpose:     Class dealing with telnet connections, parsing telnet commands.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.16
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
  #pragma implementation "telnetcon.h"
#endif

#include "telnetcon.h"
#include "telnetview.h"
#include "stringutil.h"
#include "usermessage.h"

// #include "appconfig.h"

#include <cctype>

// class constructor
CTelnetCon::CTelnetCon(CTermView* pView, CSite& SiteInfo)
	: CTermData(pView), m_Site(SiteInfo)
{
    m_pBuf = m_pLastByte = m_pRecvBuf = NULL;
    m_pCmdLine = m_CmdLine;
	m_pCmdLine[0] = '\0';

	m_State = TS_CONNECTING;
	m_Duration = 0;
	m_IdleTime = 0;
	m_AutoLoginStage = 0;

	Init();
}

// class destructor
CTelnetCon::~CTelnetCon()
{
	Close();

	Cleanup();
}

// No description
bool CTelnetCon::Connect()
{
	m_Duration = 0;
	m_IdleTime = 0;
	m_State = TS_CONNECTING;

	// If this site has auto-login settings, activate auto-login
	// and set it to stage 1, waiting for prelogin prompt or stage 2,
	// waiting for login prompt.
//	if( !m_Site.GetLogin().empty() && AppConfig.IsLoggedIn() )
//		m_AutoLoginStage = m_Site.GetPreLogin().empty() ? 2 : 1 ;
//	else
		m_AutoLoginStage = 0;

	string address;
	unsigned short port = 23;
	int p = m_Site.m_URL.find(':');
	if( p != m_Site.m_URL.npos )		// use port other then 23;
	{
		port = (unsigned short)atoi(m_Site.m_URL.c_str()+p+1);
		address = m_Site.m_URL.substr(0, p);
	}
	else
		address = m_Site.m_URL;

	Create();
	SetAddress(address);
	SetPort(port);

	Select( m_pView->m_hWnd, CTelnetView::WM_SOCKET, FD_CONNECT|FD_READ|FD_CLOSE );
	CSock::Connect(address.c_str(), port, CTelnetView::WM_GETHOSTBYNAME);
	::PostMessage( m_pView->m_parent, WM_USER_CONNECTION_OPEN, (WPARAM)m_Site.m_URL.c_str(), NULL );
	
    return true;
}

// No description
void CTelnetCon::OnRecv(int code)
{
	unsigned char buffer[4097];
	m_pRecvBuf = buffer;
    int rlen = Recv(m_pRecvBuf, sizeof(buffer)-1);

    m_pRecvBuf[rlen] = '\0';
    m_pBuf = m_pRecvBuf;
    m_pLastByte = m_pRecvBuf + rlen;

    ParseReceivedData();

	if( m_AutoLoginStage > 0 )
		CheckAutoLogin();

	// for mouse browsing support, Added on July 1, 2008
	SetPageState();
	//
//	CheckBoardName();


	UpdateDisplay();
}

void CTelnetCon::OnConnect(int code)
{
	m_State = TS_CONNECTED;
}

void CTelnetCon::OnClose(int code)
{
	m_State = TS_CLOSED;

	::PostMessage( m_pView->m_parent, WM_USER_CONNECTION_CLOSE, NULL, NULL );
	//	if disconnected by the server too soon, reconnect automatically.
	if( m_Duration < m_Site.m_AutoReconnect )
		Connect();
}

/*
 * Parse received data, process telnet command 
 * and ANSI escape sequence.
 */
void CTelnetCon::ParseReceivedData()
{
    for( m_pBuf = m_pRecvBuf; m_pBuf < m_pLastByte; m_pBuf++ )
    {
		if( m_CmdLine[0] == TC_IAC )	// IAC, in telnet command mode.
		{
			ParseTelnetCommand();
			continue;
		}

        if( *m_pBuf == TC_IAC )    // IAC, in telnet command mode.
        {
            m_CmdLine[0] = TC_IAC;
            m_pCmdLine = &m_CmdLine[1];
            continue;
		}
		// *m_pBuf is not a telnet command, let genic terminal process it.
		CTermData::PutChar( *m_pBuf );
    }
}

// Process telnet command.
void CTelnetCon::ParseTelnetCommand()
{
    *m_pCmdLine = *m_pBuf;
    m_pCmdLine++;
	switch( m_CmdLine[1] )
	{
	case TC_WILL:
		{
			if( 3 > (m_pCmdLine-m_CmdLine) )
				return;
			char ret[]={TC_IAC,TC_DONT,*m_pBuf};
			switch(*m_pBuf)
			{
			case TO_ECHO:
			case TO_SUPRESS_GO_AHEAD:
				ret[1] = TC_DO;
				break;
			}
			Send(ret, 3);
			break;
		}
	case TC_DO:
		{
			if( 3 > (m_pCmdLine-m_CmdLine) )
				return;
			char ret[]={TC_IAC,TC_WILL,*m_pBuf};
			switch(*m_pBuf)
			{
			case TO_TERMINAL_TYPE:
			case TO_NAWS:
				break;
			default:
				ret[1] = TC_WONT;
			}
			Send(ret,3);
			if( TO_NAWS == *m_pBuf )	// Send NAWS
			{
				unsigned char naws[]={TC_IAC,TC_SB,TO_NAWS,0,80,0,24,TC_IAC,TC_SE};
				naws[3] = m_ColsPerPage >>8;	// higher byte
				naws[4] = m_ColsPerPage & 0xff; // lower byte
				naws[5] = m_RowsPerPage >> 8;	// higher byte
				naws[6] = m_RowsPerPage & 0xff; // lower byte
				Send( naws, sizeof(naws));
			}
			break;
		}
	case TC_WONT:
	case TC_DONT:
		if( 3 > (m_pCmdLine-m_CmdLine) )
			return;
		break;
	case TC_SB:	// sub negotiation
		if( *m_pBuf == TC_SE )	// end of sub negotiation
		{
			switch( m_CmdLine[2] )
			{
			case TO_TERMINAL_TYPE:
				{
					// Return terminal type.  2004.08.05 modified by PCMan.
					unsigned char ret_head[] = { TC_IAC, TC_SB, TO_TERMINAL_TYPE, TO_IS };
					unsigned char ret_tail[] = { TC_IAC, TC_SE };
					int ret_len = 4 + 2 + m_Site.m_TermType.length();
					unsigned char *ret = new unsigned char[ret_len];
					memcpy( ret, ret_head, 4);
					memcpy( ret + 4, m_Site.m_TermType.c_str(), m_Site.m_TermType.length() );
					memcpy( ret + 4 + m_Site.m_TermType.length() , ret_tail, 2);
					Send(ret, ret_len);
					delete []ret;
				}
			}
		}
		else
			return;	// prevent m_CmdLine from being cleard.
	}
	m_CmdLine[0] = '\0';
	m_pCmdLine = m_CmdLine;
}

void CTelnetCon::Disconnect()
{
	if( m_State != TS_CONNECTED )
		return;
	m_State = TS_CLOSED;
	Close();
}

void CTelnetCon::OnTimer()
{
	if( m_State == TS_CLOSED )
		return;
	m_Duration++;
	m_IdleTime++;
//	Note by PCMan:
//	Here is a little trick.
//	Since we have increased m_IdleTime by 1, it's impossible for 
//	m_IdleTime to equal zero.
//	When 'Anti Idle' is disabled, m_Site.m_AntiIdle must = 0.
//	So m_Site.m_AntiIdle != m_IdleTimeand, and the following Send() won't be called.
//	Hence we don't need to check if 'Anti Idle' is enabled or not.


	if( m_Site.m_AntiIdle == m_IdleTime )
	{
		//	2004.8.5 Added by PCMan.	Convert non-printable control characters.
		string aistr = UnEscapeStr( m_Site.m_AntiIdleStr.c_str() );
		Send( aistr.c_str(), aistr.length() );
	}
	//	When Send() is called, m_IdleTime is set to 0 automatically.

}


//	Virtual function called from parent class to let us determine
//	whether to beep or show a visual indication instead.
void CTelnetCon::Bell()
{
}


/*!
    \fn CTelnetCon::CheckAutoLogin
 */
void CTelnetCon::CheckAutoLogin()
{
/*	int last_line = m_FirstLine + m_RowsPerPage;
	const char* prompts[] = {
		NULL,	//	Just used to increase array indices by one.
		m_Site.GetPreLoginPrompt().c_str(),	//	m_AutoLoginStage = 1
		m_Site.GetLoginPrompt().c_str(),	//	m_AutoLoginStage = 2
		m_Site.GetPasswdPrompt().c_str() };	//	m_AutoLoginStage = 3

	bool prompt_found = false;
	for( int line = m_FirstLine; line < last_line; line++ )
	{
		if( strstr(m_Screen[line], prompts[m_AutoLoginStage] ) )
		{
			prompt_found = true;
			break;
		}
	}

	if( prompt_found )
	{
		const char* responds[] = {
			NULL,	//	Just used to increase array indices by one.
			m_Site.GetPreLogin().c_str(),	//	m_AutoLoginStage = 1
			m_Site.GetLogin().c_str(),	//	m_AutoLoginStage = 2
			m_Site.GetPasswd().c_str(),	//	m_AutoLoginStage = 3
			""	//	m_AutoLoginStage = 4, turn off auto-login
			};

		string respond = responds[m_AutoLoginStage];
		respond += m_Site.GetCRLF();
		SendString(respond);

		if( !responds[ ++m_AutoLoginStage ][0] )	// Go to next stage.
		{
			m_AutoLoginStage = 0;	// turn off auto-login after all stages end.
			respond = m_Site.GetPostLogin();	// Send post-login string
			if( respond.length() > 0 )
			{
				// Unescape all control characters.
				UnEscapeStr(respond);
				SendString(respond);
			}
		}
	}
*/
}

void CTelnetCon::SendString(string str)
{
//	str.replace( 0, str.length(), "\n", m_Site.GetCRLF());
	Send(str.c_str(), str.length());
}

// for mouse browsing support, Added on July 1, 2008
char CTelnetCon::GetMenuChar(int y)
{ 
        char* str = m_Screen[y];
        for (int i = 0; ; i++)
        {
                if (str[ i ] !=' ')
                {
                        if ( isalpha(str[i]) )
                                return str[ i ];
                        return str[i + 1];
                }
        }
}

void CTelnetCon::SetPageState()
{
        m_nPageState = -1; //NORMAL

        char* pLine = m_Screen[m_FirstLine];

        if( IsUnicolor(pLine, 0, m_ColsPerPage / 2) )
        {
                pLine = m_Screen[m_FirstLine + 2];
                if(IsUnicolor(pLine,0,m_ColsPerPage / 2))
                        m_nPageState = 1; // LIST
                else
                        m_nPageState = 0; // MENU
        }
        else
        {
                pLine = m_Screen[m_FirstLine + m_RowsPerPage - 1];
                if( IsUnicolor(pLine, m_ColsPerPage / 3, m_ColsPerPage * 2 / 3) )
                        m_nPageState = 2; // READING
        }
}

bool CTelnetCon::IsUnicolor(char* pLine, int start, int end)
{
        CTermCharAttr* pAttr = GetLineAttr(pLine);
        COLORREF clr = pAttr[start].GetBgColor( CTermCharAttr::GetDefaultColorTable() );

        // a dirty hacking, because of the difference between maple and firebird bbs.
        for ( int i = start; i < end; i++)
        {
                COLORREF clr1 = pAttr[i].GetBgColor( CTermCharAttr::GetDefaultColorTable() );
                if (clr1 != clr || clr1 == CTermCharAttr::GetDefaultColorTable(0))
                {
                        return false;
                }       
        }

        return true;
}     
//

/*void CTelnetCon::CheckBoardName()
{
	if (m_nPageState != 1)//list
		return;

	static char* old_ptr;

	char* pLine = m_Screen[m_FirstLine];
	char* ptr = strstr( pLine, "¬ÝªO¡m" );

	if (ptr)
	{
		if (ptr != old_ptr){
			::PostMessage( m_pView->m_parent, WM_USER_BOARD_NAME_CHANGED, (WPARAM)ptr+4, NULL );
			old_ptr = ptr;
		}		
	}
}*/