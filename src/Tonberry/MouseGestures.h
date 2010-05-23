#ifndef	__MOUSEGESTURES_H__
#define	__MOUSEGESTURES_H__



#include <windows.h>
#include <string>

using namespace std;

class CMouseGestures
{

public:
	CMouseGestures( int Sensitivity_x = 0 );
	~CMouseGestures();

	int DoAction(POINTS& pt);
	bool NewAction(int action);
	string GetActions();
	int GetActions(int num);
	void Reset();

	// getter & setter
	int GetSensitivityX() const { return m_Sensitivity_x; }
	void SetSensitivityX(int s) { m_Sensitivity_x  = s; }

	bool beginAction;
	int prev_action;
	int prev_added;

protected:
	int* pAction_list;
	int* _pAction_list_source;
	POINTS prev_pt;

	int m_Sensitivity_x;
};


#endif