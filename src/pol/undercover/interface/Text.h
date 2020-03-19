/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#ifndef _TEXT_H_
#define _TEXT_H_

#include <common/UI/Widget.h>
#include <common/FontMgr/LogicalText.h>

class Text : public Widget
{
private:
	int m_ImageWidth;
	int m_ImageHeight;
protected:
	LogicalText m_Text;
	int m_Aling;
	std::string m_FontName;
	int m_FontColor;
public:
	Text(const char *pName);
	virtual ~Text();
public:
	//	������ ������������� ������ ������ (new_image - ��������� ����� �������� � ����� ������)
	virtual void SetRegion(int left,int top,int right,int bottom,bool new_image = false);
	//	���������� �����
	virtual void SetFont(const char* name,int color);
	//	����������� �����
	virtual void SetText(const char* text);
	//	�������� �����
	virtual const std::string& GetText(void);
	//	�������� ���������� (�����������) �����
	virtual const LogicalText& GetLogicalText(void) const;
	//	�������� ����
	virtual const Font* GetFont(void);
	//	�������� ������������ �����
	virtual const char* GetFontName(void) const;
	//	�������� ���� ������
	virtual int GetFontColor(void) const;
	//	��������� �����
	virtual void Align(int align);
	//	�������� �������� ������������ ������
	virtual int GetAlign(void) const;
	//	�������� ����� (�������� ���� ��������� �����)
	virtual void Update(void);
	//	�������� ������ ������
	int Width(void) const;
	//	�������� ������ ������
	int Height(void) const;
public:
	virtual void OnSystemChange(void);
protected:
	void ClearSurface(LPDIRECTDRAWSURFACE7 surface);
public:
	virtual const char* Type(void) const {return "text";};
};

//	�������� ������ ������
inline int Text::Height(void) const
{
	return m_Text.Height();
}
//	�������� ������ ������
inline int Text::Width(void) const
{
	return m_Text.Width();
}
//	�������� �����
inline const std::string& Text::GetText(void)
{
	return m_Text.SourceText();
}
//	�������� ���������� (�����������) �����
inline const LogicalText& Text::GetLogicalText(void) const
{
	return m_Text;
}
//	�������� ������������ �����
inline const char* Text::GetFontName(void) const
{
	return m_FontName.c_str();
}
//	�������� ���� ������
inline int Text::GetFontColor(void) const
{
	return m_FontColor;
}
//	�������� �������� ������������ ������
inline int Text::GetAlign(void) const
{
	return m_Aling;
}

#endif	//_TEXT_H_