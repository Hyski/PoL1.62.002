/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   01.08.2000

************************************************************************/
#ifndef _ZIP_INFO_H_
#define _ZIP_INFO_H_

namespace ZipInfo
{
	//	�������� ������� zip ����� - szZipName
	void ListZip(const char *szZipName,std::vector<std::string>& vStr);
	//	�������� ���������� �����, ��������� � pData, ��� ����� ���������� ����������
	bool LoadFile(const char *szZipName,
				  const char *szFileName,
				  unsigned char **pData,
				  unsigned int *pSize);
	//	����������� ���� �� ����
	bool UnzipFile(const char *szZipName,
				   const char *szFileName);
}


#endif	//_ZIP_INFO_H_