/////////////////////////////////////////////////////////////////////////////
// Name:        ScriptableObject.cpp
// Purpose:     npruntime scriptable support for 
// Author:      
// E-mail:     
// Created:     2010.
// Copyright:  
// License:     GPL : http://www.gnu.org/licenses/gpl.html
//
/////////////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
//#include "strsafe.h"
#include <npapi.h>
//#include <npupp.h>
#include <npruntime.h>

#include "nsCOMPtr.h"


#include "nsStringAPI.h"
#include "nsEmbedString.h"

#include <windowsx.h>

//#include <wininet.h>

//////#include "ietab.h"
#include "scriptableobject.h"

#define SET_VAR_BOOL(npvariant, val) { npvariant->type = NPVariantType_Bool; npvariant->value.boolValue = val; }
#define SET_VAR_INT32(npvariant, val) { npvariant->type = NPVariantType_Int32; npvariant->value.intValue = val; }

#define STRZ_OUT_TO_NPVARIANT(pszIn, npvar) { \
	int nLen = strlen(pszIn); \
	char *pszOut = (char *) NPN_MemAlloc(nLen+1); \
	strcpy_s(pszOut, nLen+1, pszIn); \
	STRINGZ_TO_NPVARIANT(pszOut, npvar); }


NPClass CPCManScriptable::MyClass =
{
    NP_CLASS_STRUCT_VERSION,
    CPCManScriptable::sAllocate,
    CPCManScriptable::sDeallocate,
	NULL,
    CPCManScriptable::sHasMethod,
    CPCManScriptable::sInvoke,
    CPCManScriptable::sInvokeDefault,
    CPCManScriptable::sHasProperty,
    CPCManScriptable::sGetProperty,
    CPCManScriptable::sSetProperty,
    CPCManScriptable::sRemoveProperty,
};

static const char *arrszProperties[] =
{
	"foo"
};

static const char *arrszMethods[] =
{
	"connect",
	"setFontFace",
	"setFontFaceEn",
	"setFontSize",
	"setAntiIdle",
	"setEscape",
	"setAutoDbcsDetection",
	"setSearchEngineData",
	"setUseMouseBrowsing",
	"setUseTextDragDrop",
	"setLoadUrlInBackGround",
	"setGesturesSensitivityX",
	"setGesturesUseXAxisOnly",
	"setUseMouseWheelHack",
	"setMouseOverEffect",
	"focus",
	"getHWND"
};

// The scriptable object
CPCManScriptable::CPCManScriptable(NPP inst)
{
    m_pInstance = (nsPluginInstance *)inst->pdata;
}

CPCManScriptable::~CPCManScriptable()
{
	//if(m_pRequestTarget && (m_pRequestTarget->referenceCount > 0))
		//NPN_ReleaseObject(m_pRequestTarget);
}


// We use our own allocation routine so we can allocate our own IETab Object.
NPObject *CPCManScriptable::sAllocate(NPP inst, NPClass *pClass)
{
    return new CPCManScriptable(inst);
}

void CPCManScriptable::sDeallocate(NPObject *npobj)
{
    delete (CPCManScriptable *) npobj;
}

bool CPCManScriptable::sInvoke(NPObject* obj, NPIdentifier methodName,
				   const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->Invoke(methodName, args, argCount, result);
}

bool CPCManScriptable::sInvokeDefault(NPObject *obj, const NPVariant *args,
						  uint32_t argCount, NPVariant *result)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->InvokeDefault(args, argCount, result);
}

bool CPCManScriptable::sHasMethod(NPObject* obj, NPIdentifier methodName)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->HasMethod(methodName);
}

bool CPCManScriptable::sHasProperty(NPObject *obj, NPIdentifier propertyName)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->HasProperty(propertyName);
}

bool CPCManScriptable::sGetProperty(NPObject *obj, NPIdentifier propertyName,
						NPVariant *result)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->GetProperty(propertyName, result);
}

bool CPCManScriptable::sSetProperty(NPObject* obj, NPIdentifier propertyName,
                 const NPVariant *value)
{
	CPCManScriptable *pObj = (CPCManScriptable *) obj;
	return pObj->SetProperty(propertyName, value);
}

bool CPCManScriptable::sRemoveProperty(NPObject* obj, NPIdentifier propertyName)
{
	return false;
}



bool CPCManScriptable::Invoke(NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    char *name = NPN_UTF8FromIdentifier(methodName);
	if(!name)
		return false;

	if(!strcmp(name, "connect"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->connect(args[0].value.stringValue.utf8characters);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setFontFace"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setFontFace(args[0].value.stringValue.utf8characters);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setFontFaceEn"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setFontFaceEn(args[0].value.stringValue.utf8characters);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setFontSize"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setFontSize(args[0].value.intValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setAntiIdle"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>1 )
			m_pInstance->setAntiIdle(args[0].value.intValue, args[1].value.stringValue.utf8characters);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setEscape"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setEscape(args[0].value.stringValue.utf8characters);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setAutoDbcsDetection"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setAutoDbcsDetection(args[0].value.intValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setSearchEngineData"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>2 )
			m_pInstance->setSearchEngineData(args[0].value.stringValue.utf8characters, args[1].value.stringValue.utf8characters, args[2].value.intValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setUseMouseBrowsing"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setUseMouseBrowsing(args[0].value.boolValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setUseTextDragDrop"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setUseTextDragDrop(args[0].value.boolValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setLoadUrlInBackGround"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setLoadUrlInBackGround(args[0].value.boolValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setGesturesSensitivityX"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setGesturesSensitivityX(args[0].value.intValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setGesturesUseXAxisOnly"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setGesturesUseXAxisOnly(args[0].value.boolValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setUseMouseWheelHack"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setUseMouseWheelHack(args[0].value.boolValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "setMouseOverEffect"))
	{
		SET_VAR_BOOL(result, true);
		if( argCount>0 )
			m_pInstance->setMouseOverEffect(args[0].value.intValue);
		else
			SET_VAR_BOOL(result, false);
	}
	else if(!strcmp(name, "focus"))
	{
		SET_VAR_BOOL(result, true);
		m_pInstance->focus();
	}

	else if(!strcmp(name, "getHWND"))
	{
		SET_VAR_INT32(result, m_pInstance->getHWND());
	}

	else
	{
		return false;
	}

	return true;
}

bool CPCManScriptable::InvokeDefault(const NPVariant *args,
						  uint32_t argCount, NPVariant *result)
{
	return true;
}

bool CPCManScriptable::HasMethod(NPIdentifier methodName)
{
    char *name = NPN_UTF8FromIdentifier(methodName);
	if(!name)
		return false;

	for(int i=0; i < ARRAYSIZE(arrszMethods); i++)
	{
	    if(!strcmp((const char *)name, arrszMethods[i]))
			return true;
	}
	return false;
}

bool CPCManScriptable::HasProperty(NPIdentifier propertyName)
{
    char *name = NPN_UTF8FromIdentifier(propertyName);
	if(!name)
		return false;

	for(int i=0; i < ARRAYSIZE(arrszProperties); i++)
	{
	    if(!strcmp((const char *)name, arrszProperties[i]))
			return true;
	}
	return false;
}

bool CPCManScriptable::GetProperty(NPIdentifier propertyName, NPVariant *result)
{
    char *name = NPN_UTF8FromIdentifier(propertyName);
	if(!name)
		return false;

	return true;
}

bool CPCManScriptable::SetProperty(NPIdentifier propertyName, const NPVariant *value)
{
    char *name = NPN_UTF8FromIdentifier(propertyName);
	if(!name)
		return false;

	return true;
}

