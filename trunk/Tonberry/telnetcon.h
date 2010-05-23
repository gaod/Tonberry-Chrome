/////////////////////////////////////////////////////////////////////////////
// Name:        telnetcon.h
// Purpose:     Class dealing with telnet connections, parsing telnet commands.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.16
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////


#ifndef TELNETCON_H
#define TELNETCON_H

#ifdef __GNUG__
  #pragma interface "telnetcon.h"
#endif

#include "termdata.h"
#include "site.h"
#include "sock.h"
#include <string>




using namespace std;

// Telnet commands
#define TC_SE                   (unsigned char)240
#define TC_NOP                  (unsigned char)241
#define TC_DATA_MARK            (unsigned char)242
#define TC_BREAK                (unsigned char)243
#define TC_INTERRUPT_PROCESS    (unsigned char)244
#define TC_ABORT_OUTPUT         (unsigned char)245
#define TC_ARE_YOU_THERE        (unsigned char)246
#define TC_ERASE_CHARACTER      (unsigned char)247
#define TC_ERASE_LINE           (unsigned char)248
#define TC_GO_AHEAD	            (unsigned char)249
#define TC_SB                   (unsigned char)250
// Option commands                             
#define TC_WILL                 (unsigned char)251
#define TC_WONT                 (unsigned char)252
#define TC_DO                   (unsigned char)253
#define TC_DONT                 (unsigned char)254
#define TC_IAC                  (unsigned char)255
                                 
// Telnet options)               
#define TO_ECHO                 (unsigned char)1
#define TO_SUPRESS_GO_AHEAD     (unsigned char)3
#define TO_TERMINAL_TYPE        (unsigned char)24
#define	TO_IS                   (unsigned char)0
#define	TO_SEND                 (unsigned char)1
#define TO_NAWS                 (unsigned char)31

/*
 * A class for Telnet Connections, used to store screen buffer of 
 * every connections and their sockets, etc.
 */

class CTelnetCon : public CTermData, public CSock
{
public:
	virtual void Bell();	// called from CTermData to process beep.
	void OnTimer();

	// A flag used to indicate connecting state;
	enum{TS_CONNECTING, TS_CONNECTED, TS_CLOSED} m_State;

	void Disconnect();
	// class constructor
	CTelnetCon(CTermView* pView, CSite& SiteInfo);
	// class destructor
	~CTelnetCon();
	// No description
	virtual bool Connect();
	// Connecting duration
	unsigned int m_Duration;
	// Idle time, during which the user doesn't have any key input
	unsigned int m_IdleTime;

	CSite& m_Site;

	void OnClose(int code);
	void OnConnect(int code);
	void OnRecv(int code);
	/**
		* Parse received data, process telnet command
		* and ANSI escape sequence.
		*/
	inline void ParseReceivedData();
	// Parse telnet command.
	inline void ParseTelnetCommand();

	inline void Send(const void* pdata, int len)
	{
		CSock::Send(pdata, len);
		m_IdleTime = 0;	// Since data has been sent, we are not idle.
	}

	void SendString(string str);

	// for mouse browsing support, Added on July 1, 2008
	inline int GetPageState() { return m_nPageState; }
	char GetMenuChar(int y);



protected:
	// Buffer to receive socket incoming data
	unsigned char* m_pRecvBuf;
	unsigned char* m_pBuf;
	unsigned char* m_pLastByte;
    unsigned int m_AutoLoginStage;	// 0 means turn off auto-login.
protected:
    void CheckAutoLogin();
    void SendStringAsync(string str);

	// for mouse browsing support, Added on July 1, 2008
    // Page state for mouse browsing
    /*	enum m_nPageState
    {
      NORMAL=-1,
      MENU=0,
      LIST=1,
      READING=2
      };*/
    int m_nPageState;

private:
    void SetPageState();
    bool IsUnicolor(char* line, int start, int end);
//	void CheckBoardName();

};

#endif // TELNETCON_H

