/////////////////////////////////////////////////////////////////////////////
// Name:        stringutil.cpp
// Purpose:     Some string related utilities
// Author:      PCMan (HZY)   http://pcman.ptt.cc/
// E-mail:      hzysoft@sina.com.tw
// Created:     2004.7.24
// Copyright:   (C) 2004 PCMan
// Licence:     GPL : http://www.gnu.org/licenses/gpl.html
// Modified by:
/////////////////////////////////////////////////////////////////////////////

#include "stringutil.h"

using namespace std;

int strncmpi(const char* str1, const char* str2, size_t len)
{
	size_t i = 0;	int r;
	for( ; str1[i] && str2[i] && i < len; i++ )
	{
		r = toupper( (unsigned char)str1[i] ) - toupper( (unsigned char)str2[i] );
		if( r )
			return r;
	}
	return 0;
}


// Note by PCMan:
// EscapeStr() & UnEscapeStr() :
// Encode non-printable characters with '^' and '\\'
// \x?? means a hexadecimal integer. ex: \x0d = '\r' and \x0a = '\n'
// \\ means \ character
// ^ symbol means CTRL, ex: ^U means CTRL+U, or '\x15'
// \^ means ^ character

// 2004/08/05 modified by PCMan
string EscapeStr(const char* pstr)
{
	string _str;
	for( ; *pstr ; pstr++ )
	{
		unsigned ch = (unsigned char)*pstr;
		if( ' ' > ch  ) // if it's not a printable character.
		{
			char hex[4];
			sprintf(hex, "\\%02x", (unsigned char)*pstr );
			_str += hex;
			pstr++;
			continue;
		}
		else if( '^' == ch || '\\' == ch )
			_str += '\\';
		_str += ch;
	}
	return _str;
}

// 2004/08/05 modified by PCMan
string UnEscapeStr(const char* pstr)
{
	string _str;
	for( ; *pstr ; pstr++ )
	{
		unsigned ch = (unsigned char)*pstr;
		if( '\\' == ch )
		{
			switch( pstr[1] )
			{
			case '\\':	//	"\\" means '\' character
				pstr++;
				_str += '\\';
				break;
			case '^':	//	"\^" means '^' character
				pstr++;
				_str += '^';
				break;
			default:	//	\x?? means a hexadecimal integer. ex: \x0d = '\r' and \x0a = '\n'
				char hex[4];
				int val;
				sscanf(hex, "%2x", &val);
				_str += (char)val;
				continue;
			}
		}
		else if( '^' == ch )		// ^ symbol means CTRL, ex: ^U means CTRL+U, or '\x15'
		{
			ch = (unsigned char)pstr[1];
			if( ch >= '@' && ch <= '_' )
			{
				pstr ++;
				_str += (char)(ch - '@');
				continue;
			}
		}
		_str += *pstr;
	}
	return _str;
}

string ConvertFromCRLF(const char* pstr)
{
	string ret;
	for( ; *pstr; pstr++ )
	{
		if( *pstr == '\r' )	// CR
		{
			ret += '\n';
			if(*(pstr+1) == '\n')
				pstr++;
		}
		else
			ret += *pstr;
	}
	return ret;
}

string ConvertToCRLF(const char* pstr)
{
	string ret;
	for( ; *pstr; pstr++ )
	{
		if( *pstr == '\r' )	// CR
		{
			ret += "\r\n";
			if(*(pstr+1) == '\n')
				pstr++;
		}
		else if( *pstr == '\n' )	// LF
			ret += "\r\n";
		else
			ret += *pstr;
	}
	return ret;
}


bool str_replace( string& str, const char* substr, const char* new_substr )
{
	if( ! substr || ! new_substr )
		return false;

	bool found = false;
	string result;
	int l = strlen(substr);

	const char* p = str.c_str();
	while( *p )	{
		if( strncmp( p, substr, l ) ) {
			result += *p;
			++p;
		}
		else {
			result += new_substr;
			p += l;
			found = true;
		}
	}
	if( found )
		str = result;
	return found;
}

void strstriptail(char* str)
{
	int len=strlen(str);
	str+=len-1;
	while(*str==' ')
		str--;
	str++;
	*str=0;
	//return str;
}

void str_striptail(string& str)
{
	char* p = new char[ str.size()+1 ];
	memset(p, 0, (str.size()+1)*sizeof(char));
	strcpy( p, str.c_str() );
	strstriptail(p);
	str = p;
	delete [] p;
}

bool IsBig5(const char * str, const char * ppos)
{
	if (ppos == str)
		return ((get_chw(str) == 2));

	int w;
	do
	{
		w = get_chw(str);
		str += w;
	}
	while (str < ppos);
	if (ppos == str)
		w = get_chw(str);
	return ((w == 2) && (ppos == str));
}