/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.01.2001

************************************************************************/
#include "Precomp.h"
#include "ELs/elText.h"
#include "Banner.h"

Banner::Banner()
{
	m_pText = new elText("Banner");
	m_pText->SetFont("tip");
	m_pText->SetTextHAlign(elText::THA_LEFT);
	m_pText->SetTextVAlign(elText::TVA_TOP);
	m_pText->m_zBegin = m_pText->m_zEnd = 0.001f;
	//	������������ ������
	BannerMgr::RegisterBanner(this);
}

Banner::~Banner()
{
	//	��������������� ������
	BannerMgr::UnregisterBanner(this);
	//	���������� ������� ����������� ������
	delete m_pText;
}

void Banner::Tune(void)
{
	m_pText->Tune();
}

void Banner::SetText(const char *pText)
{
	SIZE sz;

	//	����������� ������������� ������� ������
	FontMgr::Instance()->GetGFont(m_pText->GetFontName().c_str())->GetSize(pText,&sz);
	m_pText->SetRect(0,0,sz.cx+32,sz.cy*2);
	//	--------------------
	m_pText->SetText(pText);
	m_pText->ResolveText();
	m_pText->Tune();
}

void Banner::SetColor(unsigned long ulTextColor)
{
	m_pText->SetTextColor(ulTextColor);
	m_pText->TextBlit();
}
