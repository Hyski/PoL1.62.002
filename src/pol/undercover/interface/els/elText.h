/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   21.08.2000

************************************************************************/
#ifndef _EL_TEXT_H_
#define _EL_TEXT_H_

#include "../Lagoon.h"
#include <Common/FontMgr/FormattedText.h>
#include <Common/TextureMgr/D3DTexture.h>
#include <Common/FontMgr/VirtualTexture.h>
#include <Common/FontMgr/FontMgr.h>
#include <Common/FontMgr/GFont.h>

class elText : public Lagoon
{
public:
	enum TEXT_HALIGN	{THA_LEFT,
						 THA_RIGHT,
						 THA_CENTER,
						 THA_WIDE
	                    };
	enum TEXT_VALIGN	{TVA_TOP,
						 TVA_BOTTOM,
						 TVA_CENTER
	                    };
protected:
	unsigned int m_uiHAlign;				//	����� ������������ ������
	unsigned int m_uiVAlign;				//	����� ������������ ������
protected:
	//	��� ������ �������
	D3DTLVERTEX m_d3dTLVertex[4];		//	������������
	D3DTexture m_d3dTexture;			//	�������� ��� �����
	VirtualTexture m_vTexture;			//	����������� ��������
	DWORD m_dwTextColor;				//	���� ������
protected:
	std::string m_sText;				//	��������� �����
	FormattedText m_fText;				//	��������������� �����
	std::string m_sFont;				//	��� ������
	bool m_bTextResolved;				//	���� 
public:
	int m_iOffsetY;						//  �������� �� Y
public:
	elText(const char *pName);
	virtual ~elText();
protected:
	virtual void CreateTexture(void);
public:
	void SetFont(const char *pFont);
	void SetText(const char *pText);
	void SetTextColor(DWORD dwTextColor);
	void SetTextHAlign(TEXT_HALIGN ha);
	void SetTextVAlign(TEXT_VALIGN va);
	void AddText(const char *pText);
public:
	unsigned int GetHeight(void);			//	�������� ������ ���������������� ������ (����������� ����������)
public:
	//	������� ��������� �������� (����������� ����������)
//	virtual void SetRect(RECT *pRect);
//	virtual void SetRect(int left,int top,int right,int bottom);
	virtual void SetRegion(HRGN hRegion);
	//	������� ����������
	virtual void Tune(void);
	//	������� �����������
	virtual void Render(GraphPipe *lpGraphPipe);
	//	������� ��������� �������������� (����������� ����������)
	virtual void MoveTo(int x,int y);
public:
	void TextBlit(void);
	void ResolveText(void);
	void ClearSurface(void);
public:
	std::string& GetFontName(void);
	std::string& GetText(void);
	unsigned int GetStringNum(void);
};

inline std::string& elText::GetFontName(void)
{
	return m_sFont;
}

//	�������� ������ ���������������� ������ (����������� ����������)
inline unsigned int elText::GetHeight(void)
{
	return m_fText.m_vFS.size()*FontMgr::Instance()->GetGFont(m_sFont.c_str())->Height();
}

inline unsigned int elText::GetStringNum(void)
{
	return m_fText.m_vFS.size();
}

inline std::string& elText::GetText(void)
{
	return m_sText;
}

#endif	//_EL_TEXT_H_