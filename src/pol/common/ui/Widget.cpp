/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.02.2001

************************************************************************/
#include "Precomp.h"
#include "Widget.h"

Widget::Widget(const char *pName) : m_sName(pName),m_pParent(0),m_pImage(0),m_Player(0),m_bVisible(true),m_bEnable(true)
{
	//	������� �����������������
	m_pLocation = new Location(this);
	//	������� ������ ���������
	m_Clipper = new Clipper(this);
	//	������� ������ ���������� ������� �����
	m_pFocus = new WidgetFocus(this);
}

Widget::~Widget()
{
	//	���������� �������� �� �������� �� �����������
	//	����������� �������
	if(m_pImage) m_pImage->Release();
	//	� ����������� ������������ ��� �������� �������
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
	{
		delete iW->m_pWidget;
	}
	//	����������� ������� ��������� � ������� �����
	delete m_pFocus;
	//	����������� ������� ��������� � ���������
	delete m_Clipper;
	//	����������� ������� ��������� � ������ ��������
	delete m_pLocation;
}
//	��������� � ������ ������ ��������� ������� �������
//	���� ������ � ����� �� ������ ��� ���������� ������������ 0
Widget* Widget::Insert(Widget *pWidget)
{	
	std::list<WidgetRef>::iterator iW;

	if(pWidget)
	{
		iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pWidget->Name());
		//	� ������� ������� ������ �������� ��������.
		//	������� ���������������� �� ��������� ������
		//	������� ������������� � ������ � ������������ �������,
		//	������� ������ ������� ������� � ��������.
		if(iW == m_lWidgets.end())
		{
			pWidget->m_pParent = this;
			if(m_pLocation->Region()->IsValid())
			{
				pWidget->m_pLocation->m_Origin.x = m_pLocation->OriginX()+m_pLocation->Region()->X();
				pWidget->m_pLocation->m_Origin.y = m_pLocation->OriginY()+m_pLocation->Region()->Y();
				pWidget->OnLocationChange();
			}
			m_lWidgets.push_back(WidgetRef(pWidget));
			return pWidget;
		}
	}

	return 0;
}
//	���������� ��������� ������� ������� �� �����, ��� �������
//	(���� ������� ������� �� �������) ������������ false
bool Widget::Delete(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		//	��������� �����
		m_pFocus->Clear(iW->m_pWidget);
		//	������� ������
		delete iW->m_pWidget;
		m_lWidgets.erase(iW);

		return true;
	}

	return false;
}
//	�������� ������ � ��������� ������ �� ������� ������������� �������
Widget* Widget::Remove(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);
	Widget* rWidget = 0;

	if(iW != m_lWidgets.end())
	{
		//	��������� �����
		m_pFocus->Clear(iW->m_pWidget);
		//	������� ������ �� ������
		rWidget = iW->m_pWidget;
		m_lWidgets.erase(iW);
	}

	return rWidget;
}
//	�������� �������� ������ �� �����
Widget* Widget::Child(const char *pName) 
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
		return iW->m_pWidget;

	return 0;
}
//	�������� �������� ������ �� ����� (0 - ���� ������� ���)
Widget* Widget::FromPoint(int x,int y)
{
	Widget* pChild;

	if(m_pLocation->Region()->PtInRegion(x-m_pLocation->m_Origin.x,y-m_pLocation->m_Origin.y))
	{
		for(std::list<WidgetRef>::reverse_iterator iW = m_lWidgets.rbegin();iW != m_lWidgets.rend();iW++)
		{
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_pLocation->Region()->PtInRegion(x-iW->m_pWidget->m_pLocation->m_Origin.x,y-iW->m_pWidget->m_pLocation->m_Origin.y))
			{
				if(pChild = iW->m_pWidget->FromPoint(x,y))
					return pChild;
				else
					return iW->m_pWidget;
			}
		}
	}
	
	return 0;
}
//	�������� ������ �� ������� �������� ������ �� ����� (0 - ���� ������� ���)
Widget* Widget::FirstChildFromPoint(int x,int y)
{
	Widget* pChild = 0;

	if(m_pLocation->Region()->PtInRegion(x-m_pLocation->m_Origin.x,y-m_pLocation->m_Origin.y))
	{
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
		{
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_pLocation->Region()->PtInRegion(x-iW->m_pWidget->m_pLocation->m_Origin.x,y-iW->m_pWidget->m_pLocation->m_Origin.y))
			{
				pChild = iW->m_pWidget;
			}
		}
	}
	
	return pChild;
}
//	��������� �������� ������ (�� �����) ������
bool Widget::BringToTop(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		m_lWidgets.push_back(WidgetRef(iW->m_pWidget));
		m_lWidgets.erase(iW);
		return true;
	}

	return false;
}
//	��������� �������� ������ (�� �����) �����
bool Widget::BringToBottom(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		m_lWidgets.push_front(WidgetRef(iW->m_pWidget));
		m_lWidgets.erase(iW);
		return true;
	}

	return false;
}
//	����������� ������� � ���� �������� ��������
void Widget::Render(WidgetView* pView)
{
	if(m_bVisible)
	{
		Draw(pView);						//	������������ ������ ������
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
			iW->m_pWidget->Render(pView);	//	������������ ��� �������� �������
	}
}
//	����������� �������
void Widget::Draw(WidgetView* pView)
{
	pView->Render(this);				
}

//	����������� �� ��������� ���� �� ��������� ���������
void Widget::OnChange(Widget* /*pChild*/)
{
}
//	����������� �� ��������� ������ ������ �������
void Widget::OnSystemChange(void)
{
	if(m_pImage) m_pImage->Reload();
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
		iW->m_pWidget->OnSystemChange();	//	���� �������� �������
}
//	��������� ��������� � ������������
void Widget::OnLocationChange(void)
{
	static int left,top,right,bottom;

	//	�������
	if(Parent())
	{
		if(m_Clipper->IsActive())
		{
			left	= Parent()->GetClipper()->SubX();
			top		= Parent()->GetClipper()->SubY();
			right	= left + Parent()->GetClipper()->Region()->Width();
			bottom	= top + Parent()->GetClipper()->Region()->Height();
			m_Clipper->Clip(m_pLocation->m_Region,left,top,right,bottom);
		}
		else
		{
			m_pLocation->m_Region.GetRect(&left,&top,&right,&bottom);
			m_Clipper->Clip(m_pLocation->m_Region,left,top,right,bottom);
		}
	}
	else
		m_Clipper->Clip(m_pLocation->m_Region,0,0,m_pLocation->m_Region.Width(),m_pLocation->m_Region.Height());
	//	�������� �������
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
	{
		iW->m_pWidget->m_pLocation->m_Origin.x = m_pLocation->OriginX()+m_pLocation->Region()->X();
		iW->m_pWidget->m_pLocation->m_Origin.y = m_pLocation->OriginY()+m_pLocation->Region()->Y();
		iW->m_pWidget->OnLocationChange();
	}
}
//	����������� � �������� ���������� ������� �������
void Widget::OnTick(void)
{
	if(Tick())
	{
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
			iW->m_pWidget->OnTick();	//	���� �������� ��������
	}
}
//	���������� �����-�� ��������
bool Widget::Tick(void)
{
	return true;
}
//	������� ������ � ��� ��� �������� ������� 
void Widget::Move(int dx,int dy)
{
	m_pLocation->m_Region.Transform(1,0,dx,0,1,dy);
	OnLocationChange();
}
//	�������� ������ � ��� ��� �������� ������� � �������� �����
void Widget::MoveTo(int x,int y)
{
	Move(x-m_pLocation->Region()->X(),y-m_pLocation->Region()->Y());
}
//	����������� � ������� ���� �� ������������ �������
//	true - ���������� �����������
bool Widget::OnMouse(VMouseState* pMS)
{
	if(ProcessMouseInput(pMS))
	{
		for(std::list<WidgetRef>::reverse_iterator iW = m_lWidgets.rbegin();iW != m_lWidgets.rend();iW++)
		{
			Location* l = iW->m_pWidget->GetLocation();
			const VRegion* region = l->Region();
			bool in_region = region->PtInRegion(pMS->x-l->m_Origin.x,pMS->y-l->m_Origin.y);
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_bEnable && in_region)
			{//	����� ������ �� ������� ������������ ����
				if(m_pFocus->Get(WidgetFocus::FF_MOUSE) != iW->m_pWidget)
				{
					m_pFocus->Set(WidgetFocus::FF_MOUSE,iW->m_pWidget);
				}
				if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
					m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouse(pMS);
				
				return false;
			}
		}
		if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
			m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouseLost();
		m_pFocus->Clear(WidgetFocus::FF_MOUSE,m_pFocus->Get(WidgetFocus::FF_MOUSE));
	}

	return true;
}

//	��������� ��������� �����
bool Widget::ProcessMouseInput(VMouseState* /*pMS*/)
{
	return true;
}

//	����������� � ������� ������������� ������ �� ������������ �������
bool Widget::OnKeyboard(VKeyboardState* pKS)
{
	if(m_bVisible && m_bEnable)
	{
		if(ProcessKeyboardInput(pKS))
		{
			if(m_pFocus->Get(WidgetFocus::FF_KEYBOARD))
				m_pFocus->Get(WidgetFocus::FF_KEYBOARD)->OnKeyboard(pKS);
		}

		return false;
	}

	return true;
}
//	��������� ������������� �����
bool Widget::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	return true;
}
//	����������� � ������ ��������� ������
void Widget::OnMouseLost(void)
{
	if(ProcessMouseLost())
	{
		if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
			m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouseLost();
	}
}

bool Widget::ProcessMouseLost(void)
{
	return true;
}
//	����������� � ������ ������������� ������
void Widget::OnKeyboardLost(void)
{
	if(ProcessKeyboardLost())
	{
		if(m_pFocus->Get(WidgetFocus::FF_KEYBOARD))
			m_pFocus->Get(WidgetFocus::FF_KEYBOARD)->OnKeyboardLost();
	}
}

bool Widget::ProcessKeyboardLost(void)
{
	return true;
}
//	���������� ��������� �������
void Widget::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
	if(!m_bVisible && m_pParent) m_pParent->Focus()->Clear(this);
}

//	���������� ������ �����
void Widget::Enable(bool bEnable)
{
	m_bEnable = bEnable;
	if(!m_bEnable && m_pParent) m_pParent->Focus()->Clear(this);
}

//	������������� ������ ������������� ������
Widget::Player* Widget::SetPlayer(Player* player)
{
	Player* old_player = m_Player;
	m_Player = player;

	return old_player;
}

//*********************************************************************//
//********************* class Widget::Location **************************//
Widget::Location::Location(Widget *pOwner) : m_pOwner(pOwner)
{
}

Widget::Location::~Location()
{
}

void Widget::Location::Create(VPoint *pPoints,unsigned int num)
{
	m_Region.Create(pPoints,num);
	m_pOwner->OnLocationChange();
}

void Widget::Location::Create(int left,int top,int right,int bottom)
{
	m_Region.Create(left,top,right,bottom);
	m_pOwner->OnLocationChange();
}

void Widget::Location::Create(const VRegion& r)
{
	m_Region = r;
	m_pOwner->OnLocationChange();
}

//*********************************************************************//
//******************** class WidgetMouseFocus *************************//
WidgetMouseFocus::WidgetMouseFocus(Widget *pOwner) : VFocus(pOwner)
{
}

bool WidgetMouseFocus::Set(Widget *pChild)
{
	if(pChild && pChild->m_bEnable && pChild->m_bVisible && pChild->Focus()->IsEnable(WidgetFocus::FF_MOUSE))
	{
		if(m_pUnderFocus) m_pUnderFocus->OnMouseLost();
		m_pUnderFocus = pChild;
		return true;
	}

	return false;
}

//	���� �� pChild ���� ����� ����������� ��� ��� ������
void WidgetMouseFocus::Clear(Widget * /*pChild*/)
{
	m_pUnderFocus = 0;
}
//*********************************************************************//
//****************** class WidgetKeyboardFocus ************************//
WidgetKeyboardFocus::WidgetKeyboardFocus(Widget *pOwner) : VFocus(pOwner)
{
}

bool WidgetKeyboardFocus::Set(Widget *pChild)
{
	if(pChild && pChild->m_bEnable && pChild->m_bVisible && pChild->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
	{
		if(m_pUnderFocus) m_pUnderFocus->OnKeyboardLost();
		m_pUnderFocus = pChild;
		return true;
	}

	return false;
}

Widget* WidgetKeyboardFocus::Next(void)
{
	std::list<WidgetRef>::reverse_iterator iW;
	int size = m_pOwner->m_lWidgets.size();

	if(m_pUnderFocus)
	{
		iW = std::find(m_pOwner->m_lWidgets.rbegin(),m_pOwner->m_lWidgets.rend(),m_pUnderFocus->Name());
		iW++;	//	������� ��������
		if(iW == m_pOwner->m_lWidgets.rend()) iW = m_pOwner->m_lWidgets.rbegin();
	}
	else
	{
		iW = m_pOwner->m_lWidgets.rbegin();
	}
	if(iW != m_pOwner->m_lWidgets.rend())
	{
		for(int i=0;i<size;i++)
		{
			if(iW->m_pWidget->m_bEnable && iW->m_pWidget->m_bVisible && iW->m_pWidget->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
			{
				if(m_pUnderFocus && (m_pUnderFocus != iW->m_pWidget))
					m_pUnderFocus->OnKeyboardLost();
				m_pUnderFocus = iW->m_pWidget;
				break;
			}
			iW++;//	������� ��������
			if(iW == m_pOwner->m_lWidgets.rend()) iW = m_pOwner->m_lWidgets.rbegin();
		}
	}

	return m_pUnderFocus;
}

Widget* WidgetKeyboardFocus::Prev(void)
{
	std::list<WidgetRef>::iterator iW;
	int size = m_pOwner->m_lWidgets.size();

	if(m_pUnderFocus)
	{
		iW = std::find(m_pOwner->m_lWidgets.begin(),m_pOwner->m_lWidgets.end(),m_pUnderFocus->Name());
		iW++;	//	������� ��������
		if(iW == m_pOwner->m_lWidgets.end()) iW = m_pOwner->m_lWidgets.begin();
	}
	else
	{
		iW = m_pOwner->m_lWidgets.begin();
	}
	if(iW != m_pOwner->m_lWidgets.end())
	{
		for(int i=0;i<size;i++)
		{
			if(iW->m_pWidget->m_bEnable && iW->m_pWidget->m_bVisible && iW->m_pWidget->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
			{
				if(m_pUnderFocus && (m_pUnderFocus != iW->m_pWidget))
					m_pUnderFocus->OnKeyboardLost();
				m_pUnderFocus = iW->m_pWidget;
				break;
			}
			iW++;//	������� ��������
			if(iW == m_pOwner->m_lWidgets.end()) iW = m_pOwner->m_lWidgets.begin();
		}
	}

	return m_pUnderFocus;
}

void WidgetKeyboardFocus::Clear(Widget *pChild)
{
	if(pChild)
	{
		if(m_pUnderFocus == pChild)
		{
			m_pUnderFocus = 0;
		}
	}
}
//*********************************************************************//
//************************* class WidgetFocus **************************//
WidgetFocus::WidgetFocus(Widget *pOwner) 
{
	m_pFFixers[FF_MOUSE] = new WidgetMouseFocus(pOwner);
	m_pFFixers[FF_KEYBOARD] = new WidgetKeyboardFocus(pOwner);
}

WidgetFocus::~WidgetFocus()
{
	delete m_pFFixers[FF_MOUSE];
	delete m_pFFixers[FF_KEYBOARD];
}

//	���������� �����
bool WidgetFocus::Set(FOCUS_FIXER ff,Widget *pChild)
{
	return m_pFFixers[ff]->Set(pChild);
}
//	�������� ������� � �������	
Widget* WidgetFocus::Get(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Get();
}
//	�������� �����
Widget* WidgetFocus::Next(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Next();
}
//	�������� �����
Widget* WidgetFocus::Prev(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Prev();
}
//	���� �� pChild ���� ����� ����������� ��� ��� ������
void WidgetFocus::Clear(FOCUS_FIXER ff,Widget *pChild)
{
	m_pFFixers[ff]->Clear(pChild);
}
//	���� �� pChild ���� ����� ����������� ��� ��� ������
void WidgetFocus::Clear(Widget *pChild)
{
	for(int ff=0;ff<MAX_FIXER;ff++)
		m_pFFixers[ff]->Clear(pChild);
}

//************************************************************************//
//*****************	       class Widget::Clipper       *******************//
Widget::Clipper::Clipper(Widget *pOwner) : m_pOwner(pOwner),m_IsActive(true)
{
	m_SubX = m_SubY = 0;
}

Widget::Clipper::~Clipper()
{
}
//	������������/�������������� 
void Widget::Clipper::Activate(bool activate)
{
	m_IsActive = activate;
	m_pOwner->OnLocationChange();
}
//	�������� ������ ������ ����������� (������� ������� ������� �����������)
void Widget::Clipper::Clip(const VRegion& rgn,int left,int top,int right,int bottom)
{
	m_Region.Create(std::max(rgn.X(),left),
					std::max(rgn.Y(),top),
					std::min(rgn.X()+rgn.Width(),right),
					std::min(rgn.Y()+rgn.Height(),bottom));

	m_SubX		= m_Region.X()-rgn.X();
	m_SubY		= m_Region.Y()-rgn.Y();
}


