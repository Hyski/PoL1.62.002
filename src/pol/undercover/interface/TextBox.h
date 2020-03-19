/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#ifndef _TEXT_BOX_H_
#define _TEXT_BOX_H_

#include "Text.h"

class Static;

class TextBox : public Text
{
	class Strategy;
	class NormalStrategy;
	class ConsoleStrategy;
	friend NormalStrategy;
	friend ConsoleStrategy;

private:
	bool m_dirty;

	static const float m_Delay;		//	����� ��������
	Static* m_pUp;
	Static* m_pDown;
	int m_ScrollStep;			//	��� �������� ������
	int m_TextHeight;			//	������ ������
	int m_TextOffset;			//	�������� ������ ������
	Strategy* m_Strategy;		//	��������� ���������
	std::string m_text;			//  ����� ����

public:
	TextBox(const char *pName);
	virtual ~TextBox();
public:
	//	������������� 
	struct ScrollTraits
	{
		const char* m_UpImageName;
		const char* m_DownImageName;
		int m_Width;
		int m_Height;
	};
	virtual void SetVisible(bool b);
	virtual void SetScroll(const ScrollTraits& st);
	virtual void SetRegion(int left,int top,int right,int bottom);
	//	���������� �����
	virtual void SetFont(const char* name,int color);
	//	���������� �����
	virtual void SetText(const char* text);
	virtual void AddText(const char* text);
	virtual void Clear(void);
	//	���������� ����� 
	enum SCROLL_DIRECTION {SD_UP,SD_DOWN,SD_BEGIN,SD_END};
	//	sd: ��� ���������� check_time: ��������� �����
	virtual void Scroll(SCROLL_DIRECTION sd,bool check_time = false);
	virtual void Scroll(int pixel_count);
	virtual void ScrollTo(int offset);
	//	��������� �� �����
	virtual bool IsScrollEnd(void) const;
	//	���������� ����� ��-���������� ������ ������
	enum MODE {M_NORMAL,M_CONSOLE};
	virtual void SetMode(MODE mode);
	//	��������� ������
	virtual void ToEnd(void);
private:
	//	��� ����������
	SCROLL_DIRECTION m_PrevSD;	//	���������� ��������� ��� ����������
	float m_LastScrollSeconds;	//	����� ������� ������
protected:
	virtual void OnSystemChange(void);
	//	��������� �� ��������� ����
	virtual void OnChange(Widget* pChild);
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ������ �������� �����
	virtual bool ProcessMouseLost(void);
	//	��������� ������ ������������� �����
	virtual bool ProcessKeyboardLost(void);
	//	������� ���� ���� ���������
	virtual bool Tick(void);
public:
	virtual const char* Type(void) const {return "textbox";};
private:
	void ApplyText();
	//	�������� �����������
	void UpdateSurface(void);
};

//**********************************************************************
//	class TextBox::Strategy
class TextBox::Strategy
{
protected:
	TextBox* m_FeedBack;
public:
	Strategy(TextBox* feed_back) : m_FeedBack(feed_back) {}
	virtual ~Strategy() {}
	virtual void UpdateSurface(void) = 0;
	virtual void SetText(const char* text) = 0;
	virtual void AddText(const char* text) = 0;
	virtual void Clear(void) = 0;
	virtual void Tick(void) = 0;
	virtual void FontChanged(void) = 0;
	virtual void OnSystemChange(void) = 0;
	virtual void OnScroll(int pixels) = 0;
	virtual void ToEnd(void) = 0;
};

#endif	//_TEXT_BOX_H_