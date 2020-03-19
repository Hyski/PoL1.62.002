/**************************************************************************

								MiST land Font

                          Copyright by MiST land 2000
  -------------------------------------------------------------------------
  Description: ������ ������, ������������� MiST land �� ������ ������ ���
               �������� 3D ���.
  -------------------------------------------------------------------------
  ��������� �����:
  0x00000000				MLF_FILEHEADER
							lSize = sizeof(MLF_FILEHEADER)	- ������������ 
							���	�������� ������.
							sID = "MiST land Font\0" - ������������ ��� 
							�������������.
							Version - ������ ������ 
										Version[0] - ����� (1)
										Version[1] - ����� (0)
  +sizeof(MLF_FILEHEADER)   MLF_DATAHEADER
							lSize = sizeof(MLF_DATAHEADER)	- ������������ 
							���	�������� ������.
							lFontHeight - ������ ������� ������.
                            rcChar[256] - ������, ��� �������� ���������� 
							��� ������ ����� ������.
  +sizeof(MLF_DATAHEADER)   ������� ����� ������ � ������� 8 ��� �� ������.
  -------------------------------------------------------------------------
  Version: 1.00
  Coder: Naughty
  Date:  13.06.2000

***************************************************************************/
#ifndef _MLF_H_
#define _MLF_H_

#pragma pack(push,1)

struct MLF_RECT
{
	int left,top,right,bottom;
};

struct MLF_ABC
{
	int a;
	unsigned int b;
	int c;
};

struct MLF_FILEHEADER
{
	long lSize;					//	������ ��������� - �������� ������
	char sID[15];				//	������������� == "MiST land Font\0"
	unsigned char Version[2];	//	������ ������ ������
};

struct MLF_DATAHEADER
{
	long lSize;					//	������ ��������� - �������� ������
	long lFontHeight;			//	������ ������
	MLF_RECT rcChar[256];		//	���������� ������ �����
	MLF_ABC abcChar[256];		//	abc ��� �������
	long lWidth;				//	������ �������� ������ � ��������
	long lHeight;				//	������ �������� ������ � ��������
};

#pragma pack(pop)

#endif	//_MLF_H_