/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: 
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   18.05.2000

************************************************************************/
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class XRegistry;

namespace Options
{
/*
	������ ������ ���������� �� ����:
[�����������]<��� ����������>[�����������]<"><��������><">

*/
	void Init(void);
	void Close(void);
	void Flush(bool bUseFlag = true);	//	bUseFlag == false - ������ ���������� ����
	//	������� ��������������� ����������
	void Set(const char *sName,int iValue);
	void Set(const char *sName,float fValue);
	void Set(const char *sName,const char *sValue);
	//	������� ��������� �������� ����������
	int GetInt(const char *sName);
	float GetFloat(const char *sName);
	const std::string& GetString(const char *sName);

	//	������ � ��������
	XRegistry* Registry(void);
}

#endif	//_OPTIONS_H_