// Sock.cpp: implementation of the CSock class.
//
//////////////////////////////////////////////////////////////////////

#include "Sock.h"
/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HWND CSock::m_SinkWnd = NULL;
static char SinkWndClassName[]  = "FtpSink";

CSock::CSock()
{
	m_Sock = INVALID_SOCKET;
	m_GetHostBuf = NULL;
	m_SinkWnd = NULL;
	m_Port = 0;
	m_GetHostTask = INVALID_HANDLE_VALUE;
}

CSock::~CSock()
{
	if( IsValid() )
		Close();
}

int CSock::Bind(const char *address, short port)
{
	sockaddr_in sock;
	memset(&sock,0,sizeof(sock));
	sock.sin_family=AF_INET;
	sock.sin_addr.S_un.S_addr=address ? inet_addr(address):INADDR_ANY;
	sock.sin_port=htons(port);
	return bind(m_Sock,(sockaddr*)&sock,sizeof(sock));
}

void CSock::Close()
{
	if( IsValid() )
	{
		shutdown(m_Sock, SD_BOTH);
		closesocket(m_Sock);
//		PostMessage(m_MsgWnd,WM_SOCKET_REMOVE,WPARAM(m_Sock),LPARAM(this));
		m_Sock = INVALID_SOCKET;

		if( m_GetHostBuf )
			delete []m_GetHostBuf;

		if( m_GetHostTask != INVALID_HANDLE_VALUE)
		{
			WSACancelAsyncRequest(m_GetHostTask);
			m_GetHostTask = INVALID_HANDLE_VALUE;
		}
	}
}


int CSock::Connect(const char *address, unsigned short port, UINT host_found_msg)
{
	unsigned long addr=inet_addr(address);
	if( INADDR_NONE == addr && host_found_msg)
	{
		m_GetHostBuf = new char[MAXGETHOSTSTRUCT];
		m_GetHostTask = WSAAsyncGetHostByName( m_SinkWnd, host_found_msg, address, m_GetHostBuf, MAXGETHOSTSTRUCT );
		return 0;
	}
	return DoConnect(addr);
}

int CSock::Select(HWND sinkwnd, UINT msg, long evt)
{
	m_SinkWnd = sinkwnd;
	return WSAAsyncSelect( m_Sock, sinkwnd, msg, evt );
}


bool CSock::Init()
{
	WSADATA wsad;
	memset( &wsad, 0, sizeof(wsad));
	if(WSAStartup( MAKEWORD(1, 1), &wsad ))
		return false;
	return true;
}

void CSock::Cleanup()
{
	WSACleanup();
}


int CSock::DoConnect(unsigned long addr)
{
	sockaddr_in sock;
	memset(&sock,0,sizeof(sock));
	sock.sin_family=AF_INET;
	sock.sin_port=htons(m_Port);
//	sock.sin_addr.S_un.S_addr = inet_addr(m_Address);
	sock.sin_addr.S_un.S_addr = addr;
	return connect(m_Sock,(sockaddr*)&sock,sizeof(sock));
}

