/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#ifndef _LOGICAL_TEXT_H_
#define _LOGICAL_TEXT_H_

class Font;

class LogicalText
{
public:
	enum TEXT_ALIGN 
	{
		T_TOP			= 0x00,				
		T_BOTTOM		= 0x01,
		T_LEFT			= 0x00,
		T_RIGHT			= 0x02,
		T_HCENTER		= 0x04,
		T_VCENTER		= 0x08,
		T_WIDE			= 0x10
	};
public:
	struct LogicalString
	{
		std::vector<std::string> m_vWords;		//	������ ����
		unsigned int m_AddSpace;				//	�������������� ���������� ����� �������
		unsigned int m_Offset;					//	�������� �� ������
		unsigned int m_Width;					//	������ ������ ���� �������
		unsigned int m_Symbols;					//	���-�� �������� � ������ (������� �������)
		bool m_EOS;								//	����� ������ (������)
	};
private:
	std::vector<LogicalString> m_Text;			//	������ �����
	std::string m_SourceText;					//	�������� �����
	int m_TopOffset;							//	������ ������ �� ������
	int m_TextWidth;							//	������ ������ ������
	int m_TextHeight;							//	������ ������ ������
	int m_SymbolCount;							//	���-�� �������� � ����������� ������
public:
	LogicalText();
	virtual ~LogicalText();
public:
	void Resolve(const char *pText,
				 const Font* pFont,
				 unsigned int tAlign,
				 unsigned int width_limit,
				 unsigned int height_limit);
	void Clear(void);
	const std::vector<LogicalString>& Text(void) const {return m_Text;}
	int Height(void) const {return m_TextHeight;}
	int Width(void) const {return m_TextWidth;}
	int TopOffset(void) const {return m_TopOffset;}
	int SymbolCount(void) const {return m_SymbolCount;}
	const std::string& SourceText(void) const {return m_SourceText;}
private:
	int GetString(const char *pText,const Font* pFont,unsigned int width_limit,LogicalString *pLS);
	int GetWord(const char *pText,std::string& word);

};

#endif	//_LOGICAL_TEXT_H_