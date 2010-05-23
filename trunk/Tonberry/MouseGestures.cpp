//*Mouse Gestures support (FIXME: dirty & poor)*

#include "MouseGestures.h"



CMouseGestures::CMouseGestures( int Sensitivity_x )
{
	_pAction_list_source = new int[8];
	pAction_list = _pAction_list_source;
	m_Sensitivity_x = Sensitivity_x;
	beginAction = false;
}

CMouseGestures::~CMouseGestures()
{
	delete [] _pAction_list_source;
}

int CMouseGestures::DoAction(POINTS& pt)
{
	if ( prev_pt.x == -999) prev_pt.x=pt.x;
	if ( prev_pt.y == -999) prev_pt.y=pt.y;

	int x = pt.x;
	int y = pt.y;
	int oldx = prev_pt.x;
	int oldy = prev_pt.y;
	prev_pt.x = x;
	prev_pt.y = y;

	//const double Theta = 0.57735; // tan(PI/6)
	const double Theta = 1.73205;
	//const double Theta = 0.36397; // tan(PI/8)
	//const double Theta = 2.74747;

	const int Sensitivity = 2; // 2~6
	int Sensitivity_x;

	// there is a dirty hack below
	if ( pAction_list - _pAction_list_source == 0 )
		Sensitivity_x = GetSensitivityX();
	else
		Sensitivity_x = Sensitivity;


	int dy = y - oldy;
	int dx = x - oldx;

	double z;
	if (dx==0) z=999;
	else{
		z = dy/dx;
	}

	if ( dx>0 && dy>0 )		{}
	else if ( dx<0 && dy>0) { z=-z; }
	else if ( dx<0 && dy<0) {}
	else if ( dx>0 && dy<0) { z=-z; }

	if (y > oldy && abs(dy)>Sensitivity && (z > Theta))
		return 0; //down
	else if (y < oldy && abs(dy)>Sensitivity && (z > Theta))
		return 1; //up
	else if ( x > oldx && abs(dx)>Sensitivity_x)
		return 2; //right
	else if ( x < oldx && abs(dx)>Sensitivity_x)
		return 3; //left

	return -1;
}

bool CMouseGestures::NewAction(int action)
{
	if ( pAction_list - _pAction_list_source == 8)
		return false; //full

	*pAction_list = action;
	pAction_list++;
	return true;
}

string CMouseGestures::GetActions()
{
	string str;
	char* buf = new char[8];
	int* ptr = _pAction_list_source;

	for (int i=0; i<8; i++)
	{
		itoa(*ptr, buf, 10);
		str+=buf;
		str+=",";
		ptr++;
	}
	delete [] buf;
	return str;
}

int CMouseGestures::GetActions(int num)
{
	int* ptr = _pAction_list_source;
	if (num<8) // 0~7
		return (*(ptr+num));
	else return -1;
}

void CMouseGestures::Reset()
{
	prev_pt.x = prev_pt.y = -999;
	prev_action = prev_added = -1;
	memset( _pAction_list_source, -1, 8*sizeof(int) );
	pAction_list = _pAction_list_source;
}
