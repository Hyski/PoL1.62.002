/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   04.06.2000

************************************************************************/
#include "Precomp.h"
#include "TxtFile.h"

//---------- ��� ���� ------------
#ifdef _NO_DEBUG_SHELL
CLog tfile;
#define tfile tfile["txtfile.log"]
#else
#define tfile /##/
#endif
//--------------------------------

TxtFile::TxtFile()
{
}

TxtFile::TxtFile(VFile *pVFile)
{
	Load(pVFile);
}

TxtFile::~TxtFile()
{
}

bool TxtFile::Load(VFile *pVFile)
{
	std::vector<std::string> vString;
	std::string str;
	std::string cell;
	bool bEOS = true;

	//	���������� ��� �����
	if(pVFile->Name()) m_FileName = pVFile->Name();
	//	����������� ��������� ������
	m_vData.clear();
	vString.clear();
	//	��������� �� ������������� �����
	if(pVFile->Size() != 0)
	{//	���� �� ������
		tfile("file: [%s]\n",pVFile->Name());
		pVFile->Rewind();
		while(GetCell(pVFile,cell,&bEOS))
		{
			tfile("[%s]\n",cell.c_str());
			vString.push_back(cell);
			if(bEOS)
			{
				m_vData.push_back(vString);
				tfile("-----------------------------\n");
				vString.clear();
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool TxtFile::GetCell(VFile *pVFile,std::string& str,bool *pbEOS)
{
	bool bComplex = false;

	//	������� ������
	str.clear();
	if((pVFile->Tell()!=pVFile->Size()))
	{
		*pbEOS = false;
		if(pVFile->Data()[pVFile->Tell()] == '\"')
		{//	������ � ��������, ������������� "\r\n, �� �� ""\r\n
			//	���������� ��
			pVFile->ReadChar();
			bComplex = true;
		}
		if((pVFile->Tell()!=pVFile->Size()))
		{
			while((pVFile->Tell()!=pVFile->Size()))
			{
				//	�������� "" ��� ����� ������
				if(pVFile->Data()[pVFile->Tell()] == '\"')
				{
					//	���������� ��
					pVFile->ReadChar();
					if((pVFile->Tell()!=pVFile->Size()))
					{
						if(pVFile->Data()[pVFile->Tell()] != '\"')
						{
							if((pVFile->Data()[pVFile->Tell()] == '\n') ||
							   (pVFile->Data()[pVFile->Tell()] == '\r'))
							{
								bComplex = false;
							}
							continue;
						}
					}
				}
				//	����� ������
				if(pVFile->Data()[pVFile->Tell()] == '\n')
				{
					if(!bComplex)
					{
						*pbEOS = true;
						break;
					}
				}
				//	���� ��������� - ������ ��������� ���� ������
				if(pVFile->Data()[pVFile->Tell()] == '\t')
				{
					break;
				}
				//	���������� ���� \r
				if(pVFile->Data()[pVFile->Tell()] == '\r')
				{
					pVFile->ReadChar();
					continue;
				}
				str.push_back(pVFile->ReadChar());
			}
		}
		//	��������� ��������� �� ��������� ������
		pVFile->ReadChar();
		return true;
	}

	return false;
}

void TxtFile::GetCell(unsigned int i,unsigned int j,std::string& cell)
{
	if(i>=m_vData.size())
		throw CASUS(m_FileName+std::string(": ����� �� ������� ������� �����. \n"));
	if(j>=m_vData[i].size())
		throw CASUS(m_FileName+std::string(": ����� �� ������� �������. \n"));
	cell = m_vData[i][j];
}

//=====================================================================================//
//                        void TxtFile::GetCellOrEmptyString()                         //
//=====================================================================================//
void TxtFile::GetCellOrEmptyString(unsigned int i,unsigned int j,std::string& cell)
{
	if(i < m_vData.size() && j < m_vData[i].size())
	{
		cell = m_vData[i][j];
	}
	else
	{
		cell.clear();
	}
}

const std::string& TxtFile::operator()(unsigned int i,unsigned int j) const
{
	if(i>=m_vData.size())
		throw CASUS(m_FileName+std::string(": ����� �� ������� ������� �����. \n"));
	if(j>=m_vData[i].size())
		throw CASUS(m_FileName+std::string(": ����� �� ������� �������. \n"));
	return m_vData[i][j];
}

//	������ ������ �� ������ ������, ������ ������ ������� �������� pFind
bool TxtFile::Find(const char *pFind,std::string& cell)
{
	if(pFind)
	{
		for(unsigned int i=0;i<m_vData.size();i++)
		{
			if(m_vData[i].size()>1)
			{
				if(strcmp(m_vData[i][0].c_str(),pFind)==0)
				{
					cell = m_vData[i][1];
					return true;
				}
			}
		}
	}

	return false;
}
//	������ ���������� ������ ������ � ������� j, � ������� ������� ������� ������ pFind
bool TxtFile::FindInCol(const char *pFind,unsigned int *i,unsigned int j)
{
	if(pFind)
	{
		for(unsigned int ic=0;ic<m_vData.size();ic++)
		{
			if(strcmp(m_vData[ic][j].c_str(),pFind)==0)
			{
				*i = ic;
				return true;
			}
		}
	}

	return false;
}
//	������ ���������� ������ ������ � ������ i, � ������� ������� ������� ������ pFind
bool TxtFile::FindInRow(const char *pFind,unsigned int i,unsigned int *j)
{
	if(pFind)
	{
		for(unsigned int jc=0;jc<m_vData[i].size();jc++)
		{
			if(strcmp(m_vData[i][jc].c_str(),pFind)==0)
			{
				*j = jc;
				return true;
			}
		}
	}

	return false;
}

