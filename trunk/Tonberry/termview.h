/////////////////////////////////////////////////////////////////////////////
// Name:        termview.h
// Purpose:     Displey terminal screen data sroted in CTermData.
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.17
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////


#ifndef TERMVIEW_H
#define TERMVIEW_H

#ifdef __GNUG__
  #pragma interface "termview.h"
#endif

#include <windows.h> // inheriting class's header file
#include "caret.h"	// Added by ClassView
#include <string>
#include "Ucs2Conv.h"



class CUsc2conv;

/*
 * Terminal View Base
 */
class CTelnetCon;
class CTermData;
class CHyperLink;
class CTermView
{
	friend class CTermData;

	public:
		void updateFont(bool clear_type=false);
		void RecalcCharExtent();
		void updateFontEn(bool clear_type=false);
		virtual LRESULT WndProc( UINT msg, WPARAM wp, LPARAM lp);
		HCURSOR m_ArrowCursor;
		static LRESULT  CALLBACK _WndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
		HWND m_hWnd;
		HWND m_parent;
		enum{ IDC_IM_PROXY = 10, ID_BLINK_TIMER = 100, ID_ANTIIDLE_TIMER = 101};
		enum{ WM_INIT = WM_APP+104};
		HCURSOR m_HandCursor, m_ExitCursor, m_PageDownCursor, m_PageUpCursor, m_BullsEyeCursor, m_HomeCursor, m_EndCursor, m_DragtextCursor;
		HCURSOR m_oldDragCursor; //2008.07.03 added.
		void CorrectSelPos(long &selstartx, long &selstarty, long &selendx, long &selendy);
		bool IsPosInSel(int x, int y);
		int DrawChar( HDC dc, int line, int col, int top );
		void RedrawSel( int oldx, int oldy, int newx, int newy );
		void PrepareDC( HDC dc );
		void PointToLineCol(int *x, int *y, bool adjust_x = false);
		void CopySelectedText( bool bWithColor, bool trim = true );

		int TextOut(HDC& dc, int x, int y, const char* lpszString, UINT nCount);

		int m_CharW;
		int m_CharH;
		// class constructor
		CTermView(HWND parent);
		// class destructor
		~CTermView();
		// No description
		void OnPaint( HDC dc, RECT& update_rect );
		void OnBlinkTimer();

		void SetCharPaddingX(int CharPaddingX){m_CharPaddingX=CharPaddingX;}
		void SetCharPaddingY(int CharPaddingY){m_CharPaddingY=CharPaddingY;}

		// Terminal screen data to be displayed.
		CTermData* m_pTermData;

		HFONT m_Font, m_FontEn;

		CCaret m_Caret;

		COLORREF m_HyperLinkColor;
		COLORREF* m_pColorTable;
		CHyperLink* m_pHyperLink;

		CUcs2Conv m_ucs2conv;		

	protected:
		bool m_IsSelecting;
		bool m_beginDrag; // 2008.07.03  for Drag and drop gestures support
		HMENU m_Popup, m_Popup_Nonsel;
		WNDPROC m_OldWndProc;
		bool m_ShowBlink;
		int m_CharPaddingX;
		int m_CharPaddingY;

		void OnSize( int SizeType, int cx, int cy );

		virtual void OnMouseMove(UINT flags, POINTS& pt);
		virtual void OnMouseWheel(WPARAM wp, LPARAM lp);
		virtual void OnLButtonDown(UINT flags, POINTS& pt);
		virtual void OnLButtonUp(UINT flags, POINTS& pt);
		virtual void OnRButtonDown(UINT flags, POINTS& pt);
		virtual void OnRButtonUp(UINT flags, POINTS& pt);
		virtual void OnMButtonUp(UINT flags, POINTS& pt);

		void UpdateCaretPos();
		bool HyperLinkHitTest(int x, int y, int* start, int* end);

		std::string fontFace, fontFaceEn;

		unsigned int m_CodePageID;
		int m_CursorState;


public:
	void setFontFace(const char* font_face, bool clear_type=false);
	void setFontFaceEn(const char* font_face, bool clear_type=false);

	inline unsigned int GetCodePage(){ return m_CodePageID; }
	void SetCodePage(unsigned int nCodePageID){ m_CodePageID = nCodePageID; }
};

#endif // TERMVIEW_H

