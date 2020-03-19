/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#include "Precomp.h"
#include <common/FontMgr/FontMgr.h>
#include <common/FontMgr/Font2D.h>
#include <common/UI/Static.h>
#include "DXImageMgr.h"
#include "TextBox.h"

//**********************************************************************
//	class TextBox::NormalStrategy
class TextBox::NormalStrategy : public TextBox::Strategy
{
public:
	NormalStrategy(TextBox* feed_back);
	virtual ~NormalStrategy();

public:
	void UpdateSurface(void);
	void SetText(const char* text);
	void AddText(const char* text);
	void Clear(void);
	void Tick(void);
	void FontChanged(void);
	void OnSystemChange(void);
	void OnScroll(int /*pixels*/){}
	void ToEnd(void){}
};

//**********************************************************************
//	class TextBox::ConsoleStrategy
class TextBox::ConsoleStrategy : public TextBox::Strategy
{
	class Caret;
private:
	int m_End;					//	��������� ������ ������
	bool m_TextAdded;			//	��������������� ����� ��������
	bool m_TextDrawing;			//	����� ���������� ����������
	const float m_SymbolTime;	//	���-�� ���� ��������� � �������
	float m_PointTime;			//	���������� �����
	Caret* m_Caret;				//	�������


	int m_xCaret;				//	���������� ��� �������
	int m_yCaret;				//	���������� ��� �������
	int m_yCaretOffset;			//	�������� ������� �� ���������
public:
	ConsoleStrategy(TextBox* feed_back);
	virtual ~ConsoleStrategy();
public:
	void UpdateSurface(void);
	void SetText(const char* text);
	void AddText(const char* text);
	void Clear(void);
	void Tick(void);
	void FontChanged(void);
	void OnSystemChange(void);
	void OnScroll(int pixels);
	void ToEnd(void);
};

//**********************************************************************
//	class TextBox
const float TextBox::m_Delay = 0.09f;

//=====================================================================================//
//                                 TextBox::TextBox()                                  //
//=====================================================================================//
TextBox::TextBox(const char *pName)
:	Text(pName),
	m_Strategy(new NormalStrategy(this)),
	m_dirty(false)
{
	m_pUp = 0;
	m_pDown = 0;
	m_ScrollStep = 0;
	m_TextHeight = 0;
	m_TextOffset = 0;
	//	--------------------------------------------
	m_PrevSD = SD_BEGIN;
	m_LastScrollSeconds = 0;
}

//=====================================================================================//
//                                 TextBox::~TextBox()                                 //
//=====================================================================================//
TextBox::~TextBox()
{
	delete m_Strategy;
}

//=====================================================================================//
//                             void TextBox::SetVisible()                              //
//=====================================================================================//
void TextBox::SetVisible(bool b)
{
	Text::SetVisible(b);

	if(b)
	{
		if(m_dirty)
		{
			const bool needScrollEnd = IsScrollEnd();
			m_Strategy->SetText(m_text.c_str());
			m_dirty = false;

			if(needScrollEnd) Scroll(SD_END);
		}
	}
}

//=====================================================================================//
//                              void TextBox::SetRegion()                              //
//=====================================================================================//
void TextBox::SetRegion(int left,int top,int right,int bottom)
{
	int max_offset = (std::max(0,m_TextHeight-(bottom-top)));

	if((-m_TextOffset)>max_offset)
		m_TextOffset = -max_offset;
	//	-----------------------------------
	Text::SetRegion(left,top,right,bottom);
	//	-----------------------------------
	if(m_pUp && m_pDown)
	{
		m_pUp->GetLocation()->Create(
			GetLocation()->Region()->Width()-m_pUp->GetLocation()->Region()->Width(),0,
			GetLocation()->Region()->Width(),m_pUp->GetLocation()->Region()->Height());
		m_pDown->GetLocation()->Create(
			GetLocation()->Region()->Width()-m_pDown->GetLocation()->Region()->Width(),GetLocation()->Region()->Height()-m_pDown->GetLocation()->Region()->Height(),
			GetLocation()->Region()->Width(),GetLocation()->Region()->Height());
	}
}

//=====================================================================================//
//                              void TextBox::SetScroll()                              //
//=====================================================================================//
void TextBox::SetScroll(const ScrollTraits& st)
{
	if(m_pUp) Delete(m_pUp->Name());
	if(m_pDown) Delete(m_pDown->Name());
	Insert(m_pUp = new Static("up"));
	Insert(m_pDown = new Static("down"));
	//	��������� ���������
	m_pUp->GetLocation()->Create(
		GetLocation()->Region()->Width()-st.m_Width,0,
		GetLocation()->Region()->Width(),st.m_Height);
	m_pDown->GetLocation()->Create(
		GetLocation()->Region()->Width()-st.m_Width,GetLocation()->Region()->Height()-st.m_Height,
		GetLocation()->Region()->Width(),GetLocation()->Region()->Height());
	//	��������� ��������
	m_pUp->AddImage(DXImageMgr::Instance()->CreateImage(st.m_UpImageName));
	m_pUp->SetState(0);
	m_pDown->AddImage(DXImageMgr::Instance()->CreateImage(st.m_DownImageName));
	m_pDown->SetState(0);
}

//=====================================================================================//
//                               void TextBox::SetText()                               //
//=====================================================================================//
//	���������� �����
void TextBox::SetText(const char* text)
{
	if(IsVisible())
	{
		m_dirty = false;
		m_Strategy->SetText(text);
		Scroll(m_PrevSD);
	}
	else
	{
		m_text = text;
		m_dirty = true;
	}
}

//=====================================================================================//
//                               void TextBox::AddText()                               //
//=====================================================================================//
void TextBox::AddText(const char* text)
{
	m_Strategy->AddText(text);
}

//=====================================================================================//
//                                void TextBox::Clear()                                //
//=====================================================================================//
void TextBox::Clear(void)
{
	m_Strategy->Clear();
}

//=====================================================================================//
//                               void TextBox::Scroll()                                //
//=====================================================================================//
//	���������� �����
void TextBox::Scroll(SCROLL_DIRECTION sd,bool check_time)
{
	if(!IsVisible())
	{
		m_PrevSD = sd;
		m_dirty = true;
		return;
	}

	if(!check_time || ((m_PrevSD != sd) || ((m_PrevSD == sd) && (Timer::GetSeconds()-m_LastScrollSeconds)>m_Delay)))
	{//	��������� ���������� ��� ������ ����� �� �����������: ������ ���
		const int prev_offset = m_TextOffset;

		switch(sd)
		{
		case SD_UP:
			m_TextOffset += std::min(m_ScrollStep,-m_TextOffset);
			break;
		case SD_DOWN:
			m_TextOffset -= std::max(0,std::min(m_ScrollStep,m_TextHeight-GetLocation()->Region()->Height()+m_TextOffset));
			break;
		case SD_BEGIN:
			m_TextOffset = 0;
			break;
		case SD_END:
			m_TextOffset = -(std::max(0,m_TextHeight-GetLocation()->Region()->Height()));
			break;
		}
		
		m_Strategy->OnScroll(prev_offset-m_TextOffset);

		UpdateSurface();
		m_LastScrollSeconds = Timer::GetSeconds();
	}
	m_PrevSD = sd;
}

//=====================================================================================//
//                               void TextBox::Scroll()                                //
//=====================================================================================//
void TextBox::Scroll(int pixel_count)
{
	const int prev_offset = m_TextOffset;

	if(pixel_count>0)
		m_TextOffset += std::min(pixel_count,-m_TextOffset);
	else
		m_TextOffset -= std::max(0,std::min(-pixel_count,m_TextHeight-GetLocation()->Region()->Height()+m_TextOffset));

	m_Strategy->OnScroll(prev_offset-m_TextOffset);

	UpdateSurface();
}

//=====================================================================================//
//                              void TextBox::ScrollTo()                               //
//=====================================================================================//
void TextBox::ScrollTo(int offset)
{
	const int prev_offset = m_TextOffset;

	if(offset>0)
		m_TextOffset = std::min(offset,0);
	else
		m_TextOffset = -std::max(0,std::min(-offset,m_TextHeight-GetLocation()->Region()->Height()));

	if(prev_offset != m_TextOffset)	UpdateSurface();
}

//=====================================================================================//
//                          bool TextBox::IsScrollEnd() const                          //
//=====================================================================================//
//	��������� �� �����
bool TextBox::IsScrollEnd(void) const
{
	return m_TextOffset == -(std::max(0,m_TextHeight-GetLocation()->Region()->Height()));
}

//=====================================================================================//
//                               void TextBox::SetFont()                               //
//=====================================================================================//
//	���������� �����
void TextBox::SetFont(const char* name,int color)
{
	Text::SetFont(name,color);
	m_ScrollStep = FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->Height();
	m_Strategy->FontChanged();
}

//=====================================================================================//
//                              void TextBox::OnChange()                               //
//=====================================================================================//
//	��������� �� ��������� ����
void TextBox::OnChange(Widget* pChild)
{
	if(((Static* )pChild)->LastEvent() == Static::LE_LBUTTONDOWN || ((Static* )pChild)->LastEvent() == Static::LE_LBUTTONPUSHED)
	{
		if(pChild == m_pUp)
		{
			Scroll(SD_UP,true);
			return;
		}
		if(pChild == m_pDown)
		{
			Scroll(SD_DOWN,true);
			return;
		}
	}
}
//	�������� �����������
void TextBox::UpdateSurface(void)
{
	if(IsVisible())
	{
		m_Strategy->UpdateSurface();
		m_dirty = false;
	}
	else
	{
		m_dirty = true;
	}
}

//	��������� ��������� �����
bool TextBox::ProcessMouseInput(VMouseState* pMS)
{
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
			Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);
	//	������������ �������� �������� � ��������� ���������
	if(pMS->dz != 0) Scroll(((int)pMS->dz/40)*m_ScrollStep);

	return true;
}
//	��������� ������������� �����
bool TextBox::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	return true;
}
//	��������� ������ �������� �����
bool TextBox::ProcessMouseLost(void)
{
	return true;
}
//	��������� ������ ������������� �����
bool TextBox::ProcessKeyboardLost(void)
{
	return true;
}

void TextBox::OnSystemChange(void)
{
	Text::OnSystemChange();
	//	----------------------------------------------------------------------------
	if(m_pUp) m_pUp->OnSystemChange();
	if(m_pDown) m_pDown->OnSystemChange();
	//	----------------------------------------------------------------------------
	m_Strategy->OnSystemChange();
}
//	������� ���� ���� ���������
bool TextBox::Tick(void)
{
	m_Strategy->Tick();

	return true;
}
//	���������� ����� ������
void TextBox::SetMode(MODE mode)
{
	delete m_Strategy;
	switch(mode)
	{
	case M_NORMAL:
		m_Strategy = new NormalStrategy(this);
		break;
	case M_CONSOLE:
		m_Strategy = new ConsoleStrategy(this);
		break;
	}
}
//	��������� ������
void TextBox::ToEnd(void)
{
	m_Strategy->ToEnd();
}
//**********************************************************************
//	class TextBox::NormalStrategy
TextBox::NormalStrategy::NormalStrategy(TextBox* feed_back) : Strategy(feed_back)
{
}

TextBox::NormalStrategy::~NormalStrategy()
{
}

//=====================================================================================//
//                    void TextBox::NormalStrategy::UpdateSurface()                    //
//=====================================================================================//
void TextBox::NormalStrategy::UpdateSurface(void)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(m_FeedBack->GetImage() && (surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		//	������� �����������
		m_FeedBack->ClearSurface(surface);
		//	������ ���� �����
		FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->TextOut(surface,m_FeedBack->m_TextOffset,m_FeedBack->GetLocation()->Region()->Width(),m_FeedBack->GetLocation()->Region()->Height(),&m_FeedBack->m_Text);
	}
}

//=====================================================================================//
//                       void TextBox::NormalStrategy::SetText()                       //
//=====================================================================================//
void TextBox::NormalStrategy::SetText(const char* text)
{
	LPDIRECTDRAWSURFACE7 surface;

	m_FeedBack->m_Text.Resolve(text,
							   FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor),
							   m_FeedBack->m_Aling,
							   m_FeedBack->GetLocation()->Region()->Width()-(m_FeedBack->m_pUp?m_FeedBack->m_pUp->GetLocation()->Region()->Width():0),
							   m_FeedBack->GetLocation()->Region()->Height());
	if(m_FeedBack->GetImage() && (surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		//	������� �����������
		m_FeedBack->ClearSurface(surface);
		//	������ ���� �����
		FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->TextOut(surface,m_FeedBack->m_TextOffset,m_FeedBack->GetLocation()->Region()->Width(),m_FeedBack->GetLocation()->Region()->Height(),&m_FeedBack->m_Text);
		//	���������� ������ ������
		m_FeedBack->m_TextHeight = m_FeedBack->m_Text.Height();
	}
}

//=====================================================================================//
//                       void TextBox::NormalStrategy::AddText()                       //
//=====================================================================================//
void TextBox::NormalStrategy::AddText(const char* text)
{
	m_FeedBack->SetText(std::string(m_FeedBack->m_Text.SourceText()+text).c_str());
}

void TextBox::NormalStrategy::Clear(void)
{
	LPDIRECTDRAWSURFACE7 surface;

	if(m_FeedBack->GetImage() && (surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		m_FeedBack->m_Text.Clear();
		//	������� �����������
		m_FeedBack->ClearSurface(surface);
	}
}

void TextBox::NormalStrategy::Tick(void)
{
}

void TextBox::NormalStrategy::FontChanged(void)
{
}

void TextBox::NormalStrategy::OnSystemChange(void)
{
}

//**********************************************************************
//	class TextBox::ConsoleStrategy::Caret
class TextBox::ConsoleStrategy::Caret : public Text
{
private:
	const float m_BlinkTime;
	float m_PointTime;
public:
	Caret() : Text("caret"),m_PointTime(0),m_BlinkTime(0.3f) {}
	virtual ~Caret() {}
public:
	void Blink(void)
	{
		if((Timer::GetSeconds()-m_PointTime)>m_BlinkTime)
		{
			SetVisible(!IsVisible());
			m_PointTime = Timer::GetSeconds();
		}
	}
};

//**********************************************************************
//	class TextBox::ConsoleStrategy
TextBox::ConsoleStrategy::ConsoleStrategy(TextBox* feed_back) : Strategy(feed_back),
								m_SymbolTime(20.f),m_PointTime(0),
								m_xCaret(0),m_yCaret(0),m_yCaretOffset(0)
{
	m_End = 0;
	m_TextAdded = false;
	m_TextDrawing = false;
	//	------------------------------------------------------
	m_FeedBack->Insert(m_Caret = new Caret());
	FontChanged();
}

//=====================================================================================//
//                    TextBox::ConsoleStrategy::~ConsoleStrategy()                     //
//=====================================================================================//
TextBox::ConsoleStrategy::~ConsoleStrategy()
{
	m_FeedBack->Delete(m_Caret->Name());
}

//=====================================================================================//
//                   void TextBox::ConsoleStrategy::UpdateSurface()                    //
//=====================================================================================//
void TextBox::ConsoleStrategy::UpdateSurface(void)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(m_FeedBack->GetImage() && (surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		//	������� �����������
		m_FeedBack->ClearSurface(surface);
		//	������ ���� �����
		int x,y;
		FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->TextOut(surface,
					  m_FeedBack->m_TextOffset,
					  m_FeedBack->GetLocation()->Region()->Width(),
					  m_FeedBack->GetLocation()->Region()->Height(),
					  &m_FeedBack->m_Text,0,m_End,&x,&y);
	}
}

//=====================================================================================//
//                      void TextBox::ConsoleStrategy::SetText()                       //
//=====================================================================================//
void TextBox::ConsoleStrategy::SetText(const char* text)
{
	LPDIRECTDRAWSURFACE7 surface;

	m_FeedBack->m_Text.Resolve(text,
							   FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor),
							   m_FeedBack->m_Aling,
							   m_FeedBack->GetLocation()->Region()->Width()-(m_FeedBack->m_pUp?m_FeedBack->m_pUp->GetLocation()->Region()->Width():0),
							   m_FeedBack->GetLocation()->Region()->Height());
	if(m_FeedBack->GetImage() && (surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		if(!m_TextAdded)
		{
			//	������� �����������
			m_FeedBack->ClearSurface(surface);
			//	���������� ��� ���������
			m_End = 0;
			m_TextDrawing = true;
		}
		else
		{
			m_TextAdded = false;
		}
		//	���������� ������ ������
		m_FeedBack->m_TextHeight = m_FeedBack->m_Text.Height();
	}
}

void TextBox::ConsoleStrategy::AddText(const char* text)
{
	m_TextAdded = true;
	SetText(std::string(m_FeedBack->m_Text.SourceText()+text).c_str());
}

void TextBox::ConsoleStrategy::Clear(void)
{
}

//=====================================================================================//
//                        void TextBox::ConsoleStrategy::Tick()                        //
//=====================================================================================//
void TextBox::ConsoleStrategy::Tick(void)
{
	LPDIRECTDRAWSURFACE7 surface;

	if((surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		if(m_TextDrawing)
		{
			if(!m_PointTime) m_PointTime = Timer::GetSeconds();
			
			int num = (Timer::GetSeconds()-m_PointTime)*m_SymbolTime;
			
			if(num)
			{
				//	������ �����
				FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->TextOut(surface,
					m_FeedBack->m_TextOffset,
					m_FeedBack->GetLocation()->Region()->Width(),
					m_FeedBack->GetLocation()->Region()->Height(),
					&m_FeedBack->m_Text,m_End,m_End+num,&m_xCaret,&m_yCaret);
				m_End += num;
				if(m_End >= m_FeedBack->m_Text.SymbolCount())
				{
					m_TextDrawing = false;
					m_PointTime = 0;
				}
				else
				{
					m_PointTime = Timer::GetSeconds();
				}
				const int font_height = FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->Height();
				//	��������� ���������� ������ ������ �� ����� �� ������� ���������
				if(m_yCaret > (m_FeedBack->GetLocation()->Region()->Height()-font_height))
				{
					const int difference = m_yCaret-(m_FeedBack->GetLocation()->Region()->Height()-font_height)+font_height;
					m_FeedBack->Scroll(-difference);
					m_xCaret = 0;

				}
				//	����������� ������
				m_Caret->MoveTo(m_xCaret,m_yCaret);
				m_Caret->SetVisible((m_yCaret>=0));
			}
		}
		else
		{//	������� �������
			m_Caret->Blink();
		}
	}
}

//=====================================================================================//
//                    void TextBox::ConsoleStrategy::FontChanged()                     //
//=====================================================================================//
void TextBox::ConsoleStrategy::FontChanged(void)
{
	int width,height;

	FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->GetSize('_',&width,&height);
	m_Caret->SetRegion(0,0,width,height);
	m_Caret->SetFont(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor);
	m_Caret->SetText("_");
}

//=====================================================================================//
//                   void TextBox::ConsoleStrategy::OnSystemChange()                   //
//=====================================================================================//
void TextBox::ConsoleStrategy::OnSystemChange(void)
{
	m_Caret->OnSystemChange();
}

//=====================================================================================//
//                      void TextBox::ConsoleStrategy::OnScroll()                      //
//=====================================================================================//
void TextBox::ConsoleStrategy::OnScroll(int pixels)
{
	m_yCaret -= pixels;
	m_Caret->MoveTo(m_xCaret,m_yCaret);
}

//=====================================================================================//
//                       void TextBox::ConsoleStrategy::ToEnd()                        //
//=====================================================================================//
void TextBox::ConsoleStrategy::ToEnd(void)
{
	LPDIRECTDRAWSURFACE7 surface;

	if((surface = ((DXImage *)m_FeedBack->GetImage())->m_pSurface))
	{
		m_FeedBack->Scroll(TextBox::SD_END);
		//	������ �����
		m_End = m_FeedBack->m_Text.SymbolCount();
		FontMgr::Instance()->GetFont2D(m_FeedBack->m_FontName.c_str(),m_FeedBack->m_FontColor)->TextOut(surface,
									   m_FeedBack->m_TextOffset,
									   m_FeedBack->GetLocation()->Region()->Width(),
									   m_FeedBack->GetLocation()->Region()->Height(),
									   &m_FeedBack->m_Text,0,m_End,&m_xCaret,&m_yCaret);
		m_TextDrawing = false;
		m_PointTime = 0;
		m_Caret->MoveTo(m_xCaret,m_yCaret);
		m_Caret->SetVisible(true);
	}
}
