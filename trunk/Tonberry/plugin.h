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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "pluginbase.h"
#include "site.h"

class CTelnetView;
class CPCManScriptable;
class nsPluginInstance : public nsPluginInstanceBase
{
public:
  nsPluginInstance(nsPluginCreateData* aCreateDataStruct);
  ~nsPluginInstance();

  CPCManScriptable* m_pPCManScriptable;
  CPCManScriptable* getScriptableObject();

  NPBool init(NPWindow* aWindow);
  void shut();
  NPBool isInitialized();

  // locals
  const char * getVersion();
  //NPP getInstance(){	return mInstance; }

  static string unicodeToAnsi( const wchar_t *unistr );
  CTelnetView* m_pView;
  //  char* m_FilePath;

  // we need to provide implementation of this method as it will be
  // used by Mozilla to retrive the scriptable peer
  NPError	GetValue(NPPVariable variable, void *value);

  // scriptable
  void focus();
  void connect( const char* url );
  void setFontFace(const char* name);
  void setFontFaceEn(const char* name);
  void setFontSize(short size);
  void setAntiIdle( long idleTime, const char* idleString );
  void setEscape(const char *esc);
  void setAutoDbcsDetection(unsigned int value);
  void setSearchEngineData( const char* url, const char* param_name, int num );
  void setUseMouseBrowsing( bool value );
  void setUseTextDragDrop( bool value );
  void setLoadUrlInBackGround( bool value );
  void setGesturesSensitivityX( int value );
  void setGesturesUseXAxisOnly( bool value );
  void setUseHelperWnd( bool value );
  void setUseMouseWheelHack( bool value );
  void setMouseOverEffect( int value );
  long getHWND();

  WNDPROC m_OldWndProc;

protected:
	void CreateScriptableObject();


private:
  bool securityCheckOK;
  NPP mInstance;
  NPBool mInitialized;
  HWND mhWnd;
  CSite m_Site;
};

#endif // __PLUGIN_H__