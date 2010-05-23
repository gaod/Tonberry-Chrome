
#ifndef GdiPlusPlusH
#define GdiPlusPlusH
//#define _CRT_SECURE_NO_DEPRECATE 1
//#define _WIN32_WINNT 0x501
//#define WIN32_LEAN_AND_MEAN 1
//#define UNICODE  1
//#define _UNICODE 1
#include <Windows.h>
#define for if(0);else for


static void CacheInit();
static void CacheTerm();
static BOOL IsOSXPorLater();

static void CacheFillSolidRect(COLORREF rgb, const RECT* lprc);
static BOOL IsValidDC(const HDC hdc);
static void EnhanceEdges(BYTE* lpPixels, int width, int height);
static void  ScaleDIB(BYTE* lpPixels, int width, int height);
static HDC  CreateCacheDC();
static HBITMAP  CreateCacheDIB(HDC hdc, int width, int height, BYTE** lpPixels);

BOOL WINAPI IMPL_TextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, UINT cbString);
BOOL WINAPI IMPL_TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, UINT cbString);
BOOL WINAPI IMPL_ExtTextOutA(HDC hdc, int nXStart, int nYStart, UINT fuOptions, CONST RECT *lprc, LPCSTR lpString, UINT cbString, CONST INT *lpDx);
BOOL WINAPI IMPL_ExtTextOutW(HDC hdc, int nXStart, int nYStart, UINT fuOptions, CONST RECT *lprc, LPCWSTR lpString, UINT cbString, CONST INT *lpDx);

#endif
//EOF
