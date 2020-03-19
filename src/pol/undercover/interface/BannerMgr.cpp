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
#include <common/Shell/Shell.h>
#include <common/GraphPipe/GraphPipe.h>
#include "ELs/elText.h"
#include "Banner.h"
#include "BannerMgr.h"

namespace BannerMgr
{
	//	������ ����� ��� ������������ �������
	enum {BANNERS_NUM = 128};
	point3 m_pPoints[BANNERS_NUM];
	elText* m_ppText[BANNERS_NUM];
	int m_iPointsSize = 0;	//	������� ������ ������� ��� m_pPoints � m_ppText
	std::list<Banner*> m_lBanners;
}

void BannerMgr::DrawBanners(void)
{
	//	�������������� 3D ����� � �������� ����������
	Shell::Instance()->GetGraphPipe()->TransformPoints2(m_iPointsSize,m_pPoints,m_pPoints);
	//	������������ �������
	for(int i=0;i<m_iPointsSize;i++)
	{
		if(m_pPoints[i].z != -1)
		{
			m_ppText[i]->MoveTo(ToVirtualX(m_pPoints[i].x)-(m_ppText[i]->Width()>>1),ToVirtualY(m_pPoints[i].y));
			m_ppText[i]->Render(Shell::Instance()->GetGraphPipe());
		}
	}
	m_iPointsSize = 0;
}

void BannerMgr::Tune(void)
{
	for(std::list<Banner*>::iterator i=m_lBanners.begin();i!=m_lBanners.end();i++)
		(*i)->Tune();
}

void BannerMgr::RegisterBanner(Banner *b)
{
	m_lBanners.push_back(b);
}

void BannerMgr::UnregisterBanner(Banner *b)
{
	std::list<Banner*>::iterator i = std::find(m_lBanners.begin(),m_lBanners.end(),b);
	if(i != m_lBanners.end())
		m_lBanners.erase(i);
}

void BannerMgr::AddPoint(const point3& pt,elText* pText)
{
	if(m_iPointsSize == BANNERS_NUM)
		throw CASUS("����� �� ������� �������.");
	m_pPoints[m_iPointsSize] = pt;
	m_ppText[m_iPointsSize] = pText;
	m_iPointsSize++;
}
