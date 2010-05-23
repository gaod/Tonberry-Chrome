#ifndef HelperWndH
#define HelperWndH


enum {ID_HELPER_COPY=9000, ID_HELPER_SEARCH=9001,
	ID_HELPER_CLOSE=9002, ID_HELPER_SURL_0RZ=9003,
	ID_HELPER_SURL_TINYURL=9004, ID_HELPER_COPYPASTE=9005,
	ID_HELPER_COPYANSI=9006, ID_HELPER_SELECTALL=9008	
};

static inline void SuperSleep(double sec)
{
	LARGE_INTEGER li;
	LONGLONG QPart1,QPart2;
	double minus, freq, tim;

	QueryPerformanceFrequency(&li);
	freq = (double)li.QuadPart;
	QueryPerformanceCounter(&li);
	QPart1 = li.QuadPart;

	do
	{
		QueryPerformanceCounter(&li);
		QPart2 = li.QuadPart;
		minus = (double)(QPart2-QPart1);
		tim = minus / freq;
//		::Sleep(0);

	}while( tim < sec ); 
}



class CHelperWnd
{
	public:
		CHelperWnd(HWND parent);
		~CHelperWnd();
		void Destroy();


		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		HWND m_hWnd;
};


#endif