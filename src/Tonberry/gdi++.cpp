// override.cpp - キ?イなTextOut
// 2006/09/27

#include "gdi++.h"

#if defined(__SSE3__)
#include <pmmintrin.h>
#elif defined(__SSE2__)
#include <emmintrin.h>
#endif
#include <malloc.h>		// _alloca
#include <mbctype.h>	// _getmbcp

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")


//--------------------------------------------
static UINT g_Quality = 1; // 1 means turn ON
static UINT g_Weight = 0;
static UINT g_Enhance = 0; // g_Scale MUST > 2
static UINT g_UseSubPixel = 0;
static UINT g_SubPixelDirection = 0;
static float g_MaxHeight = 0;
static UINT g_ForceAntialiasedQuality = 0;

static BOOL g_IsWinXPorLater = FALSE;

int  g_Scale = 2;
//--------------------------------------------


#define MAX_EXCLUDES 32
struct StringHash {
	DWORD	hash;
	WCHAR	name[LF_FACESIZE];
	
	StringHash() {}
	StringHash(LPCWSTR s)
	{
		Init(s);
	}
	void Init(LPCWSTR s)
	{
		wcsncpy(name, s, LF_FACESIZE);
		_UpdateHash();
	}
	bool operator ==(const StringHash& x) const
	{
		return !(hash != x.hash || wcscmp(name, x.name) != 0);
	}
	void _UpdateHash()
	{
		DWORD dw = 0;
		LPWSTR p;
		for (p = name; *p; p++) {
			dw <<= 3;
			dw ^= *p;
		}
		hash = dw;
	}
} g_ExcludeList[MAX_EXCLUDES];


// ExtTextOutWのビットマップキ?ッシ?
struct {
	HDC	hdc;
	HBITMAP	hbmp;
	BYTE*	lpPixels;
	SIZE	dibSize;
	CRITICAL_SECTION  cs;
} g_Cache = { NULL,NULL,NULL, {0,0},{0} };

/*struct CacheLock {
	CacheLock()  { EnterCriticalSection(&g_Cache.cs); }
	~CacheLock() { LeaveCriticalSection(&g_Cache.cs); }
};*/

void  CacheInit()
{
//	InitializeCriticalSection(&g_Cache.cs);
}

void  CacheTerm()
{
	if(g_Cache.hdc) DeleteDC(g_Cache.hdc);
	if(g_Cache.hbmp) DeleteObject(g_Cache.hbmp);
	g_Cache.hdc = NULL;
	g_Cache.hbmp = NULL;
	g_Cache.lpPixels = NULL;
//	DeleteCriticalSection(&g_Cache.cs);
}

void CacheFillSolidRect(COLORREF rgb, const RECT* lprc)
{
	rgb = RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
	DWORD* p = (DWORD*)g_Cache.lpPixels;
	//ULONG_PTR pend = (ULONG_PTR)p + (g_Cache.dibSize.cx * g_Cache.dibSize.cy);
	unsigned long* pend = (unsigned long*)p + (g_Cache.dibSize.cx * g_Cache.dibSize.cy);
	// lpPixelsは?の値になる?が?るのでunsignedで比較する
	//while((ULONG_PTR)p < pend)
	while((unsigned long*)p < pend)
		*p++ = rgb;
}



// 有効なDCかどうかをチェックする
BOOL IsValidDC(const HDC hdc){
	if (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY) return FALSE;
	// ここでフォ?トチェックも行う
	WCHAR szFaceName[LF_FACESIZE] = L"";
	GetTextFaceW(hdc, LF_FACESIZE, szFaceName);
//	if (IsFontExcluded(szFaceName)) return FALSE;
	return TRUE;
}


const static BYTE saturated_sum[256*2] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
  0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
  0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
  0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
  0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87, 0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
  0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97, 0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
  0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7, 0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
  0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7, 0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
  0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7, 0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
  0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7, 0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
  0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7, 0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
  0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7, 0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};
#define adds(var, val)	((var) = saturated_sum[(var) + (val) + 128])
#define subs(var, val)	((var) = saturated_sum[(var) - (val) + 128])

void EnhanceEdges(BYTE* lpPixels, int width, int height)
{
  	if (g_Enhance <= 0)      return;
        else if (g_Enhance > 4)  g_Enhance = 4;

        int  srcWidthBytes = g_Cache.dibSize.cx * 4;
	int  dstSkipBytes  = srcWidthBytes - (width*4 / g_Scale);
        int  w = (width*4 / g_Scale) + dstSkipBytes;
        register BYTE* dst;

        width  /= g_Scale;
        height /= g_Scale;
        dst = lpPixels + w;
        for(int y=1 ; y < height-1 ; y++) {
            register signed char* hdif = (signed char *)dst + srcWidthBytes * height;
            register signed char* vdif = hdif + srcWidthBytes * height;
            dst+=4; hdif+=4; vdif+=4;
            for (int x = 1 ; x < width-1 ; x++) {
                int ratio = (7 - g_Enhance) * 2;
                if (g_UseSubPixel == 0) {
                    hdif[0] = ((dst[0] << 1) - dst[-4]   - dst[4])   / ratio;
                    hdif[1] = ((dst[1] << 1) - dst[-3]   - dst[5])   / ratio;
                    hdif[2] = ((dst[2] << 1) - dst[-2]   - dst[6])   / ratio;
                }
                vdif[0] = ((dst[0] << 1) - dst[-w]   - dst[w])   / ratio;
                vdif[1] = ((dst[1] << 1) - dst[-w+1] - dst[w+1]) / ratio;
                vdif[2] = ((dst[2] << 1) - dst[-w+2] - dst[w+2]) / ratio;
                dst+=4; hdif+=4; vdif+=4;
            }
            dst += dstSkipBytes + 4;
        }
        dst = lpPixels + w;
        for(int y=1 ; y < height-1 ; y++) {
            register signed char* hdif = (signed char *)dst + srcWidthBytes * height;
            register signed char* vdif = hdif + srcWidthBytes * height;
            dst+=4; hdif+=4; vdif+=4;
            for (int x = 1 ; x < width-1 ; x++) {
                if (g_UseSubPixel == 0) {
                    subs(dst[-4], hdif[0] / 2);  adds(dst[0], hdif[0]);  subs(dst[4],  hdif[0] / 2);
                    subs(dst[-3], hdif[1] / 2);  adds(dst[1], hdif[1]);  subs(dst[5],  hdif[1] / 2);
                    subs(dst[-2], hdif[2] / 2);  adds(dst[2], hdif[2]);  subs(dst[6],  hdif[2] / 2);
                }
                subs(dst[-w],  vdif[0] / 2);   adds(dst[0], vdif[0]);   subs(dst[w],   vdif[0] / 2);
                subs(dst[1-w], vdif[1] / 2);   adds(dst[1], vdif[1]);   subs(dst[w+1], vdif[1] / 2);
                subs(dst[2-w], vdif[2] / 2);   adds(dst[2], vdif[2]);   subs(dst[w+2], vdif[2] / 2);
                dst+=4; hdif+=4; vdif+=4;
            }
            dst += dstSkipBytes + 4;
        }
}

// こんな感じなら コ?パイ?の最適化＋CPUの先読み実行で何とかならんかのぉ？
// LookupTablex3はとり?えず止め。後でベ?チしてみる。
// (2006/09/22) サブピクセ???ダ??グを元に戻してみた
//              ウィ?ドウを重ねたときなどに出る色化けは?いかわらず...
void  ScaleDIB(BYTE* lpPixels, int width, int height)
{
	int  srcWidthBytes = g_Cache.dibSize.cx * 4;
	int  srcSkipBytes  = srcWidthBytes - (width*4);
	int  dstSkipBytes  = srcWidthBytes - (width*4 / g_Scale);
#if defined(__SSE2__) || defined(__SSE3__)
	__m128i PZERO = _mm_setzero_si128();
#endif
	register BYTE* dst  = lpPixels;
	register BYTE* src0 = lpPixels;
	width  /= g_Scale;
	height /= g_Scale;

	if(g_Scale == 4) {
#if defined(__SSE3__)
		for(int y=0 ; y < height ; y++) {
			register BYTE* src1 = src0 + srcWidthBytes;
			register BYTE* src2 = src1 + srcWidthBytes;
			register BYTE* src3 = src2 + srcWidthBytes;
			int x;
			int widtha;
			widtha = width&(~3);
			for(x=0 ; x < widtha ; x+=4) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
				XMM0 = _mm_lddqu_si128((__m128i*)(src0   ));
				XMM1 = _mm_lddqu_si128((__m128i*)(src1   ));
				XMM4 = _mm_lddqu_si128((__m128i*)(src2   ));
				XMM5 = _mm_lddqu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM6 = _mm_lddqu_si128((__m128i*)(src0+16));
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM5 = _mm_lddqu_si128((__m128i*)(src1+16));
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM3 = _mm_lddqu_si128((__m128i*)(src2+16));
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM4 = _mm_lddqu_si128((__m128i*)(src3+16));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = XMM6;
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM5;
				XMM6 = _mm_unpacklo_epi8(XMM6, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_add_epi16(XMM6, XMM5);
				XMM5 = XMM3;
				XMM1 = _mm_add_epi16(XMM1, XMM2);
				XMM2 = XMM4;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM4);
				XMM4 = _mm_lddqu_si128((__m128i*)(src0+32));
				XMM5 = _mm_add_epi16(XMM5, XMM2);
				XMM2 = _mm_lddqu_si128((__m128i*)(src1+32));
				XMM6 = _mm_add_epi16(XMM6, XMM3);
				XMM3 = _mm_lddqu_si128((__m128i*)(src2+32));
				XMM1 = _mm_add_epi16(XMM1, XMM5);
				XMM5 = _mm_lddqu_si128((__m128i*)(src3+32));
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM0;
				XMM0 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM0), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(3,2,1,0)));
				XMM1 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM1), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(1,0,3,2)));
				XMM6 = XMM4;
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM2;
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM2 = _mm_unpacklo_epi8(XMM2, PZERO);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				_mm_storel_epi64((__m128i*)(dst   ), XMM0);
				XMM0 = _mm_lddqu_si128((__m128i*)(src0+48));
				XMM4 = _mm_add_epi16(XMM4, XMM2);
				XMM2 = XMM3;
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM5;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM5);
				XMM5 = _mm_lddqu_si128((__m128i*)(src1+48));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = _mm_lddqu_si128((__m128i*)(src2+48));
				XMM4 = _mm_add_epi16(XMM4, XMM3);
				XMM3 = _mm_lddqu_si128((__m128i*)(src3+48));
				XMM6 = _mm_add_epi16(XMM6, XMM2);
				XMM2 = XMM0;
				XMM4 = _mm_add_epi16(XMM4, XMM6);
				XMM6 = XMM5;
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM5);
				XMM5 = XMM1;
				XMM2 = _mm_add_epi16(XMM2, XMM6);
				XMM6 = XMM3;
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM5 = _mm_add_epi16(XMM5, XMM6);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM2 = _mm_add_epi16(XMM2, XMM5);
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM4;
				XMM4 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM4), _mm_castsi128_ps(XMM0), _MM_SHUFFLE(3,2,1,0)));
				XMM2 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM2), _mm_castsi128_ps(XMM0), _MM_SHUFFLE(1,0,3,2)));
				XMM4 = _mm_add_epi16(XMM4, XMM2);
				XMM4 = _mm_srli_epi16(XMM4, 4);
				XMM4 = _mm_packus_epi16(XMM4, XMM4);
				_mm_storel_epi64((__m128i*)(dst+ 8), XMM4);
				dst+=16; src0+=4*16; src1+=4*16; src2+=4*16; src3+=4*16;
			}
			widtha = width&(~1);
			for(    ; x < widtha ; x+=2) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
				XMM0 = _mm_lddqu_si128((__m128i*)(src0   ));
				XMM1 = _mm_lddqu_si128((__m128i*)(src1   ));
				XMM4 = _mm_lddqu_si128((__m128i*)(src2   ));
				XMM5 = _mm_lddqu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM6 = _mm_lddqu_si128((__m128i*)(src0+16));
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM5 = _mm_lddqu_si128((__m128i*)(src1+16));
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM3 = _mm_lddqu_si128((__m128i*)(src2+16));
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM4 = _mm_lddqu_si128((__m128i*)(src3+16));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = XMM6;
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM5;
				XMM6 = _mm_unpacklo_epi8(XMM6, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_add_epi16(XMM6, XMM5);
				XMM5 = XMM3;
				XMM1 = _mm_add_epi16(XMM1, XMM2);
				XMM2 = XMM4;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM4);
				XMM5 = _mm_add_epi16(XMM5, XMM2);
				XMM6 = _mm_add_epi16(XMM6, XMM3);
				XMM1 = _mm_add_epi16(XMM1, XMM5);
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM0;
				XMM0 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM0), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(3,2,1,0)));
				XMM1 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM1), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(1,0,3,2)));
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				_mm_storel_epi64((__m128i*)(dst   ), XMM0);
				dst+=8; src0+=4*8; src1+=4*8; src2+=4*8; src3+=4*8;
			}
			for(    ; x < width ; x++) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
				XMM0 = _mm_lddqu_si128((__m128i*)(src0   ));
				XMM1 = _mm_lddqu_si128((__m128i*)(src1   ));
				XMM4 = _mm_lddqu_si128((__m128i*)(src2   ));
				XMM5 = _mm_lddqu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(XMM2), _mm_castsi128_ps(XMM0)));
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				_mm_store_ss((float*)(dst   ), _mm_castsi128_ps(XMM0));
				dst+=4; src0+=4*4; src1+=4*4; src2+=4*4; src3+=4*4;
			}
			dst += dstSkipBytes;
			src0 = src3 + srcSkipBytes;
		}
#elif defined(__SSE2__)
		for(int y=0 ; y < height ; y++) {
			register BYTE* src1 = src0 + srcWidthBytes;
			register BYTE* src2 = src1 + srcWidthBytes;
			register BYTE* src3 = src2 + srcWidthBytes;
			int x;
			int widtha;
			widtha = width&(~3);
			for(x=0 ; x < widtha ; x+=4) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
				XMM0 = _mm_loadu_si128((__m128i*)(src0   ));
				XMM1 = _mm_loadu_si128((__m128i*)(src1   ));
				XMM4 = _mm_loadu_si128((__m128i*)(src2   ));
				XMM5 = _mm_loadu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM6 = _mm_loadu_si128((__m128i*)(src0+16));
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM5 = _mm_loadu_si128((__m128i*)(src1+16));
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM3 = _mm_loadu_si128((__m128i*)(src2+16));
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM4 = _mm_loadu_si128((__m128i*)(src3+16));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = XMM6;
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM5;
				XMM6 = _mm_unpacklo_epi8(XMM6, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_add_epi16(XMM6, XMM5);
				XMM5 = XMM3;
				XMM1 = _mm_add_epi16(XMM1, XMM2);
				XMM2 = XMM4;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM4);
				XMM4 = _mm_loadu_si128((__m128i*)(src0+32));
				XMM5 = _mm_add_epi16(XMM5, XMM2);
				XMM2 = _mm_loadu_si128((__m128i*)(src1+32));
				XMM6 = _mm_add_epi16(XMM6, XMM3);
				XMM3 = _mm_loadu_si128((__m128i*)(src2+32));
				XMM1 = _mm_add_epi16(XMM1, XMM5);
				XMM5 = _mm_loadu_si128((__m128i*)(src3+32));
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM0;
				XMM0 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM0), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(3,2,1,0)));
				XMM1 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM1), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(1,0,3,2)));
				XMM6 = XMM4;
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM2;
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM2 = _mm_unpacklo_epi8(XMM2, PZERO);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				_mm_storel_epi64((__m128i*)(dst   ), XMM0);
				XMM0 = _mm_loadu_si128((__m128i*)(src0+48));
				XMM4 = _mm_add_epi16(XMM4, XMM2);
				XMM2 = XMM3;
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM5;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM5);
				XMM5 = _mm_loadu_si128((__m128i*)(src1+48));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = _mm_loadu_si128((__m128i*)(src2+48));
				XMM4 = _mm_add_epi16(XMM4, XMM3);
				XMM3 = _mm_loadu_si128((__m128i*)(src3+48));
				XMM6 = _mm_add_epi16(XMM6, XMM2);
				XMM2 = XMM0;
				XMM4 = _mm_add_epi16(XMM4, XMM6);
				XMM6 = XMM5;
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM5);
				XMM5 = XMM1;
				XMM2 = _mm_add_epi16(XMM2, XMM6);
				XMM6 = XMM3;
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM6 = _mm_unpackhi_epi8(XMM6, PZERO);
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM5 = _mm_add_epi16(XMM5, XMM6);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM2 = _mm_add_epi16(XMM2, XMM5);
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM4;
				XMM4 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM4), _mm_castsi128_ps(XMM0), _MM_SHUFFLE(3,2,1,0)));
				XMM2 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM2), _mm_castsi128_ps(XMM0), _MM_SHUFFLE(1,0,3,2)));
				XMM4 = _mm_add_epi16(XMM4, XMM2);
				XMM4 = _mm_srli_epi16(XMM4, 4);
				XMM4 = _mm_packus_epi16(XMM4, XMM4);
				_mm_storel_epi64((__m128i*)(dst+ 8), XMM4);
				dst+=16; src0+=4*16; src1+=4*16; src2+=4*16; src3+=4*16;
			}
			widtha = width&(~1);
			for(    ; x < widtha ; x+=2) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
				XMM0 = _mm_loadu_si128((__m128i*)(src0   ));
				XMM1 = _mm_loadu_si128((__m128i*)(src1   ));
				XMM4 = _mm_loadu_si128((__m128i*)(src2   ));
				XMM5 = _mm_loadu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM6 = _mm_loadu_si128((__m128i*)(src0+16));
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM5 = _mm_loadu_si128((__m128i*)(src1+16));
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM3 = _mm_loadu_si128((__m128i*)(src2+16));
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM4 = _mm_loadu_si128((__m128i*)(src3+16));
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM1 = XMM6;
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = XMM5;
				XMM6 = _mm_unpacklo_epi8(XMM6, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM6 = _mm_add_epi16(XMM6, XMM5);
				XMM5 = XMM3;
				XMM1 = _mm_add_epi16(XMM1, XMM2);
				XMM2 = XMM4;
				XMM3 = _mm_unpacklo_epi8(XMM3, PZERO);
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpackhi_epi8(XMM5, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_add_epi16(XMM3, XMM4);
				XMM5 = _mm_add_epi16(XMM5, XMM2);
				XMM6 = _mm_add_epi16(XMM6, XMM3);
				XMM1 = _mm_add_epi16(XMM1, XMM5);
				XMM6 = _mm_add_epi16(XMM6, XMM1);
				XMM1 = XMM0;
				XMM0 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM0), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(3,2,1,0)));
				XMM1 = _mm_castps_si128(_mm_shuffle_ps(
					_mm_castsi128_ps(XMM1), _mm_castsi128_ps(XMM6), _MM_SHUFFLE(1,0,3,2)));
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				_mm_storel_epi64((__m128i*)(dst   ), XMM0);
				dst+=8; src0+=4*8; src1+=4*8; src2+=4*8; src3+=4*8;
			}
			for(    ; x < width ; x++) {
				__m128i XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
				XMM0 = _mm_loadu_si128((__m128i*)(src0   ));
				XMM1 = _mm_loadu_si128((__m128i*)(src1   ));
				XMM4 = _mm_loadu_si128((__m128i*)(src2   ));
				XMM5 = _mm_loadu_si128((__m128i*)(src3   ));
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM0 = _mm_unpacklo_epi8(XMM0, PZERO);
				XMM1 = _mm_unpacklo_epi8(XMM1, PZERO);
				XMM2 = _mm_unpackhi_epi8(XMM2, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM0 = _mm_add_epi16(XMM0, XMM1);
				XMM1 = XMM4;
				XMM2 = _mm_add_epi16(XMM2, XMM3);
				XMM3 = XMM5;
				XMM4 = _mm_unpacklo_epi8(XMM4, PZERO);
				XMM5 = _mm_unpacklo_epi8(XMM5, PZERO);
				XMM1 = _mm_unpackhi_epi8(XMM1, PZERO);
				XMM3 = _mm_unpackhi_epi8(XMM3, PZERO);
				XMM4 = _mm_add_epi16(XMM4, XMM5);
				XMM1 = _mm_add_epi16(XMM1, XMM3);
				XMM0 = _mm_add_epi16(XMM0, XMM4);
				XMM2 = _mm_add_epi16(XMM2, XMM1);
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM2 = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(XMM2), _mm_castsi128_ps(XMM0)));
				XMM0 = _mm_add_epi16(XMM0, XMM2);
				XMM0 = _mm_srli_epi16(XMM0, 4);
				XMM0 = _mm_packus_epi16(XMM0, XMM0);
				_mm_store_ss((float*)(dst   ), _mm_castsi128_ps(XMM0));
				dst+=4; src0+=4*4; src1+=4*4; src2+=4*4; src3+=4*4;
			}
			dst += dstSkipBytes;
			src0 = src3 + srcSkipBytes;
		}
#else

		for(int y=0 ; y < height ; y++) {
			register BYTE* src1 = src0 + srcWidthBytes;
			register BYTE* src2 = src1 + srcWidthBytes;
			register BYTE* src3 = src2 + srcWidthBytes;
			for(int x=0 ; x < width ; x++) {
				dst[0]=(src0[0] + src0[4] + src0[8] + src0[12] +
					src1[0] + src1[4] + src1[8] + src1[12] +
					src2[0] + src2[4] + src2[8] + src2[12] +
					src3[0] + src3[4] + src3[8] + src3[12]) >> 4;
				dst[1]=(src0[1] + src0[5] + src0[9] + src0[13] +
					src1[1] + src1[5] + src1[9] + src1[13] +
					src2[1] + src2[5] + src2[9] + src2[13] +
					src3[1] + src3[5] + src3[9] + src3[13]) >> 4;
				dst[2]=(src0[2] + src0[6] + src0[10] + src0[14] +
					src1[2] + src1[6] + src1[10] + src1[14] +
					src2[2] + src2[6] + src2[10] + src2[14] +
					src3[2] + src3[6] + src3[10] + src3[14]) >> 4;
				dst+=4; src0+=4*4; src1+=4*4; src2+=4*4; src3+=4*4;
			}
			dst += dstSkipBytes;
			src0 = src3 + srcSkipBytes;
		}
#endif
	}
	else if(g_Scale == 3) {
		if (g_UseSubPixel==0) {
			for(int y=0 ; y < height ; y++) {
				// Don't use subpixels
				register BYTE* src1 = src0 + srcWidthBytes;
				register BYTE* src2 = src1 + srcWidthBytes;
				for(int x=0 ; x < width; x++) {
					dst[0]=(src0[0] + src0[4] + src0[8] +
						src1[0] + src1[4] + src1[8] +
						src2[0] + src2[4] + src2[8]) / 9;
					dst[1]=(src0[1] + src0[5] + src0[9] +
						src1[1] + src1[5] + src1[9] +
						src2[1] + src2[5] + src2[9]) / 9;
					dst[2]=(src0[2] + src0[6] + src0[10] +
						src1[2] + src1[6] + src1[10] +
						src2[2] + src2[6] + src2[10]) / 9;
					dst+=4; src0+=4*3; src1+=4*3; src2+=4*3;
				}
				dst += dstSkipBytes;
				src0 = src2 + srcSkipBytes;
			}
		} else {
			if (g_SubPixelDirection==0) {
				for(int y=0 ; y < height ; y++) {
					// RGB
					register BYTE* src1 = src0 + srcWidthBytes;
					register BYTE* src2 = src1 + srcWidthBytes;					
					dst[0]=(src0[0] + src0[4] + src0[8] +
						src1[0] + src1[4] + src1[8] +
						src2[0] + src2[4] + src2[8]) / 9;
					dst[1]=(src0[1] + src0[5] + src0[9] +
						src1[1] + src1[5] + src1[9] +
						src2[1] + src2[5] + src2[9]) / 9;
					dst[2]=(src0[2] + src0[6] + src0[10] +
						src1[2] + src1[6] + src1[10] +
						src2[2] + src2[6] + src2[10]) / 9;
					dst+=4;src0+=4*3; src1+=4*3; src2+=4*3;
					for(int x=1 ; x < width-1; x++) {
						dst[0]=(src0[8] + src0[12] + src0[16] +
							src1[8] + src1[12] + src1[16] +
							src2[8] + src2[12] + src2[16]) / 9;
						dst[1]=(src0[5] + src0[9] + src0[13] +
							src1[5] + src1[9] + src1[13] +
							src2[5] + src2[9] + src2[13]) / 9;
						dst[2]=(src0[2] + src0[6] + src0[10] +
							src1[2] + src1[6] + src1[10] +
							src2[2] + src2[6] + src2[10]) / 9;
						dst+=4; src0+=4*3; src1+=4*3; src2+=4*3;
					}
					dst[0]=(src0[0] + src0[4] + src0[8] +
						src1[0] + src1[4] + src1[8] +
						src2[0] + src2[4] + src2[8]) / 9;
					dst[1]=(src0[1] + src0[5] + src0[9] +
						src1[1] + src1[5] + src1[9] +
						src2[1] + src2[5] + src2[9]) / 9;
					dst[2]=(src0[2] + src0[6] + src0[10] +
						src1[2] + src1[6] + src1[10] +
						src2[2] + src2[6] + src2[10]) / 9;
					//dst+=4; src0+=4*3; src1+=4*3; src2+=4*3;
					dst += dstSkipBytes + 4;
					src0 = src2 + (4*3) + srcSkipBytes;
				}
			} else {
				for(int y=0 ; y < height ; y++) {
					// BGR
					register BYTE* src1 = src0 + srcWidthBytes;
					register BYTE* src2 = src1 + srcWidthBytes;					
					dst[0]=(src0[0] + src0[4] + src0[8] +
						src1[0] + src1[4] + src1[8] +
						src2[0] + src2[4] + src2[8]) / 9;
					dst[1]=(src0[1] + src0[5] + src0[9] +
						src1[1] + src1[5] + src1[9] +
						src2[1] + src2[5] + src2[9]) / 9;
					dst[2]=(src0[2] + src0[6] + src0[10] +
						src1[2] + src1[6] + src1[10] +
						src2[2] + src2[6] + src2[10]) / 9;
					dst+=4;src0+=4*3; src1+=4*3; src2+=4*3;
					for(int x=1 ; x < width-1; x++) {
						dst[0]=(src0[0] + src0[4] + src0[8] +
							src1[0] + src1[4] + src1[8] +
							src2[0] + src2[4] + src2[8]) / 9;
						dst[1]=(src0[5] + src0[9] + src0[13] +
							src1[5] + src1[9] + src1[13] +
							src2[5] + src2[9] + src2[13]) / 9;
						dst[2]=(src0[10] + src0[14] + src0[18] +
							src1[10] + src1[14] + src1[18] +
							src2[10] + src2[14] + src2[18]) / 9;
						dst+=4; src0+=4*3; src1+=4*3; src2+=4*3;
					}
					dst[0]=(src0[0] + src0[4] + src0[8] +
						src1[0] + src1[4] + src1[8] +
						src2[0] + src2[4] + src2[8]) / 9;
					dst[1]=(src0[1] + src0[5] + src0[9] +
						src1[1] + src1[5] + src1[9] +
						src2[1] + src2[5] + src2[9]) / 9;
					dst[2]=(src0[2] + src0[6] + src0[10] +
						src1[2] + src1[6] + src1[10] +
						src2[2] + src2[6] + src2[10]) / 9;
					//dst+=4; src0+=4*3; src1+=4*3; src2+=4*3;
					dst += dstSkipBytes + 4;
					src0 = src2 + (4*3) + srcSkipBytes;
				}
			}
		}
	}
	else if(g_Scale == 2) {
		for(int y=0 ; y < height ; y++) {
			register BYTE* src1 = src0 + srcWidthBytes;
			for(int x=0 ; x < width ; x++) {
				dst[0]=(src0[0] + src0[4] +
					src1[0] + src1[4]) >> 2;
				dst[1]=(src0[1] + src0[5] +
					src1[1] + src1[5]) >> 2;
				dst[2]=(src0[2] + src0[6] +
					src1[2] + src1[6]) >> 2;
				dst+=4; src0+=4*2; src1+=4*2;
			}
			dst += dstSkipBytes;
			src0 = src1 + srcSkipBytes;
		}
	}
}


HDC  CreateCacheDC()
{
	if(!g_Cache.hdc)
		g_Cache.hdc = CreateCompatibleDC(NULL);
	return g_Cache.hdc;
}

HBITMAP  CreateCacheDIB(HDC hdc, int width, int height, BYTE** lpPixels)
{
	if(g_Cache.dibSize.cx < width || g_Cache.dibSize.cy < height) {
		if(g_Cache.hbmp)
			DeleteObject(g_Cache.hbmp);

		if(g_Cache.dibSize.cx < width)
			g_Cache.dibSize.cx = width;
		if(g_Cache.dibSize.cy < height)
			g_Cache.dibSize.cy = height;

		BITMAPINFOHEADER bmiHeader;
		bmiHeader.biSize = sizeof(bmiHeader);
		bmiHeader.biWidth = g_Cache.dibSize.cx;
		bmiHeader.biHeight = -g_Cache.dibSize.cy;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 32;
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = 0;
		bmiHeader.biXPelsPerMeter = 0;
		bmiHeader.biYPelsPerMeter = 0;
		bmiHeader.biClrUsed = 0;
		bmiHeader.biClrImportant = 0;
		g_Cache.hbmp = CreateDIBSection(hdc, (CONST BITMAPINFO*)&bmiHeader, DIB_RGB_COLORS, (LPVOID*)&g_Cache.lpPixels, NULL, 0);
	}
	*lpPixels = g_Cache.lpPixels;
	return g_Cache.hbmp;
}

static int div_ceil(int a, int b)
{
	if(a % b)
		return (a>0)? (a/b+1): (a/b-1);
	return a / b;
}

// 引?まで?き換えると起動?にフックを取り?いたときにClearTypeが無効になる罠

// Windows 2000用
BOOL WINAPI IMPL_TextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, UINT cbString)
{
	if (g_IsWinXPorLater) {
		return ::TextOutA(hdc, nXStart, nYStart, lpString, cbString);
	} else {
		return IMPL_ExtTextOutA(hdc, nXStart, nYStart, NULL, NULL, lpString, cbString, NULL);
	}
}

BOOL WINAPI IMPL_TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, UINT cbString)
{
	if (g_IsWinXPorLater) {
		return ::TextOutW(hdc, nXStart, nYStart, lpString, cbString);
	} else {
		return IMPL_ExtTextOutW(hdc, nXStart, nYStart, NULL, NULL, lpString, cbString, NULL);
	}
}

void ConvertDxArray(UINT CodePage, LPCSTR lpString, const int* lpDxA, int cbString, int* lpDxW, int)
{
	LPCSTR lpEnd = lpString + cbString;
	while(lpString < lpEnd) {
		*lpDxW = *lpDxA++;
		if(IsDBCSLeadByteEx(CodePage, *lpString)) {
			*lpDxW += *lpDxA++;
			lpString++;
		}
		lpDxW++;
		lpString++;
	}
}

// ANSI->Unicodeに変換してExtTextOutWに?げるExtTextOutA（Windows 2000用）
BOOL WINAPI IMPL_ExtTextOutA(HDC hdc, int nXStart, int nYStart, UINT fuOptions, CONST RECT *lprc, LPCSTR lpString, UINT cbString, CONST INT *lpDx)
{
	if (g_IsWinXPorLater) {
		return ::ExtTextOutA(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbString, lpDx);
	} else {
		LPWSTR lpszUnicode;
		int bufferLength;
		BOOL result;
		bufferLength = MultiByteToWideChar(CP_ACP, 0, lpString, cbString, 0, 0);
	//	lpszUnicode = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferLength * 2);
		lpszUnicode = (LPWSTR)malloc(bufferLength * 2);
		MultiByteToWideChar(CP_ACP, 0, lpString, cbString, lpszUnicode, bufferLength);

		UINT cp = _getmbcp();
		int* lpDxW = NULL;
		if(lpDx && cbString && cp) {
			lpDxW = (int*)calloc(sizeof(int), cbString);
			ConvertDxArray(cp, lpString, lpDx, cbString, lpDxW, 0);
			lpDx = lpDxW;
		}

		result = IMPL_ExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, (LPCWSTR)lpszUnicode, bufferLength, lpDx);
	//	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpszUnicode);
		free(lpszUnicode);
		free(lpDxW);	// free(NULL)は問題無い?が保証されていたはず
		return result;
	}
}

// キ?イなExtTextOut（?社比）の実?
BOOL WINAPI IMPL_ExtTextOutW(HDC hdc, int nXStart, int nYStart, UINT fuOptions, CONST RECT *lprc, LPCWSTR lpString, UINT cbString, CONST INT *lpDx)
{
//	CacheLock  __lock;
	int   error = 0;

	POINT curPos = { nXStart, nYStart };
	POINT destPos;
	SIZE  destSize;
	POINT canvasPos;
	SIZE  canvasSize;

	HDC     hCanvasDC = NULL;
	HGDIOBJ hPrevFont = NULL;
	HBITMAP hBmp = NULL;
	BYTE*   lpPixels = NULL;

	XFORM  xformNoTrns;
	XFORM  xformScale = {1.0f,0.0f,  0.0f,1.0f,  0.0f,0.0f};
	UINT   align;
	SIZE   textSize;

	if(lpString == NULL || cbString == 0 || g_Quality==0) {
		error = 11;//throw
	}
	if(!error) {
		if(!IsValidDC(hdc)) 
			error = 14;	// hdc is invalid
	}
	if(!error) {
		hCanvasDC = CreateCacheDC();
		if(!hCanvasDC)
			error = 1;
	}
	// get cursor / xform
	if(!error) {
		align = GetTextAlign(hdc);
		if(align & TA_UPDATECP) {
			GetCurrentPositionEx(hdc, &curPos);
		}

		GetWorldTransform(hdc, &xformNoTrns);
		if(xformNoTrns.eM12 < -0.1f || 0.1f < xformNoTrns.eM12 ||
		   xformNoTrns.eM21 < -0.1f || 0.1f < xformNoTrns.eM21) {
			error = 12;// throw rotation
		}
		xformNoTrns.eDx = 0.0f; //clear translation
		xformNoTrns.eDy = 0.0f;
		xformScale.eM11 = xformNoTrns.eM11 * g_Scale;//scaling
		xformScale.eM22 = xformNoTrns.eM22 * g_Scale;
		SetGraphicsMode(hCanvasDC, GM_ADVANCED);
		SetWorldTransform(hCanvasDC, &xformScale);
	}
	//copy font
	if(!error) {
		// こっちの方が速い？
		hPrevFont = SelectObject(hCanvasDC, GetCurrentObject(hdc, OBJ_FONT));
/*		HGDIOBJ src = SelectObject(hdc, GetStockObject(SYSTEM_FONT));
		if(!src)
			error = 2;
		else {
			SelectObject(hdc, src);
			hPrevFont = SelectObject(hCanvasDC, src);
		}*/
	}

	//GetTextExtentPoint系はSetTextCharacterExtraを必要とするらしい
	SetTextCharacterExtra(hCanvasDC, GetTextCharacterExtra(hdc));
	//text size
	if(!error) {
		if(fuOptions & ETO_GLYPH_INDEX)
			//ORIG_GetTextExtentPointI(hCanvasDC, (LPWORD)lpString, (int)cbString, &textSize);
			GetTextExtentPointI(hCanvasDC, (LPWORD)lpString, (int)cbString, &textSize);
		else
			//ORIG_GetTextExtentPointW(hCanvasDC, lpString, (int)cbString, &textSize);
//			GetTextExtentPointW(hCanvasDC, lpString, (int)cbString, &textSize);

			//GetTextExtentPointWはTeraPad on win2kで正しく描画されない
			//→ビットマップフォ?トのTransformに対?してない？
			GetTextExtentExPointW(hCanvasDC, lpString, (int)cbString, 0, NULL, NULL, &textSize);
			//ORIG_GetTextExtentPoint32W(hCanvasDC, lpString, (int)cbString, &textSize);
		if(lpDx) {
			textSize.cx = 0;
			if(fuOptions & ETO_PDY)
				for(UINT i=0; i < cbString; textSize.cx += lpDx[i],i+=2);
			else
				for(UINT i=0; i < cbString; textSize.cx += lpDx[i++]);
		}

		// イタ?ック?のス??ト?を足す(構造体の名前がABC･･?
		// 参考: http://bugzilla.mozilla.gr.jp/show_bug.cgi?id=3253
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);		
		if (tm.tmItalic) {
			ABC abcWidth={0, 0, 0};
			GetCharABCWidthsW(hdc, lpString[cbString-1], lpString[cbString-1], &abcWidth);
			textSize.cx += tm.tmOverhang - abcWidth.abcC;
		}

		//if(textSize.cy * xformNoTrns.eM22 >= 24.0f) //max 24pixel
		if (g_MaxHeight > 0) {
			if(textSize.cy * xformNoTrns.eM22 >= g_MaxHeight) //max 26pixel
				error = 13;//throw large size
		}
	}

	if(!error) {
		// 縦?き?の??
		// まだ実?してないので、暫定的に使えるようにエ?ーを?げる
		LOGFONT lf;
		GetObject(GetCurrentObject(hdc, OBJ_FONT),sizeof(LOGFONT), &lf);
		// if ((lf.lfEscapement % 90) != 0)
		if (lf.lfEscapement != 0)
			error = 15;// rotated font
	}

	//rectangle
	if(!error) {
		//trace(L"OrigTextSize=%d %d\n", textSize.cx, textSize.cy);
		//trace(L"OrigCursor=%d %d\n", curPos.x, curPos.y);

		RECT rc;
		UINT horiz = align & (TA_LEFT|TA_RIGHT|TA_CENTER);
		UINT vert  = align & (TA_BASELINE|TA_TOP);//TA_BOTTOM
		if(horiz == TA_CENTER) {
			rc.left  = curPos.x - div_ceil(textSize.cx, 2);
			rc.right = curPos.x + div_ceil(textSize.cx, 2);
			//no move
		}
		else if(horiz == TA_RIGHT) {
			rc.left  = curPos.x - textSize.cx;
			rc.right = curPos.x;
			curPos.x -= textSize.cx;//move pos
		}
		else {
			rc.left  = curPos.x;
			rc.right = curPos.x + textSize.cx;
			curPos.x += textSize.cx;//move pos
		}
		if(vert == TA_BASELINE) {
			TEXTMETRIC metric = {0};
			GetTextMetrics(hCanvasDC, &metric);
			rc.top = curPos.y - metric.tmAscent;
			rc.bottom = curPos.y + metric.tmDescent;
			//trace(L"ascent=%d descent=%d\n", metric.tmAscent, metric.tmDescent);
		}
		else {
			rc.top = curPos.y;
			rc.bottom = curPos.y + textSize.cy;
		}

		canvasPos.x = canvasPos.y = 0;

		//clipping
		if(lprc && (fuOptions & ETO_CLIPPED)) {
			if(rc.left < lprc->left) {
				canvasPos.x -= div_ceil( (int)((lprc->left - rc.left) * xformScale.eM11), g_Scale);
				rc.left = lprc->left;
			}
			if(rc.right > lprc->right) {
				if(horiz == TA_RIGHT)
					canvasPos.x -= div_ceil( (int)((rc.right - lprc->right) * xformScale.eM11), g_Scale);
				rc.right = lprc->right;
			}
			if(rc.top < lprc->top) {
				canvasPos.y -= div_ceil( (int)((lprc->top - rc.top) * xformScale.eM22), g_Scale);
				rc.top = lprc->top;
			}
			if(rc.bottom > lprc->bottom) {
				rc.bottom = lprc->bottom;
			}
			//trace(L"ClipRect=%d %d %d %d\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
		}

		destPos.x = rc.left;
		destPos.y = rc.top;
		destSize.cx = rc.right - rc.left;
		destSize.cy = rc.bottom - rc.top;
		canvasSize.cx = (int)(destSize.cx * xformScale.eM11);
		canvasSize.cy = (int)(destSize.cy * xformScale.eM22);

		//trace(L"MovedCursor=%d %d\n", curPos.x, curPos.y);
		//trace(L"TargetRect=%d %d %d %d\n", rc.left, rc.top, rc.right, rc.bottom);
		//trace(L"DestPos=%dx%d Size=%dx%d\n", destPos.x, destPos.y, destSize.cx, destSize.cy);
		//trace(L"CanvasPos=%dx%d Size=%dx%d\n", canvasPos.x, canvasPos.y, canvasSize.cx, canvasSize.cy);

		if(destSize.cx < 1 || destSize.cy < 1)
			error = 14; //throw no area
	}
	//bitmap
	if(!error) {
		hBmp = CreateCacheDIB(hCanvasDC, canvasSize.cx, canvasSize.cy, &lpPixels);
		if(!hBmp)
			error = 3;
	}
	if(!error) {
		HGDIOBJ hPrevBmp = SelectObject(hCanvasDC, hBmp);

		BOOL fillrect = (lprc && (fuOptions & ETO_OPAQUE)) ? TRUE : FALSE;

		//clear bitmap
		if(fillrect || GetBkMode(hdc) == OPAQUE) {
			SetBkMode(hCanvasDC, OPAQUE);
			COLORREF  bgcolor = GetBkColor(hdc); //両方とも同じ背景色に
			SetBkColor(hCanvasDC, bgcolor);

			//Brush作るより?速らしい(MS情報)
//			RECT rc = { 0,0, destSize.cx, destSize.cy };//WorldTransformが効いてるからdestSizeで良い
//			ORIG_ExtTextOutW(hCanvasDC, 0,0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			//DIB直接?作 (?まり変わってないかも・・・)
			//SelectObject(hCanvasDC, hPrevBmp);
			RECT rc = { 0, 0, canvasSize.cx, canvasSize.cy };
			CacheFillSolidRect(bgcolor, &rc);
			//SelectObject(hCanvasDC, hBmp);

			if(fillrect) {
				::ExtTextOutW(hdc, 0,0, ETO_OPAQUE, lprc, NULL, 0, NULL);
			}
		}
		else {
			SetBkMode(hCanvasDC, TRANSPARENT);
			BitBlt(hCanvasDC,0,0,destSize.cx,destSize.cy, hdc,destPos.x,destPos.y, SRCCOPY);//WorldTransformが効いてるからdestSizeで良い
		}

		//setup
		SetTextColor(hCanvasDC, GetTextColor(hdc));
		SetTextAlign(hCanvasDC, TA_LEFT | TA_TOP);

		//textout
		for (UINT w=0; w<=g_Weight; w++) ::ExtTextOutW(hCanvasDC, canvasPos.x, canvasPos.y, fuOptions, NULL, lpString, cbString, lpDx);

		//SaveDIB(L"a0.bmp", canvasSize.cx, canvasSize.cy, lpPixels);

		//scale
		ScaleDIB(lpPixels, canvasSize.cx, canvasSize.cy);
		EnhanceEdges(lpPixels, canvasSize.cx, canvasSize.cy);

		//SaveDIB(L"a1.bmp", canvasSize.cx, canvasSize.cy, lpPixels);

		//blt
		SetWorldTransform(hCanvasDC, &xformNoTrns);
		BitBlt(hdc, destPos.x, destPos.y, destSize.cx, destSize.cy, hCanvasDC, 0,0, SRCCOPY);

		SelectObject(hCanvasDC, hPrevBmp);

		if(align & TA_UPDATECP) {
			MoveToEx(hdc, curPos.x, curPos.y, NULL);
		}
	}

	//if(hBmp) DeleteObject(hBmp);
	if(hPrevFont) SelectObject(hCanvasDC, hPrevFont);
	//if(hCanvasDC) DeleteDC(hCanvasDC);

	if(!error)
		return TRUE;
	return ::ExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbString, lpDx);
}

//EOF
