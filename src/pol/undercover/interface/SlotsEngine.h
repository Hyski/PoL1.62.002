/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
		����� SlotEngine ��������������� ��� �������� �� �������,
		����������� ������� �� � ��������.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.09.2001

************************************************************************/
#ifndef _SLOTS_ENGINE_H_
#define _SLOTS_ENGINE_H_

#include <common/UI/Widget.h>

//#define _DEBUG_SLOTS_ENGINE

class SEMigrationMgrController;
class SlotsEngine;
class SESlotView;
class SEItemView;
class BaseThing;
class SEShadow;
class DXImage;
class Static;
class SESlot;
class Text;
class TxtFile;

extern const char* g_InventoryPictures;

//**********************************************************************
//	class SEItem
class SEItem
{
	friend SESlot;
public:
	enum STATE 
	{
		ST_NORMAL		=    0,
		ST_HIGHLIGHTED	= 1<<1,
		ST_GRAYED		= 1<<2,
		ST_NORANK		= 1<<3
	};
	static const int m_SlotIdIsAbsent;		//	������������� ����� �������� �� ����������
private:
	unsigned int m_State;					//	���������
	ipnt2_t m_Position;						//	������� � ����� (� ������� �����)
	ipnt2_t m_Size;							//	������� (� ������� �����)
	BaseThing* m_BaseThing;					//	����� � �������
	int m_SlotId;							//	�������������� � �����
	SEItemView* m_View;						//	������� ��� �����
public:
	SEItem(const char* system_name,const ipnt2_t& size,BaseThing* bs);
	virtual ~SEItem();
public:
	//	���������� ��������� �����
	void SetState(unsigned int state);
	//	���������� ��������� ���� �� �����
	void SetText(const char* text);
	//	���������� ������� ����� � ����� (���������� � ������� �����)
	void SetPosition(const ipnt2_t& pos);
	//	�������� ������� �����
	const ipnt2_t& Position(void) const {return m_Position;}
	//	�������� ������ �����
	const ipnt2_t& Size(void) const {return m_Size;}
	//	�������� ������������� �����
	int SlotId(void) const {return m_SlotId;}
	//	�������� � ������ ��������������
	void OnDrag(Widget* new_container);
	//	�������� � ����� ��������������
	void OnDrop(void);
	//	�������� ��������
	WidgetImage* GetImage(void) const;
	//	�������� ��������� ���������� ����������� �����
	ipnt2_t GetScreenPlace(void) const;
public:
	BaseThing* GetBaseThing(void) {return m_BaseThing;}
private:
	//	����������� ������� ��� ����� (�������� ����������)
	void MoveTo(Widget* container,int x,int y);
	//	����������� ������� ��� ����� (�������� ����������)
	void ScrollTo(int x,int y);
	//	���������������� �������
	void EnableClipper(bool enable);
};

//**********************************************************************
//	class SEItemView
class SEItemView : public Widget
{
public:
	static const char* m_WidgetType;
	static const char* m_WrongImage;
private:
	int m_OffsetX;					//	�������� �������� �� ������� �������
	int m_OffsetY;					//	�������� �������� �� ������� �������
	int m_Width;					//	������ ������
	int m_Height;					//	������ ������
	int m_DragX;	
	int m_DragY;
	bool m_IsDragged;
private:
	enum IMAGE
	{
		I_NORMAL,
		I_HIGHLIGHTED,
		I_GRAYED,
		I_NORANK,
		MAX_IMAGE
	};
	DXImage* m_Images[MAX_IMAGE];
	IMAGE m_CurImage;
	Text* m_Text;
public:
	SEItemView(const char* name,const char* system_name,const ipnt2_t& size_in_cells);
	virtual ~SEItemView();
public:
	void MoveTo(int x,int y);
	void SetState(unsigned int state);
	//	�������� � ������ ��������������
	void OnDrag(void);
	//	�������� � ����� ��������������
	void OnDrop(void);
	//	�������� �������� �����������
	WidgetImage* GetBaseImage(void) const;
	//	���������� ��������� ���� �� �����
	void SetText(const char* text);
private:
	//	���������� �����-�� ��������
	bool Tick(void);
	//	��������� ���������� ���������
	void OnSystemChange(void);
public:
	virtual const char* Type(void) const {return m_WidgetType;}
};

//**********************************************************************
//	class SESlot
class SESlot
{
public:
	class Iterator;
	enum STATE {ST_NORMAL,ST_SELECTED};
	enum EVENT_TYPE {ET_NONE,ET_MOUSE_LEAVE,ET_MOUSE_PRESENT,ET_LEFT_CLICK,ET_RIGHT_CLICK};	
	enum MODE 
	{
		M_INSIDE_INTERFACE,					//	���� ������������� � ���������� � ����� ����������
		M_OUTSIDE_INTERFACE					//	���� �������� ���� ��������, �� ���������� �����������
	};
	enum PLACEMENT
	{
		P_NORMAL_SLOT,						//	� ���� ����� �������� ������� ������ ������� ������
		P_ONE_ITEM_IN_SLOT,					//	� ���� ����� ��������� ������ ���� ����
		P_UNLIMITED,						//	� ���� ����� ��������� �������������� ����� ������
	};
	enum ALIGN
	{
		A_LEFT_TO_RIGHT,					//	���������� � ����� ���������� ����� �������
		A_RIGHT_TO_LEFT,					//	���������� � ����� ���������� ������ ������
	};
	enum EXPANSION_MODE
	{
		EM_TO_RIGHT,
		EM_TO_DOWN
	};
	static const int m_CellWidth;			//	������ ������ � ��������
	static const int m_CellHeight;			//	������ ������ � ��������
	static const float m_DetonateTime;		//	�����, ����� �������� ��������� ��������� �����
private:
	int m_X;								//	���������� � ������������� ���������� (� ��������)
	int m_Y;								//	���������� � ������������� ���������� (� ��������)
	int m_Width;							//	������ ����� � �������
	int m_Height;							//	������ ����� � �������
	//	������ �����
	enum CELL_STATE {CS_EMPTY,CS_FILLED,CS_INACCESSIBLE};
	std::vector<std::vector<CELL_STATE> > m_Cells;
	ipnt2_t m_CellsOffset;					//	���������� �������� �����
	EXPANSION_MODE m_ExpMode;				//	����� ���������� ��� ������������ �����
	PLACEMENT m_Placement;					//	�������� ������������ ������
	ALIGN m_Align;							//	�������� ���������� ������
	MODE m_Mode;							//	����� ������ �����
	//	�������� �����
	SlotsEngine* m_FeedBack;
	//	������� ��� �����
	SESlotView* m_View;
	//	������ ������ � �����
	std::list<SEItem*> m_Items; 
	SEItem* m_ActiveItem;					//	���������� �������� ����
	float m_ActivateTime;					//	����� ������������� �����
	bool m_IsFirstTimeStart;				//	� ������ ��� ������� ����������� START
private:
	int m_Id;								//	������������� �����
	STATE m_State;							//	��������� �����
public:
	SESlot(int id,Widget* container,SlotsEngine* feed_back);
	virtual ~SESlot();
public:
	//	���������� ������������ ����� (x,y - ���������� � ��������; width,height - � ������� �����)
	void SetLocation(int x,int y,int width,int height);
	void SetState(STATE state);
	STATE State(void) const {return m_State;}
	void SetPlacement(PLACEMENT placement);
	void SetAlign(ALIGN align);
	void SetExpMode(EXPANSION_MODE em);
	void SetInaccessibleCells(int left,int top,int right,int bottom);
	void SetSelectedImage(const char* image_name,int x,int y);
public:
	//	����� ������� ���� ������� ���������� ������� CanInsert() ��� ������� �����
	void Insert(SEItem* item);
	void Swap(SEItem* item_bottom,SEItem* item_top);
	//	������� ���� �� ����� (�� �� �� ������, ������ �� ������������)
	SEItem* Remove(SEItem* item);
	//	������� ��� ����� �� ����� � ������� ������� � ������
	void Clear(void);
	bool CanInsert(SEItem* item,const ipnt2_t* pos = 0);
	bool CanSwap(SEItem* item_bottom,SEItem* item_top);
	//	�������� ������������� �����
	int Id(void) const {return m_Id;}
	//	������, ���������� �� ���� � ���� � ��������
	bool CanPlace(SEItem* item);
	//	������, ���������� �� ���� � ���� � ��������
	bool CanPlace(const ipnt2_t& size);
	//	������ �������� �� ������ ��������� (�������� ����������, �� ������ �������� �� ����� �����)
	bool IsAccessibleCell(int x,int y);
	//	������ �������� �� ������ ��������� (���������� � ������� �����)
	bool IsAccessibleCell(const ipnt2_t& cell_xy);
	//	�������� ���������� ������ �� ��������� ���-��
	void Scroll(int dx,int dy);
	//	�������� ���������� ������ � ������
	void UnScroll(void);
	//	�������� ���������� ������� �����
	ipnt2_t Size(void) const;
	//	���������� �������� ��������
	bool ProcessMouseWheel(float dz);
public:
	//	������ � ����������
	SESlot::Iterator Begin(void);
	SESlot::Iterator End(void);
private: //	������ � ��������
	//	���������� ��������� ������������� ������� �����
	void SetCells(int left,int top,int right,int bottom,CELL_STATE state);
	//	��������� ��������� �����
	bool CheckCells(int left,int top,int right,int bottom,CELL_STATE state);
	//	���������� ����� 
	bool GetPlace(const ipnt2_t& size,const ipnt2_t* where,ipnt2_t* result);
	//	���������� ����� �� �����
	bool LeftToRight(const ipnt2_t& size,ipnt2_t* result);
	//	���������� ������ �� ����
	bool RightToLeft(const ipnt2_t& size,ipnt2_t* result);
	//	������� ��������
	bool SwingOfPendulum(const ipnt2_t& size,const ipnt2_t& where,ipnt2_t* result);
	//	����� ������������� ������� OnHanging
	bool IsDetonate(void) const;
	//	������� ��������� ��������� � ������ �����
	ipnt2_t ScreenToSlot(int x,int y);
	//	������� � ��������� ����������
	ipnt2_t SlotToScreen(int x,int y);
	//	������ ��������� �� ����� � ����� (x,y - ���������� �����)
	bool PtInItem(const SEItem* item,const ipnt2_t& slot_xy);
	//	�������� ���� (�������� ����������)
	void ShowShadow(bool enable,SEItem* item_for = 0,int x = 0,int y = 0);
	//	���������� �� ����
	bool IsShowShadow(void);
	//	���������� ������ ������ 
	void ReleaseRedundantCells(void);
public:
	//	�������� �������
	void OnEvent(int x,int y,EVENT_TYPE type);

#ifdef _DEBUG_SLOTS_ENGINE
public:
	void Render(HDC hdc);
#endif

	static void Init(SESlot *slot, int i, const TxtFile &);
};

//**********************************************************************
//	class SESlotView
class SESlotView : public Widget
{
public:
	static const char* m_WidgetType;
private:
	SESlot::EVENT_TYPE m_PrevEvent;						
	SESlot* m_FeedBack;
	Widget* m_SelectedImage;
public:
	SESlotView(Widget* container,SESlot* feed_back);
	virtual ~SESlotView();
public:
	//	���������� �������� �� ���������
	void SetSelectedImage(const char* image_name,int x,int y);
	//	����������/����� ���������
	void SetSelected(bool enable);
private:
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ����-����
	bool Tick(void);
};


//************************************************************************//
//	class SESlot::Iterator
class SESlot::Iterator
{
private:
	std::list<SEItem*>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::list<SEItem*>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	SEItem* operator->(void);
	SEItem& operator*(void);
	bool operator!=(const Iterator& it);
};

inline SESlot::Iterator& SESlot::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline SESlot::Iterator SESlot::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline SEItem* SESlot::Iterator::operator->(void)
{
	return *m_it;
}

inline SEItem& SESlot::Iterator::operator*(void)
{
	return *operator->();
}

inline bool SESlot::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//**********************************************************************
//	class SESlot
inline SESlot::Iterator SESlot::Begin(void)
{
	return Iterator(m_Items.begin());
}

inline SESlot::Iterator SESlot::End(void)
{
	return Iterator(m_Items.end());
}

//**********************************************************************
//	class SEMigrationMgr
class SEMigrationMgr
{
public:
	static const char* m_ShadowImageName;
private:
	SEMigrationMgrController* m_Controller;
	SlotsEngine* m_FeedBack;
	Widget* m_Container;
	Widget* m_Shadow;
	SEItem* m_Item;
	ipnt2_t m_Offset;
public:
	SEMigrationMgr(SlotsEngine* feed_back,Widget* container);
	virtual ~SEMigrationMgr();
public:
	void Insert(SEItem* item);
	SEItem* Remove(void);
	SEItem* GetItem(void);	// �������� ����, ������� ����� �� �������
	void SetController(SEMigrationMgrController* controller);
public:
	//	������������ ���� (�������� ����������)
	void ShowShadow(bool enable,const ipnt2_t& xy);
private:
	//	������� ����� ����
	void CreateNewShadow(WidgetImage* wi,const ipnt2_t& size_in_cells);
};

//**********************************************************************
//	class SEMigrationMgrController
class SEMigrationMgrController
{
public:
	virtual ~SEMigrationMgrController() {}
	virtual void OnDrag(void) = 0;
	virtual void OnDrop(void) = 0;
};

//**********************************************************************
//	class SEController
class SEController
{
public:
	enum LIGHTING_MODE 
	{
		LM_START,				//	��������� ������ ������������
		LM_FINISH				//	�������� ���������� ���������
	};
public:
	virtual ~SEController(){}
	virtual void OnLighting(SEItem* item,LIGHTING_MODE lm) = 0;
	virtual void OnDrag(SESlot* slot,SEItem* item) = 0;
	virtual void OnDrop(SESlot* slot,SEItem* item,const ipnt2_t& pos) = 0;
	virtual void OnItemClick(SEItem* item) = 0;
};

//**********************************************************************
//	class SlotsEngine
class SlotsEngine
{
	class Iterator;
private:
	std::map<int,SESlot*> m_Slots;
	Widget* m_Container;
	SEController* m_Controller;						//	����� � �������
	SEMigrationMgr* m_MMgr;							//	�����
public:
	SlotsEngine(Widget* container);
	virtual ~SlotsEngine();
public:
	//	������� ���� c �������������� ��������������� � ����������
	SESlot* InsertSlot(int id);
	//	������� ���� (������� ������ �� ������)
	bool RemoveSlot(int id);
	//	�������� ���� �� ��������������
	SESlot* SlotAt(int id);
public:	
	//	������� ��������� �����
	SEItem* CreateItem(const char* system_name,const ipnt2_t& size,BaseThing* bs);
public:
	//	���������� ����������
	void SetController(SEController* controller);
	SEController* Controller(void) const {return m_Controller;}
	SEMigrationMgr* MigrationMgr(void) const {return m_MMgr;}
public:
	//	������ � ����������
	SlotsEngine::Iterator Begin(void);
	SlotsEngine::Iterator End(void);
#ifdef _DEBUG_SLOTS_ENGINE
public:
	void Render(void);
#endif
};

//************************************************************************//
//	class SlotsEngine::Iterator
class SlotsEngine::Iterator
{
private:
	std::map<int,SESlot*>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::map<int,SESlot*>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	SESlot* operator->(void);
	SESlot& operator*(void);
	bool operator!=(const Iterator& it);
};

inline SlotsEngine::Iterator& SlotsEngine::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline SlotsEngine::Iterator SlotsEngine::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline SESlot* SlotsEngine::Iterator::operator->(void)
{
	return m_it->second;
}

inline SESlot& SlotsEngine::Iterator::operator*(void)
{
	return *operator->();
}

inline bool SlotsEngine::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//**********************************************************************
//	class SlotsEngine
inline SlotsEngine::Iterator SlotsEngine::Begin(void)
{
	return Iterator(m_Slots.begin());
}

inline SlotsEngine::Iterator SlotsEngine::End(void)
{
	return Iterator(m_Slots.end());
}

//**********************************************************************
//	class SEFactory
class SEFactory
{
public:
	static const char* GenerateSlotName(void);
	static const char* GenerateItemName(void);
	static void GenerateShadow(DXImage* image);
	static void GenerateHighlight(DXImage* image);
	static void GenerateGrayed(DXImage* image);
	static void GenerateNorank(DXImage* image);
	static DXImage* DoHighlightedImage(DXImage* base_image);
	static DXImage* DoGrayedImage(DXImage* base_image);
	static DXImage* DoNorankImage(DXImage* base_image);
};

#endif	//_SLOTS_ENGINE_H_