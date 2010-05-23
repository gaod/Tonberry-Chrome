/////////////////////////////////////////////////////////////////////////////
// Name:        termview.cpp
// Purpose:     Displey terminal screen data sroted in CTermData.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.17
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
  #pragma implementation "termview.h"
#endif

// for WM_MOUSEWHEEL support
#define _WIN32_WINNT  0x0500
#include <windows.h>
// ---

//#define WINVER 0x0501

#include "termview.h" // class's header file
#include "termdata.h"
#include "telnetcon.h" //
#include "resource.h"

#include "stringutil.h"
#include <commctrl.h>
#include ".\termview.h"
#include <string>
#include "gdi++.h"
#include <math.h> // fabs()

using namespace std;

static const char term_view_class[] = "TermView";

extern int g_Scale;


LRESULT CALLBACK CTermView::_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	CTermView* pView = (CTermView*)GetWindowLong( hwnd, GWL_USERDATA );
	if( pView )
		return pView->WndProc( msg, wp, lp );
	return DefWindowProc(hwnd, msg, wp, lp);
}

// class constructor
CTermView::CTermView( HWND parent )
	: m_pTermData(NULL), m_Font( NULL ), 
	m_HyperLinkColor(RGB(255,102,0)), 
	m_pColorTable(CTermCharAttr::GetDefaultColorTable())
{
	// Register class
	WNDCLASS wc = {0};
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wc.lpfnWndProc = (WNDPROC)CTermView::_WndProc;
	wc.lpszClassName = term_view_class;
	wc.style = CS_HREDRAW|CS_VREDRAW;

	RegisterClass( &wc );
	m_parent = parent;

	// Create terminal view window
	m_hWnd = CreateWindowEx( 0, term_view_class, NULL, WS_CHILD|WS_VISIBLE, 0, 0, 0, 0,
                             parent, NULL, wc.hInstance, NULL );
	SetWindowLongPtr( m_hWnd, GWL_USERDATA, (LONG_PTR)this );

	m_pTermData = NULL;
	m_pHyperLink = NULL;
	m_Caret.Create(m_hWnd);

	SetTimer( m_hWnd, ID_BLINK_TIMER, 600, NULL );
	SetTimer( m_hWnd, ID_ANTIIDLE_TIMER, 1000, NULL );
	m_CharPaddingX = 0;
	m_CharPaddingY = 0;

	m_CharW = 5;
	m_CharH = 10;
	m_Caret.SetSize(m_CharW, 2);

	updateFont();
	updateFontEn();

	m_ShowBlink = true;

	m_HandCursor = LoadCursor( NULL, IDC_HAND );
	m_ArrowCursor = LoadCursor( NULL, IDC_ARROW );
	//
	m_PageDownCursor = m_HandCursor;
	m_PageUpCursor = m_HandCursor;
	m_BullsEyeCursor = m_HandCursor;
	m_HomeCursor = m_HandCursor;
	m_EndCursor = m_HandCursor;


	/*m_ExitCursor = (HCURSOR)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_EXIT), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
	m_DragtextCursor = (HCURSOR)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DRAGTEXT), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);*/
	m_ExitCursor = m_HandCursor;	m_DragtextCursor = m_HandCursor;
	//
	m_oldDragCursor = m_ArrowCursor;

	m_IsSelecting = false;
	m_beginDrag = false;

	m_ucs2conv.InitBig52Ucs2Tab();
	m_ucs2conv.InitUcs22Big5Tab();

	unsigned int cpid = ::GetACP();
	if (cpid == 936)
		SetCodePage(cpid);
	else
		SetCodePage(950);

}

// class destructor
CTermView::~CTermView()
{
	SetWindowLongPtr( m_hWnd, GWL_USERDATA, NULL );

	if(m_pTermData)
		delete m_pTermData;
//	KillTimer(m_hWnd, ID_BLINK_TIMER);
}

void CTermView::OnPaint( HDC dc, RECT& update_rect )
{
	RECT rc;
	GetClientRect( m_hWnd, &rc );
	int w = rc.right;
	int h = rc.bottom;
	PrepareDC(dc);
	HGDIOBJ oldfnt = SelectObject( dc, m_Font );

	// Hide the caret to prevent drawing problems.
	m_Caret.Hide();
	if( m_pTermData )
	{
//		wxRegionIterator Rit(GetUpdateRegion());
//		for( ; Rit; Rit++ )

		{
/*			int top = Rit.GetY();		int bottom = top + Rit.GetH();
			int left = Rit.GetX();		int right = left + Rit.GetW();
			this->PointToLineCol( &left, &top );
			this->PointToLineCol( &right, &bottom );

			if(right < m_pTermData->m_ColsPerPage)	right++;
			if(left > 0)	left-=left>1?2:1;
			if(bottom < m_pTermData->m_RowsPerPage-1)	bottom++;
			if(top > 0)	top-=top>1?2:1;
			int iline = m_pTermData->m_FirstLine + top;
			int ilast = m_pTermData->m_FirstLine + bottom;
			top *= m_CharH;
*/
			int top = 0;
			int iline = m_pTermData->m_FirstLine;
			int ilast = m_pTermData->m_FirstLine+m_pTermData->m_RowsPerPage-1;
			for( ; iline <= ilast; iline++ , top += m_CharH )
			{
				for( int col = 0; col < m_pTermData->m_ColsPerPage; )
//				for( int col = left; col < right; )
					col += DrawChar( dc, iline, col, top );
			}
		}

		int left = m_pTermData->m_ColsPerPage*m_CharW-2;
		rc.left = left;
		SetBkColor( dc, CTermCharAttr::GetDefaultColorTable(0) );
		ExtTextOut( dc, left, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		int top = m_pTermData->m_RowsPerPage*m_CharH;
		rc.left = 0;
		rc.top = top;
		ExtTextOut( dc, 0, top, ETO_OPAQUE, &rc, NULL, 0, NULL);

		//	Restore the caret
		m_Caret.Show();
	}
	else
	{
		SetBkColor( dc, CTermCharAttr::GetDefaultColorTable(0) );
		ExtTextOut( dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	SelectObject( dc, oldfnt );
}


/** EVT_LEFT_DOWN handler */
void CTermView::OnLButtonDown(UINT flags, POINTS& pt)
{
	//virtual function.
}


void CTermView::OnBlinkTimer()
{
	m_ShowBlink = !m_ShowBlink;
	if(m_pTermData)
	{
		HDC dc = GetDC( m_hWnd );
//		PrepareDC(dc);

		HGDIOBJ oldfnt = SelectObject( dc, m_Font );

		int top = 0;
		int iline = m_pTermData->m_FirstLine;
		int ilast = m_pTermData->m_FirstLine+m_pTermData->m_RowsPerPage-1;
//		const int left = 0;	const int right = m_pTermData->m_ColsPerPage;
		for( ; iline <= ilast; iline++ , top += m_CharH )
		{
//			DrawLine( dc, i, y, true , left, right );
			CTermCharAttr* attr = m_pTermData->GetLineAttr( m_pTermData->m_Screen[iline] );
			for( int col = 0; col < m_pTermData->m_ColsPerPage; )
			{
				if( attr[col].IsBlink())
					col += DrawChar( dc, iline, col, top );
				else
					col++;
			}
		}

		SelectObject( dc, oldfnt );
		ReleaseDC( m_hWnd, dc );
	}
	m_Caret.Blink();
}


void CTermView::OnSize( int SizeType, int cx, int cy )
{
	if( !m_pTermData )
		return;

	if( ((CTelnetCon*)m_pTermData)->m_Site.m_GdippDraw == 11 )
	{
		updateFont(true);
		updateFontEn(true);
	}else{
		updateFont();
		updateFontEn();
	}
}


void CTermView::PointToLineCol(int *x, int *y, bool adjust_x)
{
	int pt_x = *x;

	*x /= m_CharW;
	if (*x < 0)
		*x = 0;
	else if( *x > m_pTermData->m_ColsPerPage )
		*x = m_pTermData->m_ColsPerPage;

	*y /= m_CharH;
	if(*y <0 )
		*y = 0;
	else if( (*y+1) > m_pTermData->m_RowsPerPage )
		*y = m_pTermData->m_RowsPerPage-1;


	char* curstr = m_pTermData->m_Screen[*y];

	if ( adjust_x )
	{
		//¤ä´©¤¤¤åªº®y¼Ð­pºâ
		if (*x > 0 && IsBig5(curstr, (*x) - 1))	//¦pªG¿ï¾Ü¤¤¤å«á¥b¬q¡A´N¿ï¨ú¤U¤@­Ó¦r
			*x = (*x) + 1;
		else if (!IsBig5(curstr, *x) && (pt_x % m_CharW)*2 > m_CharW)	//¦pªG¤]¤£¬O¤¤¤å«e¥b¡A¤~¬O­^¤å
			*x = (*x) + 1;
	}

	if ( *x > m_pTermData->m_ColsPerPage )
		*x = m_pTermData->m_ColsPerPage;

	if ( pt_x < m_CharW )
		*x = 0;

}


void CTermView::PrepareDC( HDC dc )
{
/*	HGDIOBJ oldfnt = SelectObject( dc, m_Font );
//	dc.GetTextExtent( dbchars , &m_CharW, &m_CharH);	m_CharW/=2;
	dc.GetTextExtent( " " , &m_CharW, &m_CharH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBackgroundMode(wxTRANSPARENT);

	m_CharW = m_CharH/2 + m_CharPaddingX;
	m_CharH += m_CharPaddingY;
*/
}


// 2008.07.01 Added by neopro@ptt.cc
void CTermView::OnMouseWheel(WPARAM wp, LPARAM lp)
{

	int fwKeys = GET_KEYSTATE_WPARAM(wp);
	short zDelta = GET_WHEEL_DELTA_WPARAM(wp);
	int Gap = zDelta / WHEEL_DELTA;

	//if ( fwKeys & MK_RBUTTON )
	//{
		if ( Gap > 0)
			((CTelnetCon*)m_pTermData)->Send("\x1bOA",3);
		else
			((CTelnetCon*)m_pTermData)->Send("\x1bOB",3);
	//}
}

void CTermView::OnMButtonUp(UINT flags, POINTS& pt)
{
	//virtual function.
}

void CTermView::OnLButtonUp(UINT flags, POINTS& pt)
{
	//virtual function.
}

void CTermView::OnMouseMove(UINT flags, POINTS& pt)
{
	//virtual function.
}

void CTermView::OnRButtonDown(UINT flags, POINTS& pt)
{
	//virtual function.
}

void CTermView::OnRButtonUp(UINT flags, POINTS& pt)
{
	//virtual function.
}


//	This function is used to update selected region.
//	However, this piece of code is not only inefficient but also ugly.
//	Although it works, by no means can this be a good solution.
//	Leaving it temporarily because I don't have much time to do optimization.
//	Maybe someday there'll be someone who can fix it .
#define _max(x,y,z)	( x>y ? (x>z?x:(y>z?y:z)) : y>z?y:z )
#define _min(x,y,z)	( x<y ? (x<z?x:(y<z?y:z)) : y<z?y:z )
void CTermView::RedrawSel(int oldx, int oldy, int newx, int newy)
{
	//	Always remember to hide the caret before drawing.
	m_Caret.Hide();

	HDC dc = GetDC(m_hWnd);
//	PrepareDC(dc);
	HGDIOBJ oldfnt = SelectObject( dc, m_Font );

	int starty;	int	endy;
	if( m_pTermData->m_SelBlock )
	{
		starty = _min( m_pTermData->m_SelStart.y, oldy, m_pTermData->m_SelEnd.y );
		endy = _max(m_pTermData->m_SelStart.y, oldy, m_pTermData->m_SelEnd.y );
	}
	else
	{
		if( oldy < newy )	// select downward
		{	starty = oldy;	endy = newy;	}
		else	//	upward
		{	starty = newy;	endy = oldy;	}
	}

	// This is not efficient at all.  Apparaently it needs some optimization.
	int top = starty*m_CharH;
	for( int y= starty; y <= endy; y++, top+=m_CharH )
	{
//		CTermCharAttr* pAttr = m_pTermData->GetLineAttr(m_pTermData->m_Screen[starty]);
		for( int x=0; x< m_pTermData->m_ColsPerPage;  )
			x += DrawChar( dc, y, x, top );
	}

	SelectObject( dc, oldfnt );
	ReleaseDC( m_hWnd, dc );

	//	Show the caret again but only set its visibility without display it immediatly.
	m_Caret.Show( false );
}



static const char* arrszSymCheck[] =
{
	"¢c",
	"¢d",
	"¢e",
	"¢f",
	"¢g",
	"¢h",
	"¢i",
	"¢k",
	"¢l",
	"¢m",
	"¢n",
	"¢o",
	"¢p",
	"¢¨",
	"¢©",
	"¢ª",
	"¢«",
	"",
};

int CTermView::DrawChar(HDC dc, int line, int col, int top)
{
	const char* pLine = m_pTermData->m_Screen[line];
	CTermCharAttr* pAttr = m_pTermData->GetLineAttr(pLine);
	int w = 2;
	switch( pAttr[col].GetCharSet() )
	{
	case CTermCharAttr::CS_MBCS1:
		break;
	case CTermCharAttr::CS_MBCS2:
		col--;
//		This will not cause any problem at any time because 'col' always > 0.
//		In CTermData::DetectCharSets() I've done some checks to ensure that the first
//		character of every lines cannot be marked as second bytes of MBCS characters.
		break;
	default:
//	case CTermCharAttr::CS_ASCII:
		w = 1;
	}

	const char* pChar = pLine + col;
	pAttr += col;
	int left = m_CharW*col;

	bool bSel[2];	bSel[0]= IsPosInSel( col, line );
	if( w > 1 )		bSel[1] = IsPosInSel( col+1, line );

	for( int i=0; i < w; i++ )	//	do the drawing
	{
		COLORREF Fg = pAttr[i].GetFgColor( m_pColorTable );
		COLORREF Bg = pAttr[i].GetBgColor( m_pColorTable );
		// if it's property is inverse, GetFgColor & GetBgColor will swap Bg & Fg for us.

		if( bSel[i] )	// if it's selected, reverse two colors.
		{
			Fg = RGB(~GetRValue(Fg),~GetGValue(Fg),~GetBValue(Fg));
			Bg = RGB(~GetRValue(Bg),~GetGValue(Bg),~GetBValue(Bg));
		}

//		wxBrush tmpb(Bg, wxSOLID);		dc.SetBrush(tmpb);

		SetTextColor( dc, Fg );
		SetBkColor( dc, Bg );
		int bgw = m_CharW*w;
//		dc.DrawRectangle(left, top, bgw, m_CharH);
		RECT rc2;	rc2.left = left;	rc2.top = top;
		rc2.right = rc2.left + bgw;	rc2.bottom = rc2.top + m_CharH;
		ExtTextOut( dc, left, top, ETO_OPAQUE, &rc2, NULL, 0, NULL);
		if( !pAttr[i].IsBlink() || m_ShowBlink )	// If text shold be draw.
		{
			if(/* ' ' != *pChar && */'\0' != *pChar )
			{
				bool clear_type =   (11 ==((CTelnetCon*)m_pTermData)->m_Site.m_GdippDraw);
				bool quit = false;

				//Clear Type pixel hack (space)
				if (' ' == *pChar && clear_type)
					quit = false;
				else if (' ' == *pChar && !clear_type)
					quit = true;
				if (quit)		
					break;


				// Ascii character
				if ( *pChar > 0x00 && *pChar < 0x7f ){
					HGDIOBJ oldfnt = SelectObject( dc, m_FontEn );
					TextOut( dc, left, top, pChar, w );
					SelectObject( dc, oldfnt );
				}else{ // CJK character
					TextOut( dc, left, top, pChar, w );

					//Clear Type pixel hack (square box)
					if (!clear_type)
						break;
					bool error = false;
					const char* pTmp = pChar;
					char* p = NULL;

					for (;*pTmp;pTmp++)
					{
							for (int i = 0; i < sizeof(arrszSymCheck) / sizeof(arrszSymCheck[0]) ;i++)
							{
								p = strstr( (char*)pTmp, arrszSymCheck[i] );
								if (p)
								{
									error = true;
									break;
								}
							}
							if(error)
								break;
					}
					if(error)
					{									
						StretchBlt(dc, left-1, top, 1, rc2.bottom-rc2.top, dc, left-2, top, 1, rc2.bottom-rc2.top, SRCCOPY);
						StretchBlt(dc, left  , top, 1, rc2.bottom-rc2.top, dc, left-1, top, 1, rc2.bottom-rc2.top, SRCCOPY);

					}//!Clear Type pixel hack

				}//end of CJK character
			}
			if( pAttr[i].IsUnderLine() )
			{
				HPEN pen = CreatePen( PS_SOLID, 1, Fg);
				HGDIOBJ oldpen = SelectObject( dc, pen );
				int bottom = top + m_CharH - 1;
				MoveToEx( dc, left, bottom, NULL);
				LineTo( dc, left+bgw, bottom);
				SelectObject( dc, oldpen );
				DeleteObject(pen);
			}
		}
		// 2004.08.07 Added by PCMan: Draw the underline of hyperlinks.
		if( pAttr[i].IsHyperLink() ) 
		{
			int bottom = top + m_CharH - 1;
			HPEN pen = CreatePen( PS_SOLID, 1, m_HyperLinkColor);
			HGDIOBJ oldpen = SelectObject( dc, pen );

			MoveToEx( dc, left, bottom, NULL);
			LineTo( dc, left+bgw, bottom);
			SelectObject( dc, oldpen );
			DeleteObject(pen);
		}

		if( w == 1 || i>=1 || (pAttr[i].IsSameAttr(pAttr[i+1].AsShort()) && bSel[0] == bSel[1]) )
			break;

//		dc.DestroyClippingRegion();
//		dc.SetClippingRegion( left + m_CharW, top, m_CharW, m_CharH );
	}
//	dc.DestroyClippingRegion();
	return w;
}

bool CTermView::IsPosInSel(int x, int y)
{
//  If the user selected from bottom to top or from right to left, we should swap the
//	coordinates to make them correct.
	long selstartx = m_pTermData->m_SelStart.x, selstarty = m_pTermData->m_SelStart.y;
	long selendx = m_pTermData->m_SelEnd.x, selendy = m_pTermData->m_SelEnd.y;
	bool one_line_selected = m_pTermData->m_SelStart.y == selendy;
/*	if( selstarty > selendy )
	{
		int tmp = selstarty; selstarty = selendy; selendy = tmp;
		if( selstartx > selendx )
		{	int tmp = selstartx; selstartx = selendx; selendx = tmp;	}
	}
	else if( (one_line_selected||m_pTermData->m_SelBlock) && selstartx > selendx )
	{	int tmp = selstartx; selstartx = selendx; selendx = tmp;	}	*/
//-------------------------------------------------------------------
	CorrectSelPos(selstartx, selstarty, selendx, selendy );
//			Check if specified position is in selected area;
	if( y < selstarty || y > selendy )
		return false;

	if( m_pTermData->m_SelBlock )	// If selected area is a block
		return (x >= selstartx && x < selendx );
	else	// normal selection with multiple lines selected
	{
		bool x_larger_than_startx = (x >= selstartx);
		bool x_smaller_than_endx = (x < selendx);
		if( one_line_selected )	// if only one line is selected
			return ( x_larger_than_startx && x_smaller_than_endx );
		if( y == selstarty )	// If in first selected line
			return x_larger_than_startx;
		else if( y == selendy )	// If in last selected line
			return x_smaller_than_endx;
	}
	return true;
}

void CTermView::CorrectSelPos(long &selstartx, long &selstarty, long &selendx, long &selendy)
{
//  If the user selected from bottom to top or from right to left, we should swap the
//	coordinates to make them correct.
	bool one_line_selected = m_pTermData->m_SelStart.y == selendy;
	if( selstarty > selendy )
	{
		int tmp = selstarty; selstarty = selendy; selendy = tmp;
		tmp = selstartx; selstartx = selendx; selendx = tmp;
	}
	if( (one_line_selected ||m_pTermData->m_SelBlock) && selstartx > selendx )
	{	int tmp = selstartx; selstartx = selendx; selendx = tmp;	}
//-------------------------------------------------------------------
}


void CTermView::CopySelectedText( bool bWithColor , bool trim)
{
//	Get selected text.
	string text;
	if(bWithColor)
		text = m_pTermData->GetSelectedTextWithColor(trim);
	else
		text = m_pTermData->GetSelectedText(trim);

	if( OpenClipboard( m_hWnd ) )
	{
		int TEXT_LEN = text.length();
		
		EmptyClipboard();
		if ( !bWithColor && (GetVersion() < 0x80000000) )
		{
			wchar_t* pwtmp = new wchar_t[ TEXT_LEN+1 ];
			memset(pwtmp, 0, (TEXT_LEN+1)*sizeof(wchar_t));

			if (this->GetCodePage() == 950)
				m_ucs2conv.Big52Ucs2(text.c_str(), pwtmp, TEXT_LEN);
			else
				::MultiByteToWideChar(this->GetCodePage(), 0, text.c_str(), TEXT_LEN, pwtmp, TEXT_LEN);

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
			memcpy( buf, text.c_str(), TEXT_LEN+1);
			GlobalUnlock(hmem);
			SetClipboardData( CF_TEXT, hmem );
		}
		CloseClipboard();
	}
}


LRESULT CTermView::WndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch( msg )
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = (HDC)BeginPaint( m_hWnd, &ps );
			OnPaint( dc, ps.rcPaint );
			EndPaint( m_hWnd, &ps );
			return 0;
		}
		break;
	case WM_TIMER:
		switch (wp)
		{
			case ID_BLINK_TIMER:
				OnBlinkTimer();
				break;
			case ID_ANTIIDLE_TIMER:
				((CTelnetCon*)m_pTermData)->OnTimer();
				break;
		}
		return 0;
		break;
	case WM_MOUSEMOVE:
		OnMouseMove( wp, MAKEPOINTS(lp) );
		break;
	case WM_LBUTTONDOWN:
		OnLButtonDown( wp, MAKEPOINTS(lp) );
		break;
	case WM_LBUTTONUP:
		OnLButtonUp( wp, MAKEPOINTS(lp) );
		break;
	case WM_RBUTTONDOWN:
		OnRButtonDown( wp, MAKEPOINTS(lp) );
		break;
	case WM_RBUTTONUP:
		OnRButtonUp( wp, MAKEPOINTS(lp) );
		break;
	case WM_MOUSEWHEEL:
		OnMouseWheel( wp, lp );
		break;
	case WM_MBUTTONUP:
		OnMButtonUp( wp, MAKEPOINTS(lp) );
		break;
	case WM_SIZE:
		OnSize( wp, LOWORD(lp), HIWORD(lp) );
		return 0;
		break;
	case WM_DESTROY:
		SetWindowLongPtr( m_hWnd, GWL_USERDATA, NULL );
		break;
	}
	return DefWindowProc( m_hWnd, msg, wp, lp );
}

void CTermView::RecalcCharExtent()
{
	HDC dc = GetDC(m_hWnd);
	HGDIOBJ oldfnt = SelectObject( dc, m_Font );
	SIZE sz;
	GetTextExtentPoint32W ( dc, L"¡@", 1, &sz );
	SelectObject( dc, oldfnt );
	ReleaseDC( m_hWnd, dc);
	m_CharH = sz.cy;
	m_CharW = sz.cx/2 + (sz.cx % 2);
}

void CTermView::updateFont(bool clear_type)
{
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	long cx = rc.right - rc.left;
	long cy = rc.bottom - rc.top;

	LOGFONT lf;

	GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
	lf.lfCharSet = DEFAULT_CHARSET;
	if (clear_type)
		lf.lfQuality = 5;
	/*else
		lf.lfQuality = DEFAULT_QUALITY;*/
	
	if( m_pTermData )
	{
		if ( ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == 0 || ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == -1)	// 0 = stretch, -1 = no stretch
		{
			lf.lfWidth = cx/m_pTermData->m_ColsPerPage;
			lf.lfHeight = cy/m_pTermData->m_RowsPerPage;
			if( lf.lfHeight > lf.lfWidth * 2 && ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == -1 )
				lf.lfHeight = lf.lfWidth * 2;
			lf.lfHeight = -lf.lfHeight;
		}
		else
		{
			HDC dc = ::GetDC(m_hWnd);
			lf.lfHeight = -(int)(fabs( (double) (((CTelnetCon*)m_pTermData)->m_Site.m_FontSize*10)*GetDeviceCaps(dc,LOGPIXELSY)/72)/10.0+0.5);
			lf.lfWidth = 0;
		}
	}

	if( ! fontFace.empty() )
		strcpy( lf.lfFaceName, fontFace.c_str());

	HFONT nf = CreateFontIndirect( &lf );
	if( nf )
	{
		if( m_Font )
			DeleteObject( m_Font );
		m_Font = nf;
	}

	RecalcCharExtent();
	m_Caret.SetSize(m_CharW, 2);
}

void CTermView::updateFontEn(bool clear_type)
{
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	long cx = rc.right - rc.left;
	long cy = rc.bottom - rc.top;

	LOGFONT lf;

	GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
	lf.lfCharSet = DEFAULT_CHARSET;
	if (clear_type)
		lf.lfQuality = 5;
	/*else
		lf.lfQuality = DEFAULT_QUALITY;*/
	
	if( m_pTermData )
	{
		if ( ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == 0 || ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == -1 )	// 0 = stretch, -1 = no stretch
		{

			lf.lfWidth = cx/m_pTermData->m_ColsPerPage;
			lf.lfHeight = cy/m_pTermData->m_RowsPerPage;
			if( lf.lfHeight > lf.lfWidth * 2 && ((CTelnetCon*)m_pTermData)->m_Site.m_FontSize == -1 )
				lf.lfHeight = lf.lfWidth * 2;
			lf.lfHeight = -lf.lfHeight;
		}
		else
		{
			HDC dc = ::GetDC(m_hWnd);
			lf.lfHeight = -(int)(fabs( (double) (((CTelnetCon*)m_pTermData)->m_Site.m_FontSize*10)*GetDeviceCaps(dc,LOGPIXELSY)/72)/10.0+0.5);
			lf.lfWidth = 0;
		}
	}

	if( ! fontFaceEn.empty() ){
		strcpy( lf.lfFaceName, fontFaceEn.c_str());
	}else{
		if (!fontFace.empty())
			strcpy( lf.lfFaceName, fontFace.c_str());
	}


	HFONT nf = CreateFontIndirect( &lf );
	if( nf )
	{
		if( m_FontEn )
			DeleteObject( m_FontEn );
		m_FontEn = nf;
	}

	//RecalcCharExtent();
	//m_Caret.SetSize(m_CharW, 2);
}

void CTermView::setFontFace(const char* font_face, bool clear_type)
{
	fontFace = font_face;
	updateFont(clear_type);
}

void CTermView::setFontFaceEn(const char* font_face, bool clear_type)
{
	fontFaceEn = font_face;
	updateFontEn(clear_type);
}

//
int CTermView::TextOut(HDC& dc, int x, int y, const char* lpszString, UINT nCount)
{
	wchar_t wbuf[161];// maximum of every line on the bbs screen.

	int nGdraw = ((CTelnetCon*)m_pTermData)->m_Site.m_GdippDraw;

	unsigned int cp_id = GetCodePage();
	if (cp_id != 950)
	{
		// in¥þ¬°dbcs, in[n]=out[n/2+1]; in¥þ¬°acsii, in[n]=out[n+1]
		memset(wbuf, 0, (nCount+1)*sizeof(wchar_t) ); // <-- in bytes
		::MultiByteToWideChar(cp_id, 0, lpszString, nCount, wbuf, nCount+1);

		if ( nGdraw>0 && nGdraw<5 ){
			::g_Scale = nGdraw;
			return ::IMPL_TextOutW( dc, x, y, wbuf, wcslen(wbuf) );
		}else{
			::g_Scale = 0;
			return ::TextOutW(dc, x,y,wbuf, wcslen(wbuf));
		}
	}

	size_t size = m_ucs2conv.Big52Ucs2(lpszString, wbuf, nCount);
	wbuf[size]=0;

	if ( nGdraw>0 && nGdraw<5 ){
		::g_Scale = nGdraw;
		return ::IMPL_TextOutW( dc, x, y, wbuf, size );
	}
	else{
		::g_Scale = 0;
		return ::TextOutW(dc, x,y,wbuf, size);	
	}
}

//
bool CTermView::HyperLinkHitTest(int x, int y, int* start, int* end)
{
        char* pline = m_pTermData->m_Screen[y];
        CTermCharAttr* pattr = m_pTermData->GetLineAttr(pline);
        if( x > 0 && x < m_pTermData->m_ColsPerPage && pattr[x].IsHyperLink() )
        {
                int _start, _end;
                for( _start = x-1; _start > 0 && pattr[_start].IsHyperLink(); _start-- );
                if( !pattr[_start].IsHyperLink() )
                        _start++;
                for( _end = x+1; _end < m_pTermData->m_ColsPerPage && pattr[_end].IsHyperLink(); _end++ );
                *start = _start;
                *end = _end;                
                return true;
        }
        return false;
}

void CTermView::UpdateCaretPos()
{
	if( !m_pTermData )
		return;

	int x = m_pTermData->m_CaretPos.x * m_CharW /*+ m_LeftMargin*/;
	int y = (m_pTermData->m_CaretPos.y + 1) * m_CharH - 2 /*+ m_TopMargin*/;
	m_Caret.Move( x, y );
}