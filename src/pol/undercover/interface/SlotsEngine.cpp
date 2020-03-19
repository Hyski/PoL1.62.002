F/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.09.2001

************************************************************************/
#include "Precomp.h"
#include <common/UI/Widget.h>
#include <common/UI/Static.h>
#include "DXImageMgr.h"
#include "Text.h"
#include "SlotsEngine.h"
#include <common/DataMgr/TxtFile.h>
#include <common/D3DApp/Input/Input.h>

const char* g_InventoryPictures = "pictures/interface/inventory/";

//**********************************************************************
//	class SEFactory
const char* SEFactory::GenerateSlotName(void)
{
	static int slot_counter = 0;
	static char slot_name[50];
	
	sprintf(slot_name,"slotX_%d",slot_counter++);
	
	return slot_name;
}

const char* SEFactory::GenerateItemName(void)
{
	static int item_counter = 0;
	static char item_name[50];
	
	sprintf(item_name,"itemX_%d",item_counter++);
	
	return item_name;
}

void SEFactory::GenerateShadow(DXImage* image)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(image && (surface = image->m_pSurface))
	{
		DDSURFACEDESC2 ddsd;
		HDC hdc;
		
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		
		surface->GetSurfaceDesc(&ddsd);
		if(!FAILED(surface->GetDC(&hdc)))
		{
			for(int i=0;i<ddsd.dwHeight;i++)
			{
				for(int j=0;j<ddsd.dwWidth;j++)
				{
					if(GetPixel(hdc,j,i) != 0xffff00)
					{
						if((j&1) == (i&1)) SetPixel(hdc,j,i,0xffff00);
					}
				}
			}
			surface->ReleaseDC(hdc);
		}
	}
}

void SEFactory::GenerateHighlight(DXImage* image)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(image && (surface = image->m_pSurface))
	{
		DDSURFACEDESC2 ddsd;
		HDC hdc;
		
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		
		surface->GetSurfaceDesc(&ddsd);
		if(!FAILED(surface->GetDC(&hdc)))
		{
			for(int i=0;i<ddsd.dwHeight;i++)
			{
				for(int j=0;j<ddsd.dwWidth;j++)
				{
					if(GetPixel(hdc,j,i) != 0xffff00)
					{
						if((j&1) == (i&1)) SetPixel(hdc,j,i,0x00ffff);
					}
				}
			}
			
			surface->ReleaseDC(hdc);
		}
	}
}

void SEFactory::GenerateGrayed(DXImage* image)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(image && (surface = image->m_pSurface))
	{
		DDSURFACEDESC2 ddsd;
		HDC hdc;
		
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		
		surface->GetSurfaceDesc(&ddsd);
		if(!FAILED(surface->GetDC(&hdc)))
		{
			for(int i=0;i<ddsd.dwHeight;i++)
			{
				for(int j=0;j<ddsd.dwWidth;j++)
				{
					if(GetPixel(hdc,j,i) != 0xffff00)
					{
						if((j&1) == (i&1)) SetPixel(hdc,j,i,0xff);
					}
				}
			}
			
			surface->ReleaseDC(hdc);
		}
	}
}

//=====================================================================================//
//                          void SEFactory::GenerateNorank()                           //
//=====================================================================================//
void SEFactory::GenerateNorank(DXImage* image)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(image && (surface = image->m_pSurface))
	{
		DDSURFACEDESC2 ddsd;
		HDC hdc;
		
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		
		surface->GetSurfaceDesc(&ddsd);
		if(!FAILED(surface->GetDC(&hdc)))
		{
			for(int i=0;i<ddsd.dwHeight;i++)
			{
				for(int j=0;j<ddsd.dwWidth;j++)
				{
					if(((j&1) == (i&1)) && GetPixel(hdc,j,i) != 0xffff00)
					{
						SetPixel(hdc,j,i,0xffffff);
					}
				}
			}
			
			surface->ReleaseDC(hdc);
		}
	}
}

DXImage* SEFactory::DoHighlightedImage(DXImage* base_image)
{
	DXImage* result_image = 0;
	
	if(base_image)
	{
		std::string image_name = base_image->m_Resource + "_hl";
		
		if(!DXImageMgr::Instance()->IsImageCreated(image_name.c_str()))
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str(),base_image);
			GenerateHighlight(result_image);
		}
		else
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str());
		}
	}
	
	return result_image;
}

DXImage* SEFactory::DoGrayedImage(DXImage* base_image)
{
	DXImage* result_image = 0;
	
	if(base_image)
	{
		std::string image_name = base_image->m_Resource + "_grayed";
		
		if(!DXImageMgr::Instance()->IsImageCreated(image_name.c_str()))
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str(),base_image);
			GenerateGrayed(result_image);
		}
		else
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str());
		}
	}
	
	return result_image;
}

//=====================================================================================//
//                         DXImage* SEFactory::DoNorankImage()                         //
//=====================================================================================//
DXImage* SEFactory::DoNorankImage(DXImage* base_image)
{
	DXImage* result_image = 0;
	
	if(base_image)
	{
		std::string image_name = base_image->m_Resource + "_norank";
		
		if(!DXImageMgr::Instance()->IsImageCreated(image_name.c_str()))
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str(),base_image);
			GenerateNorank(result_image);
		}
		else
		{
			result_image = DXImageMgr::Instance()->CreateImage(image_name.c_str());
		}
	}
	
	return result_image;
}


//**********************************************************************
//	class SEItemView
const char* SEItemView::m_WidgetType = "item_as_widget";
const char* SEItemView::m_WrongImage = "pictures/interface/inventory/wrong_image.tga";
SEItemView::SEItemView(const char* name,const char* system_name,const ipnt2_t& size_in_cells) : Widget(name),
					   m_CurImage(I_NORMAL),m_IsDragged(false),m_Width(0),m_Height(0)
{
	DXImage* base_image;

	base_image = DXImageMgr::Instance()->CreateImage(std::string(std::string(g_InventoryPictures)+std::string(system_name)+".tga").c_str());
	DXImageMgr::Instance()->GetImageSize(base_image,&m_Width,&m_Height);

	if(m_Width && m_Height)
	{
		m_OffsetX = (size_in_cells.x*SESlot::m_CellWidth-m_Width)/2;
		m_OffsetY = (size_in_cells.y*SESlot::m_CellHeight-m_Height)/2;
	}
	else
	{//	�������� ���, ����������� ������������ ��������
		base_image->Release();
		base_image = DXImageMgr::Instance()->CreateImage(m_WrongImage);
		m_Width = size_in_cells.x*SESlot::m_CellWidth;
		m_Height = size_in_cells.y*SESlot::m_CellHeight;
		m_OffsetX = 0;
		m_OffsetY = 0;
	}

	GetLocation()->Create(0,0,m_Width,m_Height);
	//	�������������� ��������� ��������
	m_Images[I_NORMAL]		= base_image;
	m_Images[I_HIGHLIGHTED] = SEFactory::DoHighlightedImage(m_Images[I_NORMAL]);
	m_Images[I_GRAYED]		= SEFactory::DoGrayedImage(m_Images[I_NORMAL]);
	m_Images[I_NORANK]		= SEFactory::DoNorankImage(m_Images[I_NORMAL]);
	//	������������� ��������� ���������
	SetImage(m_Images[m_CurImage]);
	//	������� ��������� ����
	Insert(m_Text = new Text("text"));
	int left = 0;
	int top  = std::max(0,m_Height-SESlot::m_CellHeight*2);
	m_Text->SetRegion(left,top,m_Width,std::min(top+SESlot::m_CellHeight*2,m_Height));
	m_Text->Align(LogicalText::T_BOTTOM|LogicalText::T_RIGHT);
	m_Text->SetFont("oskar",0x00dd00);
	m_Text->Enable(false);
	//	----------------------------------
	GetClipper()->Activate(false);
}

SEItemView::~SEItemView()
{
	//	������� ��������
	SetImage(0);
	for(int i=0;i<MAX_IMAGE;i++)
	{
		if(m_Images[i]) m_Images[i]->Release();
	}
	//	������� ������ ���� �� ������������� �������
	if(Parent()) Parent()->Remove(Name());
}

void SEItemView::MoveTo(int x,int y)
{
	Widget::MoveTo(x+m_OffsetX,y+m_OffsetY);
}

void SEItemView::SetState(unsigned int state)
{
										m_CurImage = I_NORMAL;
	if(state&SEItem::ST_HIGHLIGHTED)	m_CurImage = I_HIGHLIGHTED;
	if(state&SEItem::ST_GRAYED)			m_CurImage = I_GRAYED;
	SetImage(m_Images[m_CurImage]);
}
//	�������� � ������ ��������������
void SEItemView::OnDrag(void)
{
	m_DragX = m_Width/2;
	m_DragY = m_Height/2;
	m_IsDragged = true;
	Enable(false);
}
//	�������� � ����� ��������������
void SEItemView::OnDrop(void)
{
	m_IsDragged = false;
	Enable(true);
}
//	���������� �����-�� ��������
bool SEItemView::Tick(void)
{
	if(m_IsDragged)
	{
		MoveTo(Input::MouseState().x-m_DragX-GetLocation()->OriginX(),
			   Input::MouseState().y-m_DragY-GetLocation()->OriginY());
	}

	return false;
}
//	�������� �������� �����������
WidgetImage* SEItemView::GetBaseImage(void) const
{
	return m_Images[I_NORMAL];
}
//	��������� ���������� ���������
void SEItemView::OnSystemChange(void)
{
	if(m_Images[I_NORMAL]) m_Images[I_NORMAL]->Reload();
	m_Images[I_HIGHLIGHTED] = SEFactory::DoHighlightedImage(m_Images[I_NORMAL]);
	m_Images[I_GRAYED]		= SEFactory::DoGrayedImage(m_Images[I_NORMAL]);
	m_Images[I_NORANK]		= SEFactory::DoNorankImage(m_Images[I_NORMAL]);
	SetImage(m_Images[m_CurImage]);
	//	���������� ������ �������
	m_Text->OnSystemChange();
}
//	���������� ��������� ���� �� �����
void SEItemView::SetText(const char* text)
{
	m_Text->SetText(text);
}

//**********************************************************************
//	class SEItem
const int SEItem::m_SlotIdIsAbsent = -1;
SEItem::SEItem(const char* system_name,const ipnt2_t& size,BaseThing* bs) : m_View(new SEItemView(SEFactory::GenerateItemName(),system_name,size)),
			   m_Size(size),m_BaseThing(bs),m_SlotId(m_SlotIdIsAbsent)
{
}

SEItem::~SEItem()
{
	//	������� ������� ��� �����
	delete m_View;
}

void SEItem::SetState(unsigned int state)
{
	m_View->SetState(state);
}
//	����������� ������� ��� ����� (�������� ����������)
void SEItem::MoveTo(Widget* container,int x,int y)
{
	if(container) container->Insert(m_View);
	if(m_View) m_View->MoveTo(x,y);
}
//	����������� ������� ��� ����� (�������� ����������)
void SEItem::ScrollTo(int x,int y)
{
	if(m_View) m_View->MoveTo(x,y);
}
//	���������� ������� ����� � ����� (���������� � ������� �����)
void SEItem::SetPosition(const ipnt2_t& pos)
{
	m_Position = pos;
}
//	���������������� �������
void SEItem::EnableClipper(bool enable)
{
	if(m_View) m_View->GetClipper()->Activate(enable);
}
//	�������� � ������ ��������������
void SEItem::OnDrag(Widget* new_container)
{
	if(m_View && m_View->Parent()) m_View->Parent()->Remove(m_View->Name());
	new_container->Insert(m_View);
	if(m_View) m_View->OnDrag();
}
//	�������� � ����� ��������������
void SEItem::OnDrop(void)
{
	if(m_View && m_View->Parent()) m_View->Parent()->Remove(m_View->Name());
	if(m_View) m_View->OnDrop();
}
//	�������� ��������
WidgetImage* SEItem::GetImage(void) const
{
	return m_View->GetBaseImage();
}
//	���������� ��������� ���� �� �����
void SEItem::SetText(const char* text)
{
	m_View->SetText(text);
}
//	�������� ��������� ���������� ����������� �����
ipnt2_t SEItem::GetScreenPlace(void) const
{
	int x = m_View->GetLocation()->OriginX()+m_View->GetLocation()->Region()->X();
	int y = m_View->GetLocation()->OriginY()+m_View->GetLocation()->Region()->Y();

	return ipnt2_t(x,y);
}
//**********************************************************************
//	class SEMigrationMgr
const char* SEMigrationMgr::m_ShadowImageName = "shadow_image_for_SE";
SEMigrationMgr::SEMigrationMgr(SlotsEngine* feed_back,Widget* container) : m_FeedBack(feed_back),m_Container(container),
				m_Item(0),m_Controller(0)
{
	m_Container->Insert(m_Shadow = new Widget("se_shadow"));
	m_Shadow->Enable(false);
	m_Shadow->SetVisible(false);
}

SEMigrationMgr::~SEMigrationMgr()
{
	if(m_Item) delete m_Item;
	//	������� ������ ����
	m_Shadow->Parent()->Remove(m_Shadow->Name());
	delete m_Shadow;
}

void SEMigrationMgr::Insert(SEItem* item)
{
	//	������������� �����
	if(m_Item) throw CASUS("SEMigrationMgr: ������� ��������� ���� � ������� MigrationMgr.");
	m_Item = item;
	//	������������� ����
	m_Shadow->Parent()->BringToTop(m_Shadow->Name());
	m_Shadow->GetLocation()->Create(0,0,m_Item->Size().x*SESlot::m_CellWidth,m_Item->Size().y*SESlot::m_CellHeight);
	CreateNewShadow(m_Item->GetImage(),m_Item->Size());
	//	�������� ��������� �����
	m_Item->OnDrag(m_Container);
	//	�������� ��������� �����������
	if(m_FeedBack->Controller()) m_FeedBack->Controller()->OnLighting(m_Item,SEController::LM_START);
	if(m_Controller) m_Controller->OnDrag();
}

SEItem* SEMigrationMgr::Remove(void)
{
	SEItem* item = m_Item;

	if(!m_Item) throw CASUS("SEMigrationMgr: ������� ������� �������������� ����.");
	//	���������� �������� � ����
	if(m_Shadow->GetImage())
	{
		m_Shadow->GetImage()->Release();
		m_Shadow->SetImage(0);
	}
	m_Item->OnDrop();
	m_Item = 0;
	//	�������� ��������� �����������
	if(m_FeedBack->Controller()) m_FeedBack->Controller()->OnLighting(item,SEController::LM_FINISH);
	if(m_Controller) m_Controller->OnDrop();

	return item;
}

SEItem* SEMigrationMgr::GetItem(void)
{
	return m_Item;
}
//	������������ ���� (�������� ����������)
void SEMigrationMgr::ShowShadow(bool enable,const ipnt2_t& xy)
{
	const int x = xy.x - (m_Shadow->Parent()->GetLocation()->OriginX()+m_Shadow->Parent()->GetLocation()->Region()->X());
	const int y = xy.y - (m_Shadow->Parent()->GetLocation()->OriginY()+m_Shadow->Parent()->GetLocation()->Region()->Y());

	m_Shadow->SetVisible(enable);
	m_Shadow->MoveTo(x+m_Offset.x,y+m_Offset.y);
}
//	������� ����� ����
void SEMigrationMgr::CreateNewShadow(WidgetImage* wi,const ipnt2_t& size_in_cells)
{
	if(wi)
	{	
		m_Shadow->SetImage(DXImageMgr::Instance()->CreateImage(m_ShadowImageName,static_cast<DXImage*>(wi)));
		if(m_Shadow->GetImage())
		{
			int width,height;

			SEFactory::GenerateShadow(static_cast<DXImage*>(m_Shadow->GetImage()));
			DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Shadow->GetImage()),&width,&height);
			m_Shadow->GetLocation()->Create(0,0,width,height);
			m_Offset.x = (size_in_cells.x*SESlot::m_CellWidth-width)/2;
			m_Offset.y = (size_in_cells.y*SESlot::m_CellHeight-height)/2;
		}
	}
}

void SEMigrationMgr::SetController(SEMigrationMgrController* controller)
{
	m_Controller = controller;
}

//**********************************************************************
//	class SESlotView
const char* SESlotView::m_WidgetType = "slot_as_widget";
SESlotView::SESlotView(Widget* container,SESlot* feed_back) : Widget(SEFactory::GenerateSlotName()),m_FeedBack(feed_back),
					   m_PrevEvent(SESlot::ET_NONE)
{
	container->Insert(this);
//	SetImage(DXImageMgr::Instance()->CreateImage("pictures/interface/slot_background.bmp"));
	container->Insert(m_SelectedImage = new Widget(std::string(std::string(Name())+"selected_image").c_str()));
	m_SelectedImage->Enable(false);
	m_SelectedImage->SetVisible(false);
}

SESlotView::~SESlotView()
{
	if(Parent())
	{
		Parent()->Remove(m_SelectedImage->Name());
		Parent()->Remove(Name());
	}
	//	���������� ������ ������ �������
	delete m_SelectedImage;
}
//	���������� �������� �� ���������
void SESlotView::SetSelectedImage(const char* image_name,int x,int y)
{
	int width,height;

	if(m_SelectedImage->GetImage())	m_SelectedImage->GetImage()->Release();
	m_SelectedImage->SetImage(DXImageMgr::Instance()->CreateImage(image_name));
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_SelectedImage->GetImage()),&width,&height);
	m_SelectedImage->GetLocation()->Create(x,y,x+width,y+height);
}
//	����������/����� ���������
void SESlotView::SetSelected(bool enable)
{
	m_SelectedImage->SetVisible(enable);
}
//	��������� ��������� �����
bool SESlotView::ProcessMouseInput(VMouseState* pMS)
{
	if(m_FeedBack->IsAccessibleCell(pMS->x,pMS->y))
	{
		if(!((pMS->dz != 0) && m_FeedBack->ProcessMouseWheel(pMS->dz)))
		{
								  m_PrevEvent = SESlot::ET_MOUSE_PRESENT;
			if(pMS->LButtonFront) m_PrevEvent = SESlot::ET_LEFT_CLICK;
			if(pMS->RButtonFront) m_PrevEvent = SESlot::ET_RIGHT_CLICK;
						  
			m_FeedBack->OnEvent(pMS->x,pMS->y,m_PrevEvent);
		}
	}
	else
	{
		if(m_PrevEvent != SESlot::ET_NONE)
		{
			m_FeedBack->OnEvent(pMS->x,pMS->y,SESlot::ET_MOUSE_LEAVE);
			m_PrevEvent = SESlot::ET_NONE;
		}
	}

	return true;
}
//	��������� ����-����
bool SESlotView::Tick(void)
{
	if(Parent()->IsVisible())
	{//	���� ��� ������ ���������� ������ ���� ������������ ������ �����
		if(m_PrevEvent != SESlot::ET_NONE)
		{
			const int x = Input::MouseState().x;
			const int y = Input::MouseState().y;
			
			VRegion region = *(GetLocation()->Region());
			
			region.Transform(1,0,GetLocation()->OriginX(),0,1,GetLocation()->OriginY());
			
			if(!region.PtInRegion(x,y))
			{
				m_FeedBack->OnEvent(x,y,SESlot::ET_MOUSE_LEAVE);
				m_PrevEvent = SESlot::ET_NONE;
			}
		}
	}

	return true;
}

//**********************************************************************
//	class SESlot
const int SESlot::m_CellWidth = 10;			//	������ ������ � ��������
const int SESlot::m_CellHeight = 10;		//	������ ������ � ��������
const float SESlot::m_DetonateTime = 2.f;	//	����� ���������
SESlot::SESlot(int id,Widget* container,SlotsEngine* feed_back) : m_Id(id),m_View(container?new SESlotView(container,this):0),
			   m_Placement(P_NORMAL_SLOT),m_Align(A_LEFT_TO_RIGHT),m_Mode(M_INSIDE_INTERFACE),
			   m_FeedBack(feed_back),m_ActiveItem(0),m_CellsOffset(0,0),m_ExpMode(EM_TO_RIGHT)
{
}

SESlot::~SESlot()
{
	//	���������� ��� �����, ����������� � �����
	for(std::list<SEItem*>::iterator i=m_Items.begin();i!=m_Items.end();i++)
		delete *i;
	//	���������� ������� ��� �������
	delete m_View;
}
//	���������� ������������ ����� (x,y - ���������� � ��������; width,height - � ������� �����)
void SESlot::SetLocation(int x,int y,int width,int height)
{
	m_X = x;	m_Width = width;
	m_Y = y;	m_Height = height;

	if(m_View)
	{
		m_View->GetLocation()->Create(m_X,m_Y,m_X+m_Width*m_CellWidth,m_Y+m_Height*m_CellHeight);
	}

	//	������������� ������
	m_Cells.clear();
	for(int i=0;i<height;i++)
	{
		m_Cells.push_back(std::vector<CELL_STATE>());
		for(int j=0;j<width;j++)
		{
			m_Cells[i].push_back(CS_EMPTY);
		}
	}
}
//	���������� �������� �� ���������
void SESlot::SetSelectedImage(const char* image_name,int x,int y)
{
	if(m_View) m_View->SetSelectedImage(image_name,x,y);
}

void SESlot::SetPlacement(PLACEMENT placement)
{
	m_Placement = placement;
}

void SESlot::SetAlign(ALIGN align)
{
	m_Align = align;
}

void SESlot::SetExpMode(EXPANSION_MODE em)
{
	m_ExpMode = em;
}
//	����� ������� ���� ������� ���������� ������� CanInsert() ��� ������� �����
//	� ������ ����� CanInsert ������������� ���������� � �����.
void SESlot::Insert(SEItem* item)
{
	//	������������� �������������� ����� �����
	item->m_SlotId = m_Id;
	//	������������� ������
	if(m_Placement == P_ONE_ITEM_IN_SLOT)
	{
		//	���������� ������� ��� �����
		item->MoveTo(m_View,
					 (m_View->GetLocation()->Region()->Width()-item->m_Size.x*m_CellWidth)/2,
					 (m_View->GetLocation()->Region()->Height()-item->m_Size.y*m_CellHeight)/2);
	}
	else
	{
		SetCells(item->m_Position.x,item->m_Position.y,item->m_Position.x+item->m_Size.x,item->m_Position.y+item->m_Size.y,CS_FILLED);
		//	���������� ������� ��� �����
		item->MoveTo(m_View,
					 (item->m_Position.x+m_CellsOffset.x)*m_CellWidth,
					 (item->m_Position.y+m_CellsOffset.y)*m_CellHeight);
		if(m_Placement == P_UNLIMITED) item->EnableClipper(true);
	}

	m_Items.push_back(item);
}

void SESlot::Swap(SEItem* item_bottom,SEItem* item_top)
{
	Remove(item_bottom);
	Insert(item_top);
}
//	������� ���� �� ����� (�� �� �� ������, ������ �� ������������)
SEItem* SESlot::Remove(SEItem* item)
{
	SetCells(item->m_Position.x,item->m_Position.y,
			 item->m_Position.x+item->m_Size.x,item->m_Position.y+item->m_Size.y,CS_EMPTY);
	for(std::list<SEItem*>::iterator i=m_Items.begin();i!=m_Items.end();i++)
	{
		if((*i) == item)
		{
			m_Items.erase(i);
			break;
		}
	}
	if(m_Placement == P_UNLIMITED) item->EnableClipper(false);
	if(item == m_ActiveItem) m_ActiveItem = 0;

	return item;
}
//	������� ��� ����� �� ����� � ������� ������� � ������
void SESlot::Clear(void)
{
	//	���������� ��� �����, ����������� � �����
	for(std::list<SEItem*>::iterator ii=m_Items.begin();ii!=m_Items.end();ii++)
		delete *ii;
	m_Items.clear();
	//	���� ���� ����������� ���������� ������ ������ 
	if(m_Placement == P_UNLIMITED) ReleaseRedundantCells();
	//	������� ��� ����������� ������
	for(int i=0;i<m_Cells.size();i++)
	{
		for(int j=0;j<m_Cells[i].size();j++)
		{
			if(m_Cells[i][j] == CS_FILLED) m_Cells[i][j] = CS_EMPTY;
		}
	}
	//	��� ��������� �����
	m_ActiveItem = 0;
}
//	���������� ������ ������ 
void SESlot::ReleaseRedundantCells(void)
{
	int size;
	
	if(size = Size().y)
	{//	������� ������
		int num_of_item_to_delete = size-m_Height;
		std::vector<std::vector<CELL_STATE> >::reverse_iterator ir = m_Cells.rbegin();
		
		while(num_of_item_to_delete && (ir!=m_Cells.rend()))
		{
			m_Cells.erase(ir.base());
			num_of_item_to_delete--;
		}
	}
	if(size = Size().x)
	{//	������� �������
		for(int j=0;j<Size().y;j++)
		{
			std::vector<CELL_STATE>::reverse_iterator iir = m_Cells[j].rbegin();
			int num_of_item_to_delete = size-m_Width;
			
			while(num_of_item_to_delete && (iir!=m_Cells[j].rend()))
			{
				m_Cells[j].erase(iir.base());
				num_of_item_to_delete--;
			}
		}
	}
}

bool SESlot::CanInsert(SEItem* item,const ipnt2_t* pos)
{
	bool result = false;

	switch(m_Mode)
	{
	case M_INSIDE_INTERFACE:
		switch(m_Placement)
		{
		case P_NORMAL_SLOT:
			result = GetPlace(item->m_Size,pos,&item->m_Position);
			break;
		case P_ONE_ITEM_IN_SLOT:
			if(!m_Items.size() && CanPlace(item))
			{
				item->m_Position = ipnt2_t(0,0);
				result = true;
			}
			break;
		case P_UNLIMITED:
			if(!(result = GetPlace(item->m_Size,0,&item->m_Position)))
			{//	��������� ������ � ������� ��� ���
				switch(m_ExpMode)
				{
				case EM_TO_RIGHT:
					{//	������
						for(int i=0;i<m_Cells.size();i++)
							for(int j=0;j<item->m_Size.x;j++)
								m_Cells[i].push_back(CS_EMPTY);
					}
					break;
				case EM_TO_DOWN:
					{//	����
						for(int i=0;i<item->m_Size.y;i++)
						{
							m_Cells.push_back(std::vector<CELL_STATE>());
							for(int j=0;j<m_Cells[0].size();j++)
								m_Cells[m_Cells.size()-1].push_back(CS_EMPTY);
						}
					}
					break;
				}
				result = GetPlace(item->m_Size,0,&item->m_Position);
			}
			break;
		}
		break;
	case M_OUTSIDE_INTERFACE:
		item->m_Position = ipnt2_t(0,0);
		result = true;
		break;

	}

	return result;
}

bool SESlot::CanSwap(SEItem* item_bottom,SEItem* item_top)
{
	bool result;

	Remove(item_bottom);
	result = CanInsert(item_top,&item_bottom->m_Position);
	Insert(item_bottom);
	
	return result;
}
//	���������� ��������� ������������� ������� �����
void SESlot::SetCells(int left,int top,int right,int bottom,CELL_STATE state)
{
	for(;left<right;left++)
	{
		for(int i=top;i<bottom;i++)
		{
			m_Cells[i][left] = state;
		}
	}
}
//	��������� ��������� �����
bool SESlot::CheckCells(int left,int top,int right,int bottom,CELL_STATE state)
{
	for(;left<right;left++)
	{
		for(int i=top;i<bottom;i++)
		{
			if(m_Cells[i][left] != state) return false;
		}
	}

	return true;
}

void SESlot::SetInaccessibleCells(int left,int top,int right,int bottom)
{
	SetCells(left,top,right,bottom,CS_INACCESSIBLE);
}

bool SESlot::GetPlace(const ipnt2_t& size,const ipnt2_t* where,ipnt2_t* result)
{
	//	�������� �� ����������� ���������� � �������
	if(where)
	{//	���� ����������� ������� � ������������ �����
		return SwingOfPendulum(size,*where,result);
	}
	else
	{//	���� ����������� ������� � ����� ��������� ����� �������� ���������
		switch(m_Align)
		{
		case A_LEFT_TO_RIGHT:
			return LeftToRight(size,result);
		case A_RIGHT_TO_LEFT:
			return RightToLeft(size,result);
		}
	}

	return false;
}

//	���������� ����� �� �����
bool SESlot::LeftToRight(const ipnt2_t& size,ipnt2_t* result)
{
	const int width = std::max(0,(int)m_Cells[0].size()-size.x+1);
	const int height = std::max(0,(int)m_Cells.size()-size.y+1);
	int i,j;

	for(j=0;j<width;j++)
	{
		for(i=0;i<height;i++)
		{
			if(m_Cells[i][j] == CS_EMPTY)
			{
				if(CheckCells(j,i,j+size.x,i+size.y,CS_EMPTY))
				{
					result->x = j;
					result->y = i;
					
					return true;
				}
			}
		}
	}

	return false;
}

//	���������� ������ �� ����
bool SESlot::RightToLeft(const ipnt2_t& size,ipnt2_t* result)
{
	const int width = std::max(0,(int)m_Cells[0].size()-size.x);
	const int height = std::max(0,(int)m_Cells.size()-size.y+1);
	int i,j;

	for(j=width;j>=0;j--)
	{
		for(i=0;i<height;i++)
		{
			if(m_Cells[i][j] == CS_EMPTY)
			{
				if(CheckCells(j,i,j+size.x,i+size.y,CS_EMPTY))
				{
					result->x = j;
					result->y = i;
					
					return true;
				}
			}
		}
	}

	return false;
}
//	where - �������� ����������, � ������� ��������� ����� ���������������� �����
bool SESlot::SwingOfPendulum(const ipnt2_t& size,const ipnt2_t& where,ipnt2_t* result)
{
	const int width = std::max(0,(int)m_Cells[0].size()-size.x+1);
	const int height = std::max(0,(int)m_Cells.size()-size.y+1);
	const ipnt2_t slot_xy = ipnt2_t(std::min(std::max(0,where.x),(width-1)),
									std::min(std::max(0,where.y),(height-1)));
							
	int x = slot_xy.x;

	for(int j=0;j<(width*2);j++)
	{
		x += j*((j&1)?1:-1);

		int y = slot_xy.y;

		for(int i=0;i<(height*2);i++)
		{
			y += i*((i&1)?1:-1);

			if(IsAccessibleCell(ipnt2_t(x,y)) && IsAccessibleCell(ipnt2_t(x+size.x-1,y+size.y-1)))
			{
				if(CheckCells(x,y,x+size.x,y+size.y,CS_EMPTY))
				{
					result->x = x;
					result->y = y;

					return true;
				}
			}
		}
	}

	return false;
}

//	�������� �������
void SESlot::OnEvent(int x,int y,EVENT_TYPE type)
{
	//	�������� �����������
	if(m_FeedBack->Controller())
	{
		Widget* item_view = m_View->FirstChildFromPoint(x,y);
		SEItem* item = 0;
		ipnt2_t slot_xy = ScreenToSlot(x,y);

		//	���������� ����� ���� ��������� ��� ������
		if(item_view)
		{
			for(std::list<SEItem*>::iterator i=m_Items.begin();i!=m_Items.end();i++)
			{
				if((*i)->m_View == item_view)
				{
					if(m_Placement == P_ONE_ITEM_IN_SLOT)
						item = *i;
					else
						if(PtInItem(*i,slot_xy)) item = *i;
					break;
				}
			}
		}
		
		if(m_FeedBack->MigrationMgr()->GetItem())
		{//	����� ��������� ������ (DRAG'N'DROP)
			ipnt2_t item_xy = m_FeedBack->MigrationMgr()->GetItem()->GetScreenPlace();
			item_xy = ScreenToSlot(item_xy.x,item_xy.y);

			switch(type)
			{
			case ET_LEFT_CLICK:
				m_FeedBack->Controller()->OnDrop(this,item,item_xy);
				ShowShadow(false);
				break;
			case ET_MOUSE_PRESENT:
				ShowShadow(item?false:true,m_FeedBack->MigrationMgr()->GetItem(),item_xy.x,item_xy.y);
				break;
			case ET_MOUSE_LEAVE:
				ShowShadow(false);
				break;
			}
		}
		else
		{//	����� ������
			if(item)
			{//	����� ��������� �� �����
				switch(type)
				{
				case ET_MOUSE_PRESENT:
					if(m_ActiveItem)
					{
						if(m_ActiveItem != item)
						{
							if(IsDetonate())
								m_FeedBack->Controller()->OnLighting(m_ActiveItem,SEController::LM_FINISH);
							m_ActivateTime = Timer::GetSeconds();
							m_IsFirstTimeStart = true;
						}
						else
						{
							if(m_IsFirstTimeStart && IsDetonate())
							{
								m_FeedBack->Controller()->OnLighting(item,SEController::LM_START);
								m_IsFirstTimeStart = false;
							}
						}
					}
					else
					{//	����� ��������������� ����� ���
						m_ActivateTime = Timer::GetSeconds();
					}
					m_ActiveItem = item;
					break;
				case ET_LEFT_CLICK:
					m_FeedBack->Controller()->OnDrag(this,item);
					break;
				case ET_RIGHT_CLICK:
					m_FeedBack->Controller()->OnItemClick(item);
					break;
				}
			}
			else
			{
				if(m_ActiveItem)
				{
					if(IsDetonate())
						m_FeedBack->Controller()->OnLighting(m_ActiveItem,SEController::LM_FINISH);
					m_IsFirstTimeStart = true;
					m_ActiveItem = 0;
				}
			}
		}
	}
}
//	���������� ��������� 
void SESlot::SetState(STATE state)
{
	switch(state)
	{
	case ST_NORMAL:
		m_View->SetSelected(false);
		break;
	case ST_SELECTED:
		m_View->SetSelected(true);
		break;
	}
	m_State = state;
}
//	������, ���������� �� ���� � ���� � ��������
bool SESlot::CanPlace(SEItem* item)
{
	return (!(item->m_Size.y>m_Cells.size()) && 
		    !(item->m_Size.x>m_Cells[0].size()));
}
//	������, ���������� �� ���� � ���� � ��������
bool SESlot::CanPlace(const ipnt2_t& size)
{
	return (!(size.y>m_Cells.size()) && 
		    !(size.x>m_Cells[0].size()));
}
//	����� ������������� ������� OnHanging
bool SESlot::IsDetonate(void) const
{
	return (Timer::GetSeconds()-m_ActivateTime)>m_DetonateTime;
}
//	������� ��������� ��������� � ������ �����
ipnt2_t SESlot::ScreenToSlot(int x,int y)
{
	x -= (m_View->GetLocation()->OriginX()+m_View->GetLocation()->Region()->X());
	y -= (m_View->GetLocation()->OriginY()+m_View->GetLocation()->Region()->Y());

	return ipnt2_t(x/m_CellWidth,y/m_CellHeight);
}
//	������� � ��������� ����������
ipnt2_t SESlot::SlotToScreen(int x,int y)
{
	x *= m_CellWidth;
	y *= m_CellHeight;

	return ipnt2_t(x + (m_View->GetLocation()->OriginX()+m_View->GetLocation()->Region()->X()),
				   y + (m_View->GetLocation()->OriginY()+m_View->GetLocation()->Region()->Y()));
}
//	������ �������� �� ������ ��������� (�������� ����������)
bool SESlot::IsAccessibleCell(int x,int y)
{
	ipnt2_t cell_xy = ScreenToSlot(x,y);

	if((cell_xy.x>=0 && cell_xy.x<m_Cells[0].size()) &&
	   (cell_xy.y>=0 && cell_xy.y<m_Cells.size()))
	{
		return m_Cells[cell_xy.y][cell_xy.x] != CS_INACCESSIBLE;
	}

	return false;
/*
	//	�������� �� ����� �� �������
	cell_xy.x = std::max(0,std::min(cell_xy.x,(int)m_Cells[0].size()-1));
	cell_xy.y = std::max(0,std::min(cell_xy.y,(int)m_Cells.size()-1));

	return m_Cells[cell_xy.y][cell_xy.x] != CS_INACCESSIBLE;*/
}
//	������ �������� �� ������ ��������� (���������� � ������� �����)
bool SESlot::IsAccessibleCell(const ipnt2_t& cell_xy)
{
	if((cell_xy.x>=0 && cell_xy.x<m_Cells[0].size()) &&
	   (cell_xy.y>=0 && cell_xy.y<m_Cells.size()))
	{
		return m_Cells[cell_xy.y][cell_xy.x] != CS_INACCESSIBLE;
	}

	return false;
}
//	������ ��������� �� ����� � �����
bool SESlot::PtInItem(const SEItem* item,const ipnt2_t& slot_xy)
{
	const int left   = item->m_Position.x;
	const int top    = item->m_Position.y;
	const int right  = left+item->m_Size.x;
	const int bottom = top+item->m_Size.y;

	const int x = slot_xy.x-m_CellsOffset.x;
	const int y = slot_xy.y-m_CellsOffset.y;


	return (x>=left && x<=right) &&
		   (y>=top  && y<=bottom);
}
//	���������� �� ����
inline bool SESlot::IsShowShadow(void)
{
	return (m_Placement == P_NORMAL_SLOT) && (m_State == ST_SELECTED);
}
//	�������� ���� (�������� ����������)
void SESlot::ShowShadow(bool enable,SEItem* item_for,int x,int y)
{
	if(IsShowShadow())
	{
		if(enable)
		{
			if(CanInsert(item_for,&ipnt2_t(x,y)))
				m_FeedBack->MigrationMgr()->ShowShadow(enable,SlotToScreen(item_for->m_Position.x,item_for->m_Position.y));
			else
				m_FeedBack->MigrationMgr()->ShowShadow(false,ipnt2_t(0,0));
		}
		else
		{
			m_FeedBack->MigrationMgr()->ShowShadow(enable,ipnt2_t(0,0));
		}
	}
	else
	{
		m_FeedBack->MigrationMgr()->ShowShadow(false,ipnt2_t(0,0));
	}
}
//	�������� ���������� ������ �� ��������� ���-�� �����
void SESlot::Scroll(int dx,int dy)
{
	m_CellsOffset.x = std::min(0,m_CellsOffset.x+dx);
	m_CellsOffset.y = std::min(0,m_CellsOffset.y+dy);
	//	����������� � ������ ���������� �������� �����
	m_CellsOffset.x = std::max(m_CellsOffset.x,m_Width-Size().x);
	m_CellsOffset.y = std::max(m_CellsOffset.y,m_Height-Size().y);
	//	����������� ����������� ������
	for(std::list<SEItem*>::iterator i=m_Items.begin();i!=m_Items.end();i++)
	{
		(*i)->ScrollTo(((*i)->m_Position.x+m_CellsOffset.x)*m_CellWidth,
					   ((*i)->m_Position.y+m_CellsOffset.y)*m_CellHeight);
	}
}
//	�������� ���������� ������ � ������
void SESlot::UnScroll(void)
{
	Scroll(-m_CellsOffset.x,-m_CellsOffset.y);
}
//	�������� ���������� ������� �����
ipnt2_t SESlot::Size(void) const
{
	ipnt2_t size(0,0);

	if(m_Cells.size() && m_Cells[0].size())
	{
		size.x = m_Cells[0].size();
		size.y = m_Cells.size();
	}

	return size;
}
//	���������� �������� ��������
bool SESlot::ProcessMouseWheel(float dz)
{
	if(m_Placement == P_UNLIMITED)
	{
		switch(m_ExpMode)
		{
		case EM_TO_RIGHT:
			Scroll(((int)dz/40),0);
			return true;
		case EM_TO_DOWN:
			Scroll(0,((int)dz/40));
			return true;
		}
	}

	return false;
}

#ifdef _DEBUG_SLOTS_ENGINE
void SESlot::Render(HDC hdc)
{
	const int x = m_View->GetLocation()->OriginX()+m_X;
	const int y = m_View->GetLocation()->OriginY()+m_Y;

	for(int i=0;i<m_Cells.size();i++)
	{
		for(int j=0;j<m_Cells[i].size();j++)
		{
			const int lx = x+(j+m_CellsOffset.x)*m_CellWidth;
			const int ly = y+(i+m_CellsOffset.y)*m_CellHeight;

			switch(m_Cells[i][j])
			{
			case CS_EMPTY:
				continue;
			case CS_FILLED:
				SelectObject(hdc,GetStockObject(NULL_BRUSH));
				SelectObject(hdc,GetStockObject(WHITE_PEN));
				Rectangle(hdc,lx+4,ly+4,lx+6,ly+6);
				break;
			case CS_INACCESSIBLE:
				SelectObject(hdc,GetStockObject(BLACK_BRUSH));
				SelectObject(hdc,GetStockObject(NULL_PEN));
				Rectangle(hdc,lx,ly,lx+m_CellWidth,ly+m_CellHeight);
				break;
			}
		}
	}
/*	if(m_State == ST_SELECTED)
	{
		HPEN pen = CreatePen(PS_SOLID,2,0xfff00);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		SelectObject(hdc,pen);
		Rectangle(hdc,x+10,y+10,x+m_Width*m_CellWidth-10,y+m_Height*m_CellHeight-10);
		SelectObject(hdc,GetStockObject(NULL_PEN));
		DeleteObject(pen);
	}*/
}
#endif


//**********************************************************************
//	class SlotsEngine
SlotsEngine::SlotsEngine(Widget* container) : m_Container(container),m_Controller(0),
											  m_MMgr(new SEMigrationMgr(this,container))
{
#ifdef _DEBUG_SLOTS_ENGINE
//	m_Controller = new NhtTestController(this);
#endif
}

SlotsEngine::~SlotsEngine()
{
#ifdef _DEBUG_SLOTS_ENGINE
//	delete m_Controller;
#endif
	//	���������� MigrationMgr
	delete m_MMgr;
	//	���������� �����
	for(std::map<int,SESlot*>::iterator i=m_Slots.begin();i!=m_Slots.end();i++)
		delete i->second;
}
//	���������� ����������
void SlotsEngine::SetController(SEController* controller)
{
	m_Controller = controller;
}
//	������� ���� c �������������� ���������������
SESlot* SlotsEngine::InsertSlot(int id)
{
	std::map<int,SESlot*>::iterator i = m_Slots.find(id);
	SESlot* slot = 0;

	if(i == m_Slots.end())
		m_Slots.insert(std::map<int,SESlot*>::value_type(id,slot = new SESlot(id,m_Container,this)));

	return slot;
}
//	������� ���� (������� ������ �� ������)
bool SlotsEngine::RemoveSlot(int id)
{
	std::map<int,SESlot*>::iterator i = m_Slots.find(id);

	if(i != m_Slots.end())
	{
		delete i->second;
		m_Slots.erase(i);

		return true;
	}

	return false;
}
//	�������� ���� �� ��������������
SESlot* SlotsEngine::SlotAt(int id)
{
	std::map<int,SESlot*>::iterator i = m_Slots.find(id);

	if(i != m_Slots.end()) return i->second;

	return 0;
}

SEItem* SlotsEngine::CreateItem(const char* system_name,const ipnt2_t& size,BaseThing* bs)
{
	return new SEItem(system_name,size,bs);
}

#ifdef _DEBUG_SLOTS_ENGINE
void SlotsEngine::Render(void)
{
	HDC hdc;

	D3DKernel::GetBB()->GetDC(&hdc);


	for(std::map<int,SESlot*>::iterator i=m_Slots.begin();i!=m_Slots.end();i++)
	{
		i->second->Render(hdc);
	}

	D3DKernel::GetBB()->ReleaseDC(hdc);
}
#endif

//=====================================================================================//
//                                 void SESlot::Init()                                 //
//=====================================================================================//
void SESlot::Init(SESlot *slot, int i, const TxtFile &script)
{
	int index = i+1;

	int x = atoi(script(index,1).c_str());
	int y = atoi(script(index,2).c_str());
	
	int width = atoi(script(index,3).c_str());
	int height = atoi(script(index,4).c_str());
	
	slot->SetLocation(x,y,width,height);

	if(atoi(script(index,5).c_str())) slot->SetPlacement(SESlot::P_ONE_ITEM_IN_SLOT);
	slot->SetAlign(atoi(script(index,6).c_str())?SESlot::A_LEFT_TO_RIGHT:SESlot::A_RIGHT_TO_LEFT);
	if(atoi(script(index,7).c_str())) slot->SetPlacement(SESlot::P_UNLIMITED);
	//	��������� ���������� � ��������� ���������
	slot->SetSelectedImage(script(index,8).c_str(),atoi(script(index,9).c_str()),atoi(script(index,10).c_str()));
	//	���������� � ������ ����������
	std::string em = script(index,11);
	if(em == "to_right") slot->SetExpMode(SESlot::EM_TO_RIGHT);
	if(em == "to_down") slot->SetExpMode(SESlot::EM_TO_DOWN);
	//	������������� ������� ��������, ������� ������ ������
	int num_of_always_busy = atoi(script(index,12).c_str());
	int left,top,right,bottom;
	for(int j=0;j<num_of_always_busy;j++)
	{
		if(sscanf((char *)script(index,13+j).c_str(),"%d,%d,%d,%d",&left,&top,&right,&bottom) == 4)
		{
			slot->SetInaccessibleCells(left,top,right,bottom);
		}
	}
}
