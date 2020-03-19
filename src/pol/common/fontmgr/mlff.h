/**************************************************************************

							 MiST land Fast Font

                          Copyright by MiST land 2000
  -------------------------------------------------------------------------
  Description: 
  -------------------------------------------------------------------------
  ��������� �����:
  0x00000000				MLFF_FILEHEADER
							lSize = sizeof(MLF_FILEHEADER)	- ������������ 
							���	�������� ������.
							sID = "MiST land Fast Font\0" - ������������ ��� 
							�������������.
							Version - ������ ������ 
										Version[0] - ����� (1)
										Version[1] - ����� (0)
  +sizeof(MLFF_FILEHEADER)   MLF_DATAHEADER
							lSize = sizeof(MLF_DATAHEADER)	- ������������ 
							���	�������� ������.
							lFontHeight - ������ ������� ������.
                            rcChar[96] - ������, ��� �������� ���������� 
							��� ������ ����� ������.
  +sizeof(MLFF_DATAHEADER)   ������� ����� ������ � ������� 8 ��� �� ������.
  -------------------------------------------------------------------------
  Version: 1.00
  Coder: Naughty
  Date:  09.10.2000

***************************************************************************/
#ifndef _MLFF_H_
#define _MLFF_H_

struct MLFF_RECT
{
	int left,top,right,bottom;
};

struct MLFF_ABC
{
	int a;
	unsigned int b;
	int c;
};

struct MLFF_FILEHEADER
{
	long lSize;					//	������ ��������� - �������� ������
	char sID[20];				//	������������� == "MiST land Fast Font\0"
	unsigned char Version[2];	//	������ ������ ������
};

struct MLFF_DATAHEADER
{
	long lSize;					//	������ ��������� - �������� ������
	long lFontHeight;			//	������ ������
	MLFF_RECT rcChar[96];		//	���������� ������ �����
	MLFF_ABC abcChar[96];		//	abc ��� �������
	long lWidth;				//	������ �������� ������ � ��������
	long lHeight;				//	������ �������� ������ � ��������
};

#endif	//_MLFF_H_