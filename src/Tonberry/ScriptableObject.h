/////////////////////////////////////////////////////////////////////////////
// Name:        ScriptableObject.h
// Purpose:     
// Author:      
// E-mail:      
// Created:     
// Copyright:   
// License:     GPL : http://www.gnu.org/licenses/gpl.html
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

//#include "stdafx.h"
#include <nscore.h>
#include "plugin.h"


class CPCManScriptable : public NPObject
{
public:
    CPCManScriptable(NPP inst);
    ~CPCManScriptable();

	// NPP methods
	bool Invoke(NPIdentifier methodName,
				   const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool InvokeDefault(const NPVariant *args,
						  uint32_t argCount, NPVariant *result);
	bool HasMethod(NPIdentifier methodName);
	bool HasProperty(NPIdentifier propertyName);
	bool GetProperty(NPIdentifier propertyName, NPVariant *result);
	bool SetProperty(NPIdentifier propertyName, const NPVariant *value);
	bool RemoveProperty(NPIdentifier propertyName);
	
	// Static callback methods.
	static NPObject *sAllocate(NPP inst, NPClass *);
	static void sDeallocate(NPObject *npobj);
	static bool sInvoke(NPObject* obj, NPIdentifier methodName,
				   const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool sInvokeDefault(NPObject *obj, const NPVariant *args,
						  uint32_t argCount, NPVariant *result);
	static bool sHasMethod(NPObject* obj, NPIdentifier methodName);
	static bool sHasProperty(NPObject *obj, NPIdentifier propertyName);
	static bool sGetProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result);
	static bool sSetProperty(NPObject* obj, NPIdentifier propertyName,
                     const NPVariant *value);
	static bool sRemoveProperty(NPObject* obj, NPIdentifier propertyName);

	static NPClass MyClass;



protected:
    nsPluginInstance *m_pInstance;

};
