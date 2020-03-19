/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
		 ������� ����� ��� �������� ���������.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.02.2001

************************************************************************/
#ifndef _WIDGET_H_
#define _WIDGET_H_

#include "VData.h"

class Widget;
class WidgetView;
class WidgetImage;
class WidgetFocus;
class WidgetMouseFocus;
class WidgetKeyboardFocus;

//	������ �� ������
struct WidgetRef
{
	Widget* m_pWidget;
	WidgetRef(){m_pWidget = 0;}
	WidgetRef(Widget* pWidget){m_pWidget = pWidget;}
};

//	�������� �������
class Widget
{
public:
	class Iterator;
	class Location;
	class Clipper;
	class Player;
	friend Iterator;
	friend Location;
	friend Clipper;
	friend WidgetMouseFocus;
	friend WidgetKeyboardFocus;
private:
	//	��� ������� Widget'�
	std::string m_sName;
	//	������������ ������
	Widget *m_pParent;
	//	���� ���������
	bool m_bVisible;
	//	���� ����������� ��� �����
	bool m_bEnable;
	//	�������� �����������
	WidgetImage *m_pImage;
	//	����������������� �������
	Location *m_pLocation;
	//	������ ��������� �����������
	Clipper* m_Clipper; 
	//	������ ���������� ������� �����
	WidgetFocus *m_pFocus;
protected:
	//	������ ���������� �������� ��������������
	Player* m_Player;
public:
	struct Tip
	{
		std::string m_Text;
		bool m_Activated;
		Tip() : m_Activated(false) {}
	} m_Tip;
private:
	//	������ ������ �� �������� �������
	std::list<WidgetRef> m_lWidgets;
public:
	//	��� �������� �������, ���������� ������ ��� ���
	Widget(const char *pName);
	virtual ~Widget();
public:
	//	��������� � ������ ������ ��������� ������� �������
	//	���� ������ � ����� �� ������ ��� ���������� ������������ 0
	virtual Widget* Insert(Widget *pWidget);
	//	���������� ��������� ������� ������� �� �����, ��� �� �����
	//	(���� ������� ������� �� �������) ������������ false
	virtual bool Delete(const char *pName);
	//	�������� ������ � ��������� ������ �� ������� ������������� �������
	virtual Widget* Remove(const char *pName);
	//	�������� �������� ������ �� �����
	virtual Widget* Child(const char *pName); 
	//	��������� �������� ������ (�� �����) ������
	virtual bool BringToTop(const char *pName);
	//	��������� �������� ������ (�� �����) �����
	virtual bool BringToBottom(const char *pName);
	//	�������� ������������ ������
	virtual Widget* Parent(void) const;
	//	�������� ����� �������� �������� ������ �� ����� (0 - ���� ������� ���)
	virtual Widget* FromPoint(int x,int y);
	//	�������� ������ �� ������� �������� ������ �� ����� (0 - ���� ������� ���)
	virtual Widget* FirstChildFromPoint(int x,int y);
	//	���������� ��� �������
	virtual const char* Name(void) const;
	//	���������� ��� �������
	virtual const char* Type(void) const {return "widget";};
	//	����������� �� ��������� ���� �� ��������� ���������
	virtual void OnChange(Widget* pChild);
	//	����������� �� ��������� ������ ������ �������
	virtual void OnSystemChange(void);
	//	����������� � �������� ���������� ������� �������
	virtual void OnTick(void);
	//	����������� ������� � ���� �������� ��������
	virtual void Render(WidgetView* pView);
	//	����������� �������
	virtual void Draw(WidgetView* pView);
	//	���������� ��������� �������
	virtual void SetVisible(bool bVisible);
	//	�������� ��������� �������
	virtual bool IsVisible(void);
	//	���������� �������� �����������
	virtual void SetImage(WidgetImage* pImage);
	//	�������� �������� �����������
	virtual WidgetImage* GetImage(void) const;
	//	�������� ������������ �������
	virtual Widget::Location* GetLocation(void) const;
	//	�������� �������
	virtual Widget::Clipper* GetClipper(void) const;
	//	�������� ������ � ��� ��� �������� ������� �� ������������ ��������
	virtual void Move(int dx,int dy);
	//	�������� ������ � ��� ��� �������� ������� � �������� �����
	virtual void MoveTo(int x,int y);
	//	���������� ������ ����� 
	virtual void Enable(bool bEnable);
	//	�������� ����������� �������
	virtual bool IsEnable(void);
	//	����������� � ������� ���� �� ������������ �������
	virtual bool OnMouse(VMouseState* pMS);
	//	����������� � ������� ������������� ������ �� ������������ �������
	virtual bool OnKeyboard(VKeyboardState* pKS);
	//	��������� ������ �����
	virtual bool ProcessMouseLost(void);
	//	��������� ������ �����
	virtual bool ProcessKeyboardLost(void);
	//	������ ���������� �������
	virtual WidgetFocus* Focus(void) const;
	//	������������� ������ ������������� ������
	virtual Player* SetPlayer(Player* player);
	//	������ � ���������
	Widget::Iterator Widget::Begin(void);
	Widget::Iterator Widget::End(void);
protected:
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	����������� � ������ ��������� ������
	virtual void OnMouseLost(void);
	//	����������� � ������ ������������� ������
	virtual void OnKeyboardLost(void);
	//	���������� �����-�� ��������
	virtual bool Tick(void);
	//	��������� ��������� � ������������
	virtual void OnLocationChange(void);
};

//	�������� ��������� ������ �� �����
inline bool operator == (const WidgetRef& x,const char *pName)
{
	return !strcmp(x.m_pWidget->Name(),pName);
}

//	���������� ��� �������
inline const char* Widget::Name(void) const
{
	return m_sName.c_str();
}

//	�������� ������������ ������
inline Widget* Widget::Parent(void) const
{
	return m_pParent;
}

//	���������� �������� �����������
inline void Widget::SetImage(WidgetImage* pImage)
{
	m_pImage = pImage;
}
//	�������� ������������ �������
inline Widget::Location* Widget::GetLocation(void) const
{
	return m_pLocation;
}
//	�������� �������
inline Widget::Clipper* Widget::GetClipper(void) const
{
	return m_Clipper;
}

inline WidgetFocus* Widget::Focus(void) const
{
	return m_pFocus;
}

inline WidgetImage* Widget::GetImage(void) const
{
	return m_pImage;
}

inline bool Widget::IsVisible(void)
{
	return m_bVisible;
}

inline bool Widget::IsEnable(void)
{
	return m_bEnable;
}

//*********************************************************************//
//********************* class Widget::Location **************************//
class Widget::Location
{
private:
//	class Clipper;
	//	���������� ������������
	Widget *m_pOwner;
	friend Widget;
	//	������ ����� � ����������� � ��������� ����������� ��������
	VRegion m_Region;
	//	����� ������ ������� � ���������� �����������
	VPoint m_Origin;
/*	//	������ ��������� �����������
	Clipper* m_Clipper;*/
public:
	Location(Widget *pOwner);
	virtual ~Location();
public:
	void Create(VPoint *pPoints,unsigned int num);
	void Create(int left,int top,int right,int bottom);
	void Create(const VRegion& r);
public:
	const VRegion* Region(void){return &m_Region;}
	int OriginX(void){return m_Origin.x;}
	int OriginY(void){return m_Origin.y;}
/*public:
	//	��������� � ���������� ��������
	void ActivateClipper(bool activate);
	//	��������� �������� �����������
	int SubClipX(void);
	int SubClipY(void);*/
};

//*********************************************************************//
//************************* class VFocus ******************************//
class VFocus
{
protected:
	//	���������� ������
	Widget *m_pOwner;
	//	���������� ������
	Widget *m_pUnderFocus;
	//	������������ ��� ���
	bool m_bEnable;
public:
	VFocus(Widget *pOwner){m_pOwner = pOwner;m_pUnderFocus = 0;m_bEnable = true;}
	virtual ~VFocus(){};
public:
	virtual bool Set(Widget *pChild) = 0;		//	���������� �����	
	virtual Widget *Get(void) = 0;				//	�������� ������� � �������	
	virtual Widget *Next(void) = 0;				//	�������� �����	
	virtual Widget *Prev(void) = 0;				//	�������� �����
	virtual void Clear(Widget *pChild) = 0;		//	���� �� pChild ���� ����� ����������� ��� ��� ������
	void Enable(bool bEnable){m_bEnable = bEnable;};
	bool IsEnable(void){return m_bEnable;};
};

//*********************************************************************//
//******************** class WidgetMouseFocus *************************//
class WidgetMouseFocus : public VFocus
{
public:
	WidgetMouseFocus(Widget *pOwner);
	virtual ~WidgetMouseFocus(){};
public:
	bool Set(Widget *pChild);	
	Widget *Get(void){return m_pUnderFocus;}				
	Widget *Next(void){return m_pUnderFocus;}
	Widget *Prev(void){return m_pUnderFocus;}
	void Clear(Widget *pChild);
};

//*********************************************************************//
//****************** class WidgetKeyboardFocus ************************//
class WidgetKeyboardFocus : public VFocus
{
public:
	WidgetKeyboardFocus(Widget *pOwner);
	virtual ~WidgetKeyboardFocus(){};
public:
	bool Set(Widget *pChild);	
	Widget *Get(void){return m_pUnderFocus;}				
	Widget *Next(void);
	Widget *Prev(void);
	void Clear(Widget *pChild);
};

//*********************************************************************//
//************************* class WidgetFocus **************************//
class WidgetFocus
{
public:
	enum FOCUS_FIXER
	{
		FF_MOUSE,
		FF_KEYBOARD,
		MAX_FIXER
	};
private:
	VFocus *m_pFFixers[MAX_FIXER];
public:
	WidgetFocus(Widget *pOwner);
	virtual ~WidgetFocus();
public:
	bool Set(FOCUS_FIXER ff,Widget *pChild);			//	���������� �����	
	Widget *Get(FOCUS_FIXER ff);						//	�������� ������� � �������	
	Widget *Next(FOCUS_FIXER ff);						//	�������� �����	
	Widget *Prev(FOCUS_FIXER ff);						//	�������� �����
	void Clear(FOCUS_FIXER ff,Widget *pChild);			//	���� �� pChild ���� ����� ����������� ��� ��� ������
	void Clear(Widget *pChild);							//	���� �� pChild ���� ����� ����������� ��� ��� ������
	void Enable(FOCUS_FIXER ff,bool bEnable){m_pFFixers[ff]->Enable(bEnable);};
	bool IsEnable(FOCUS_FIXER ff){return m_pFFixers[ff]->IsEnable();};
};
//*********************************************************************//
//************************* class WidgetImage **************************//
class WidgetImage	//	���������� �� ���������� �������� ������
{
public:
	WidgetImage(){};
	virtual ~WidgetImage(){};
public:
	//	����������� �������� ������ � ����������� � �������
	virtual void Reload(void) = 0;
	//	���������� ������� ��������� � ���������
	virtual void Release(void) = 0;

	/// ���������� ������ ��������
	virtual int GetWidth() = 0;
};

//*********************************************************************//
//************************* class WidgetView **************************//
class WidgetView	//	���������� �� ���������� ������ ������� �� �����
{
public:
	WidgetView(){};
	virtual ~WidgetView(){};
public:
	virtual void Render(Widget *pWidget) = 0;
};

//************************************************************************//
//****************          class Widget::Iterator       *****************//
class Widget::Iterator
{
private:
	std::list<WidgetRef>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::list<WidgetRef>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	Widget* operator->(void);
	Widget& operator*(void);
	bool operator!=(const Iterator& it);
};

inline Widget::Iterator& Widget::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline Widget::Iterator Widget::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline Widget* Widget::Iterator::operator->(void)
{
	return m_it->m_pWidget;
}

inline Widget& Widget::Iterator::operator*(void)
{
	return *operator->();
}

inline bool Widget::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//************************************************************************//
//*****************	             class Widget          *******************//
inline Widget::Iterator Widget::Begin(void)
{
	return Iterator(m_lWidgets.begin());
}

inline Widget::Iterator Widget::End(void)
{
	return Iterator(m_lWidgets.end());
}

//************************************************************************//
//*****************	class Widget::Clipper    *******************//
class Widget::Clipper
{
private:
	//	���������� ������
	Widget *m_pOwner;
	//	������������� ���������
	VRegion m_Region;
	//	������� ����� ���� ��������������� - (��������� � �������)
	int m_SubX;
	int m_SubY;
	//	���� ������������� ��������
	bool m_IsActive;
public:
	Clipper(Widget *pOwner);
	virtual ~Clipper();
public:
	//	�������� ������ ������ ����������� (������� ������� ������� �����������)
	void Clip(const VRegion& rgn,int left,int top,int right,int bottom);
	//	������������/�������������� 
	void Activate(bool activate);
public:
	const VRegion* Region(void){return &m_Region;}
	int SubX(void){return m_SubX;}
	int SubY(void){return m_SubY;}
	bool IsActive(void){return m_IsActive;}
};

//************************************************************************//
//	����� ��� ������������ ������
//	class Widget::Player
class Widget::Player
{
public:
	//************************************************************************//
	//	��� ������ ��� ����������� �����
	//	class Widget::Player::Sample
	class Sample
	{
	public:
		virtual ~Sample() {}
		//	���������� ������� ��������� � �������
		virtual void Release(void) = 0;
	};
public:
	virtual ~Player() {}
	virtual void Play(Sample* sample) = 0;
	virtual void Stop(Sample* sample) = 0;
};

#endif	//_WIDGET_H_