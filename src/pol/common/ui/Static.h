/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
	   Static - ��� ������� ������� ����� ��������� ���������.
	��������� ��������������� ����������. ��������� ����� ����
	��������� �������: ������������ ������� �� ����
	   - LBUTTONDOWN	- ������ ���� ��������
	   - LBUTTONUP		- ������ ���� ��������
	   - RBUTTONDOWN	- ������ ���� �������� � ��������
	   - RBUTTONUP		- ������ ���� �������� � ��������
	   - LDBLCLICK		- ������� ������
	   - RDBLCLICK		- ������� ������
	   - MOUSELOST		- ���� �������
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   11.03.2001

************************************************************************/
#ifndef _STATIC_H_
#define _STATIC_H_

#include "Widget.h"

class Static : public Widget
{
public:
	enum LAST_EVENT
	{
		LE_NONE,
		LE_MOUSEMOVE,		//	- ����� ���������
		LE_LBUTTONDOWN,		//	- ������ ���� ��������
		LE_LBUTTONUP,		//	- ������ ���� ��������
		LE_LBUTTONPUSHED,	//	- ������ ���� ������
		LE_RBUTTONDOWN,		//	- ������ ���� �������� � ��������
		LE_RBUTTONUP,		//	- ������ ���� �������� � ��������
		LE_RBUTTONPUSHED,	//	- ������ ���� ������
		LE_LDBLCLICK,		//	- ������� ������
		LE_RDBLCLICK,		//	- ������� ������
		LE_MOUSELOST		//	- ���� �������
  	};
public:
	Static(const char *pName);
	virtual ~Static();
private:
	std::vector<WidgetImage*> m_pImages;		//	������ ��������		
	int m_State;								//	������ ������� �������� (����� ���������)
	LAST_EVENT m_LastEvent;
	Widget::Player::Sample* m_ClickSample;
public:
	//	�������� �������� (� �������������� ���������)
	void AddImage(WidgetImage *image);
	//	���������� ������ � ������������ ���������
	void SetState(unsigned int state);
	//	�������� ������� ���������
	int GetState(void) const;
	//	�������� ���-�� ���������
	unsigned int GetStateSize(void) const;
	//	�������� �������� �� �������
	WidgetImage* GetImage(unsigned int state) const;
	//	�������� ��������� ��������� ���������
	Static::LAST_EVENT LastEvent(void) const;
	//	���������� ���� �� ����
	void SetClickSample(Player::Sample* sample);
protected:
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������ �������� �����
	virtual bool ProcessMouseLost(void);
public:
	//	��������� ���������� ���������
	virtual void OnSystemChange(void);
public:
	virtual const char* Type(void) const {return "static";};
};

inline int Static::GetState(void) const
{
	return m_State;
}

inline Static::LAST_EVENT Static::LastEvent(void) const
{
	return m_LastEvent;
}
//	�������� ���-�� ���������
inline unsigned int Static::GetStateSize(void) const
{
	return m_pImages.size();
}

#endif	//_STATIC_H_