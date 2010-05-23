/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "npapi.h"


#include <windows.h>
#include <tchar.h>
#include <windowsx.h>

//#include <wininet.h>
//  #include <mbstring.h>

#include "plugin.h"
#include "ScriptableObject.h"
#include "telnetview.h"
#include "site.h"
#include "usermessage.h"


//////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct);
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(nsPluginCreateData* aCreateDataStruct) : nsPluginInstanceBase(),
  mInstance(aCreateDataStruct->instance),
  mInitialized(FALSE)
{
  mhWnd = NULL;
  m_pView = NULL;
  m_pPCManScriptable = NULL;

  m_Site.m_ColsPerPage = 80;
  m_Site.m_RowsPerPage = 24;
  m_Site.m_CRLF = 0;
  m_Site.m_TermType = "vt100";
  m_Site.m_AutoDbcsDetection = 0;
  
  if( aCreateDataStruct->mode==NP_EMBED )
  {
    for( int i=0; i < aCreateDataStruct->argc; ++i )
    {
    }
  }
}

nsPluginInstance::~nsPluginInstance()
{
	if(m_pPCManScriptable)
	{
		NPN_ReleaseObject(m_pPCManScriptable);
	}
}

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);


NPBool nsPluginInstance::init(NPWindow* aWindow)
{
  if(aWindow == NULL)
    return FALSE;

  mhWnd = (HWND)aWindow->window;
  if(mhWnd == NULL)
    return FALSE;

  // associate window with our nsPluginInstance object so we can access 
  // it in the window procedure
  SetWindowLong(mhWnd, GWL_USERDATA, (LONG)this);
  m_OldWndProc = (WNDPROC)SetWindowLong( mhWnd, GWL_WNDPROC, (long)PluginWinProc);

 
  m_pView = new CTelnetView(mhWnd);

  RECT rc;
  GetClientRect( mhWnd, &rc );
  MoveWindow( m_pView->m_hWnd, 0, 0, rc.right, rc.bottom, TRUE );

  m_Site.m_URL = "ptt.cc";
  m_pView->NewCon(m_Site);
  m_pView->setFontFace(m_Site.m_FontFace.c_str());	m_pView->setFontFaceEn(m_Site.m_FontFaceEn.c_str());
  SetFocus(m_pView->m_hWnd);

  mInitialized = TRUE;
  return TRUE;
}

void nsPluginInstance::shut()
{
  // subclass it back
  (WNDPROC)SetWindowLong( mhWnd, GWL_WNDPROC, (long)m_OldWndProc);

  if(m_pView)
  delete m_pView;
  m_pView = NULL;
  mhWnd = NULL;
  mInitialized = FALSE;
}

NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}

const char * nsPluginInstance::getVersion()
{
  return NPN_UserAgent(mInstance);
}

static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  LRESULT ret = 0;
  nsPluginInstance* pi;
  pi = (nsPluginInstance*)GetWindowLong( hWnd, GWL_USERDATA);

  switch (msg)
  {
  case WM_SIZE:
    if(pi) {
      MoveWindow( pi->m_pView->m_hWnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE );
    }
    break;
	case WM_SETFOCUS:
		if(pi){
			if (pi->m_pView)
				SetFocus(pi->m_pView->m_hWnd);
		}
	break;
	case WM_USER_CONNECTION_CLOSE:
		{
			char buf[256];
			 SYSTEMTIME st;
			 ::GetLocalTime(&st);
			 wsprintf(buf, "連線已經中斷。 %u:%u:%u", st.wHour, st.wMinute, st.wSecond);   
			::MessageBox(hWnd, buf, "提醒",MB_ICONQUESTION);	
		}
	break;

    default:
    ret = DefWindowProc(hWnd, msg, wParam, lParam);
      break;
  }
  return ret;
}


string nsPluginInstance::unicodeToAnsi( const wchar_t *unistr )
{
  int l = WideCharToMultiByte( CP_THREAD_ACP, 0, unistr, -1, NULL, 0, NULL, NULL )+1;
  char* buf = new char[l];
  WideCharToMultiByte( CP_THREAD_ACP, 0, unistr, -1, buf, l, NULL, NULL );
  string ret(buf);
  delete []buf;
  return ret;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
//

void nsPluginInstance::CreateScriptableObject()
{
	NPObject *so = NPN_CreateObject(mInstance, &CPCManScriptable::MyClass);
	m_pPCManScriptable = (CPCManScriptable *) so;

	// We retain it until we are released ourselves.
	NPN_RetainObject(so);
}

NPError  nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
	switch(aVariable) {
	default:
		return NPERR_GENERIC_ERROR;
	case NPPVpluginNameString:
		*((char **)aValue) = "Tonberry";
		break;
	case NPPVpluginDescriptionString:
		*((char **)aValue) = "Tonberry plugin.";
		break;
	case NPPVpluginScriptableNPObject:
        {
			NPObject *so = (NPObject *) m_pPCManScriptable;

			if(!so)
			{
				CreateScriptableObject();
				so = (NPObject *) m_pPCManScriptable;
			}

			if(so)
			{
				NPN_RetainObject(so);
			}

			*(NPObject **)aValue = so;
        }
		break;
#if defined(XULRUNNER_SDK)
	case NPPVpluginNeedsXEmbed:
		*((PRBool *)value) = PR_FALSE;
		break;
#endif
	}
	return NPERR_NO_ERROR;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
//
CPCManScriptable* nsPluginInstance::getScriptableObject()
{
	if(!m_pPCManScriptable)
	{
		CreateScriptableObject();
	}
	return m_pPCManScriptable;
}

// scriptable


void nsPluginInstance::connect( const char* url )
{
  m_Site.m_URL = url;
  if (m_pView && m_pView->m_hWnd)
    m_pView->NewCon( m_Site );
}

void nsPluginInstance::setFontFace(const char* name)
{
  //m_Site.m_FontFace = unicodeToAnsi( name );
  if (m_pView && m_pView->m_hWnd)
    m_pView->setFontFace( m_Site.m_FontFace.c_str() );
}

void nsPluginInstance::setFontFaceEn(const char* name)
{
  //m_Site.m_FontFaceEn = unicodeToAnsi( name );
  if (m_pView && m_pView->m_hWnd)
    m_pView->setFontFaceEn( m_Site.m_FontFaceEn.c_str() );
}

void nsPluginInstance::setFontSize(short size)
{
  // Not implemented yet
}

void nsPluginInstance::setAntiIdle( long idleTime, const char* idleString )
{
  m_Site.m_AntiIdle = idleTime;
  m_Site.m_AntiIdleStr = idleString;
}

void nsPluginInstance::setEscape(const char *esc)
{
  m_Site.m_ESCConv = esc;
}

// 2008.06.26 added by neopro@ptt.cc
void nsPluginInstance::setAutoDbcsDetection(unsigned int value)
{
  m_Site.m_AutoDbcsDetection = value;
}

// 2008.06.27
void nsPluginInstance::setSearchEngineData( const char* url, const char* param_name, int num )
{
  if (m_pView && m_pView->m_hWnd)
    m_pView->SetSearchEngineData(  url, param_name, num );
}

void nsPluginInstance::setUseMouseBrowsing(bool value)
{
  m_Site.m_UseMouseBrowsing = value;
}

void nsPluginInstance::setUseTextDragDrop(bool value)
{
  m_Site.m_UseTextDragDrop = value;
}

void nsPluginInstance::setLoadUrlInBackGround(bool value)
{
  m_Site.m_LoadUrlInBackGround = value;
}

void nsPluginInstance::setGesturesSensitivityX( int value )
{
  m_Site.m_GesturesSensitivityX = value;
}

void nsPluginInstance::setGesturesUseXAxisOnly( bool value )
{
  m_Site.m_GesturesUseXAxisOnly = value;
}

void nsPluginInstance::setUseHelperWnd( bool value )
{
  m_Site.m_UseHelperWnd = value;
}

void nsPluginInstance::setUseMouseWheelHack( bool value )
{
  //m_Site.m_UseMouseWheelHack = value;
}

void nsPluginInstance::setMouseOverEffect( int value )
{
  m_Site.m_MouseOverEffect = value;
}

void nsPluginInstance::focus()
{
  if (m_pView && m_pView->m_hWnd)
    SetFocus(m_pView->m_hWnd);
}

long nsPluginInstance::getHWND()
{
  if (m_pView && m_pView->m_hWnd)
    return long(m_pView->m_hWnd);
  else
	return -1;
}
