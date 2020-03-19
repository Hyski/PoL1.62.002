/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#include "Precomp.h"
#include "Font.h"
#include "LogicalText.h"

//---------- ��� ���� ------------
#ifdef _NOT_LOG
CLog lt_log;
#define lt	lt_log["lt.log"]
#else
#define lt	/##/
#endif
//--------------------------------

LogicalText::LogicalText() : m_TextWidth(0),m_TextHeight(0)
{
}

LogicalText::~LogicalText()
{
}

/******************************************************************************
	�� ����� ����������� ������
	 - ���� tAlign == CENTER,LEFT,RIGHT, �� AddSpace == 0 ������;
	 - ���� tAlign == WIDE, �� AddSpace �������������� ������ �� width_limit;
	 - ���� tAlign == CENTER,RIGHT, �� Offset ��������������.
*******************************************************************************/
void LogicalText::Resolve(const char *pText,
						  const Font* pFont,
						  unsigned int tAlign,
						  unsigned int width_limit,
						  unsigned int height_limit)
{
	LogicalString ls;
	int len = strlen(pText);

	lt(" --- RESOLVE TEXT ---\n");
	m_TextWidth = 0;
	if(pText && pFont)
	{
		//	���������� �����
		m_SourceText = pText;
		lt("    ...text: [%s]\n\n$\n\n",pText);
		//	������� ������
		m_Text.clear();
		//	��������� �����
		for(int i=0;i<len;i++)
		{
			i += GetString(pText+i,pFont,width_limit,&ls);			
			//	�������� m_AddSpace
			if(!(tAlign&T_WIDE)) ls.m_AddSpace = 0;
			//	������������ �������� �� ����
			if(tAlign&T_RIGHT)
				ls.m_Offset = width_limit - ls.m_Width;
			if(tAlign&T_HCENTER)
				ls.m_Offset = (width_limit - ls.m_Width)/2;
			if(ls.m_EOS)
			{
				//	����������� ��������� �������
				if(tAlign&T_WIDE) ls.m_AddSpace = 0;
			}
			//	��������� 
			m_Text.push_back(ls);
			m_TextWidth = std::max(m_TextWidth,static_cast<int>(ls.m_Width));
		}
		//	����������� ��������� �������
		if(tAlign&T_WIDE && m_Text.size())
			m_Text[m_Text.size()-1].m_AddSpace = 0;
		//	����������� ������� �� ������
		m_TopOffset = 0;
		m_TextHeight = pFont->Height()*m_Text.size();
		if(!(m_TextHeight>height_limit))
		{
			if(tAlign & T_BOTTOM)
				m_TopOffset = height_limit-m_TextHeight;
			if(tAlign & T_VCENTER)
				m_TopOffset = (height_limit-m_TextHeight)/2;
		}
		//	������������ ���-�� �������� � ��������������� ������
		m_SymbolCount = 0;
		for(int i=(m_Text.size()-1);i>=0;i--)
		{
			m_Text[i].m_Symbols = 0;
			for(int j=(m_Text[i].m_vWords.size()-1);j>=0;j--)
			{
				m_SymbolCount += m_Text[i].m_vWords[j].size()+1;
				m_Text[i].m_Symbols += m_Text[i].m_vWords[j].size()+1;
			}
			m_SymbolCount--;
			m_Text[i].m_Symbols--;
		}
	}
}

int LogicalText::GetString(const char *pText,const Font* pFont,unsigned int width_limit,LogicalString *pLS)
{
	int len = strlen(pText);
	std::string word;
	int space_width,space_height;
	int cx,cy;
	int string_width = 0;

	lt("     ...resolve string:\n");
	//	������� ������
	pLS->m_vWords.clear();
	pLS->m_AddSpace = 0;
	pLS->m_Offset = 0;
	pLS->m_Width = 0;
	pLS->m_EOS = false;
	//	������� ������ ������� ������
	pFont->GetSize(' ',&space_width,&space_height);
	string_width = -space_width;
	//	��������� �����
	int i=0;
	for(;i<len;i++)
	{
		//	�������� �����
		i += GetWord(pText+i,word);
		if(word.size())
		{
			//	�������� ������ ����� � �������� ������ 
			pFont->GetSize(word.c_str(),&cx,&cy);
			lt("     - word length: %d of [%s];\n",cx,word.c_str());
			//	�������� ����� ������ ������
			string_width += cx+space_width;
			//	��������� �� ����� �� �������
			if(string_width > width_limit)
			{//	������� �� �������
				i -= (word.size()+1);
				string_width -= (cx+space_width);
				break;
			}
			//	��������� ����� � ������
			pLS->m_vWords.push_back(word);
		}
		//	��������� �� ����� ������
		if(pText[i] == '\n')
		{//	����� ������
			lt("     - the end of string;\n");
			pLS->m_EOS = true;
			break;
		}
	}
	//	���� ���� ����� ������ ���� � ������
	if(!pLS->m_vWords.size())
	{
		i += word.size();
		pLS->m_vWords.push_back(word);
	}
	//	��������� ���������� � ������
	if((width_limit>string_width) && (pLS->m_vWords.size()>1))
		pLS->m_AddSpace = (width_limit-string_width)/(pLS->m_vWords.size()-1);
	//	���������� ������ ������
	pLS->m_Width = std::min(string_width,(int)width_limit);

	return i;
}

int LogicalText::GetWord(const char *pText,std::string& word)
{
	int len = strlen(pText);
	bool bSymbolFind = false;

	word.clear();
	int i=0;
	for(;i<len;i++)
	{
		if(pText[i] == '\n') break;
		if(pText[i] == ' ' || pText[i] == '\t')
		{
			if(bSymbolFind)	break;
			word += ' ';
		}
		else
		{
			word += pText[i];
			bSymbolFind = true;
		}
	}
	lt("      - find: [%s]\n",word.c_str());

	return i;
}

void LogicalText::Clear(void)
{
	m_Text.clear();
}
