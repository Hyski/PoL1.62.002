/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.07.2000

************************************************************************/
#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

//	������������ ������������� ����������
#define		VENDOR_INTEL	"GenuineIntel"
#define		VENDOR_AMD		"AuthenticAMD"
#define		VENDOR_CYRIX	"CyrixInstead"

//	������������ ���������� �� Intel
#define		INTEL_486SL		"486(TM) SL processor"
#define		INTEL_DX2WB		"Write-Back Enhanced DX2(TM) processor"
#define		INTEL_DX4		"DX4(TM) processor"
#define		INTEL_DX4O		"DX4(TM) OverDrive(R) processor"
#define		INTEL_P			"Pentium(R) processor"
#define		INTEL_PO		"Pentium(R) OverDrive(R) processor"
#define		INTEL_PPRO		"Pentium(R) Pro processor"
#define		INTEL_PII		"Pentium(R) II processor"
#define		INTEL_CELERON	"Celeron(TM) processor"
#define		INTEL_PIII		"Pentium(R) III processor"
#define		INTEL_XEON		"Pentium(R) III Xeon processor"
#define		INTEL_PIIO		"Pentium(R) II OverDrive(R) processor"

#define     PROCESSOR_UNKNOWN "Unknown processor"

class CpuInfo
{
public:
	unsigned int m_bSupport;				//	���� ��������� ������� CPUID
	char m_sVendor[13];						//	�������� �������������
	char m_sCpuName[48];					//	�������� ����������
	//	��������� ����������
	int m_iStepping;						//	����� ���������� ���������������
	int m_iModel;							//	������ ���������������
	int m_iFamily;							//	����� ��������� ���������������
	int m_iType;							//	��� ��������������� 
	//	����� ������������ ����������
	unsigned int m_bFPU;					//	������� FPU
	unsigned int m_bMMX;					//	������� MMX ����������
	unsigned int m_b3DNow;					//	��������� ���������� 3DNow! (������ ��� AMD)
	unsigned int m_bTSC;					//	��������� ������� RDTSC � ���� CR4.TSD
	unsigned int m_bSIMD;					//	��������� ���������� ��������� ��������� SIMD
	unsigned int m_bCMOV;					//	��������� ������� CMOV � ��� ������� ���������������� 
											//  FPU (m_bFPU==true) - ������ FCMOV � FCOMI
public:
	CpuInfo();
	virtual ~CpuInfo();
public:
	void Info(void);
private:
	void GetCPUIDStringAMD(unsigned long funcNum, char *String);
	const char* GetIntelProcName(void);

};

#endif	//_CPU_INFO_H_