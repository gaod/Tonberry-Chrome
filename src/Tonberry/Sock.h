// Sock.h: interface for the CSock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCK_H__8E29C759_DA57_4D9A_9FF7_5502E67F421E__INCLUDED_)
#define AFX_SOCK_H__8E29C759_DA57_4D9A_9FF7_5502E67F421E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <winsock.h>

#define SD_BOTH         0x02

using namespace std;

class CSock  
{
friend class CSock;
public:
	void OnGetHost(LPARAM lp)
	{
		if( 0 == WSAGETASYNCERROR(lp) && m_GetHostBuf )
		{
			hostent* host = (hostent*)m_GetHostBuf;
			DoConnect( ((in_addr*)host->h_addr_list[0])->s_addr );
			delete []m_GetHostBuf;
			m_GetHostBuf = NULL;
			m_GetHostTask = INVALID_HANDLE_VALUE;
		}
	}
	int DoConnect(unsigned long addr);
	SOCKET GetSock() { return m_Sock; };
	bool Accept(CSock* pListenSock, sockaddr* addr, int *addrlen){ m_Sock = accept(pListenSock->m_Sock, addr, addrlen); return IsValid(); }
	bool Create() { m_Sock = socket(AF_INET, 0, 0);  return IsValid(); }
	int Listen(int backlog) { return listen(m_Sock, backlog); }
	int Recv( const void* buf, int len)  { return recv(m_Sock, (char*)buf, len, 0); }
	int Send( const void* buf, int len)  { return send(m_Sock, (char*)buf, len, 0); }
	int SendStr(const char* str) {	return Send(str,strlen(str));	}
	void SetAddress( string ads ){ m_Address = ads; }
	void SetPort( unsigned int  port ){ m_Port = port; }
	string GetAddress(){ return m_Address; }
	unsigned int GetPort() { return m_Port; }

	static void Cleanup();
	static bool Init();
	static void SetSinkWnd(HWND wnd) { m_SinkWnd = wnd; }

	int Select(HWND sinkwnd, UINT msg, long evt);
	int Connect(const char *address, unsigned short port, UINT host_found_msg);
	virtual void Close();
	int Bind(const char *address, short port);
	bool IsValid(){ return (m_Sock != INVALID_SOCKET); }
	CSock();
	virtual ~CSock();

protected:
	string m_Address;
	unsigned int m_Port;
	static HWND m_SinkWnd;
	SOCKET m_Sock;
	char* m_GetHostBuf;
	HANDLE m_GetHostTask;
};

#endif // !defined(AFX_SOCK_H__8E29C759_DA57_4D9A_9FF7_5502E67F421E__INCLUDED_)
