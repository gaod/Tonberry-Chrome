/////////////////////////////////////////////////////////////////////////////
// Name:        stringutil.h
// Purpose:     Some string related utilities
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.24
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#ifndef	__STRINGUTIL_H__
#define	__STRINGUTIL_H__

#include <string>

	std::string EscapeStr(const char* pstr);
	std::string UnEscapeStr(const char* pstr);
	inline void EscapeStr(std::string& str){ str = EscapeStr(str.c_str()); }
	inline void UnEscapeStr(std::string& str){ str = UnEscapeStr(str.c_str()); }

	std::string ConvertFromCRLF(const char* pstr);
	std::string ConvertToCRLF(const char* pstr);
	inline void ConvertFromCRLF(std::string& str){ str = ConvertFromCRLF(str.c_str()); }
	inline void ConvertToCRLF(std::string& str){ str = ConvertToCRLF(str.c_str()); }
	int strncmpi(const char* str1, const char* str2, size_t len);
	bool str_replace( std::string& str, const char* substr, const char* new_substr );

	void strstriptail(char* str);
	void str_striptail(std::string& str);

	bool IsBig5(const char * str, const char * ppos);

	inline int get_chw(const char * str)
	{	return (*str > 0 && *str < 127) ? 1 : 2;	}

	inline bool IsBig5(const char * str, int pos)
	{
		return IsBig5(str, str + pos);
	}

#endif
