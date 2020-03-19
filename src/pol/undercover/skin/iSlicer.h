#if !defined(__GVZ_SLICER_INTERFACE_INCLUDED__)
#define __GVZ_SLICER_INTERFACE_INCLUDED__

#include <list>

class gPoly;

class gPoint
{
public:
	float	pX, pY;

	gPoint () {pX = pY = 0;}
	gPoint (float _pX, float _pY) {pX = _pX; pY = _pY;}
	gPoint (const gPoint &_) {pX = _.pX; pY = _.pY;}
	gPoint &operator = (const gPoint &_) {pX = _.pX; pY = _.pY; return *this;}
};

typedef std::list<gPoint>						pList;
typedef std::list<gPoint>::iterator				ipList;
typedef std::list<gPoint>::reverse_iterator		rpList;
typedef std::list<gPoint>::reference			rList;

class gLine
{
public:
	gLine () {nX = nY = pX = pY = 0;}
	gLine (float _pX, float _pY, float _nX, float _nY)
	{
		pX = _pX; pY = _pY;
		nX = _nX; nY = _nY;
	}

	float	nX, nY;
	float	pX, pY;
};

class iSlicer
{
public:
	static gPoly *CreatePoly ();		// ������� ��-�
	static void DeletePoly (gPoly *);	// ������� ��-�

	static void AddPoint (gPoly *, gPoint);	// ��������� ������� � ��-�
	static void ClearPoly (gPoly *);		// ������� ��� ������� � ��-��
	static gPoly *SlicePoly (gPoly *, gLine);	// ����� ��-� ������, ���������� ���������� �����
												// � �������� ������������ �������
	static int GetPointCount (gPoly *);		// ���������� ���-�� ������
	static ipList FirstPoint (gPoly *);		// ���������� �������� ��� ������ �������
};

#endif