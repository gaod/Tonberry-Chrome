// Caret.cpp: implementation of the CCaret class.
//
/////////////////////////////////////////////////////////////////////////////
// Name:        caret.cpp
// Purpose:     Show a cursor on the black terminal screen
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw2004.7.17
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
  #pragma implementation "caret.h"
#endif

#include "caret.h"

#pragma comment(lib, "Imm32.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCaret::CCaret()
{
	m_Parent = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_Width = m_Height = 0;
	m_IsVisible = m_IsShow = false;
}

CCaret::~CCaret()
{

}

//Inverse the color of caret when it sets visible flag.
void CCaret::Blink()
{
	if( m_IsVisible ){
		m_IsShow = !m_IsShow;
		DrawInverse();
	}

}

//Hide this caret
void CCaret::Hide()
{
	if((m_IsShow && m_IsVisible )){
		m_IsShow = m_IsVisible = false;
		DrawInverse();
	}
}

//Unhide the caret
void CCaret::Show(bool bImmediately)
{
	m_IsVisible = true;
	if( bImmediately && !m_IsShow ){
		m_IsShow = true;
		DrawInverse();
	}
}

//Reset the position X and Y, perform hiding and reshowing if necessary
void CCaret::Move( int x, int y )
{
	bool bNeedRestore = m_IsVisible;
	if( m_IsVisible )
		Hide();
	m_Pos.x = x;	m_Pos.y = y;
	if( bNeedRestore )
		Show();

	HIMC imc = ImmGetContext(m_Parent);
	COMPOSITIONFORM cs;
	cs.dwStyle = CFS_DEFAULT;
	cs.ptCurrentPos = m_Pos;
	GetClientRect( m_Parent, &cs.rcArea );
	cs.rcArea.left = m_Pos.x;
	cs.rcArea.top = m_Pos.y;
	ImmSetCompositionWindow( imc, &cs );
}

//Reset the position X and Y, perform hiding and reshowing if necessary
void CCaret::SetSize( int Width, int Height )
{
	bool bNeedRestore = m_IsVisible;
	if( m_IsVisible )
		Hide();
	m_Width = Width;	m_Height = Height;
	if( bNeedRestore )
		Show();
}
