/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.07.2000

************************************************************************/
#include "precomp.h"

#include "CpuInfo.h"

CpuInfo::CpuInfo()
{
	//	����������� ��������� �������� ����������
	strcpy(m_sCpuName,PROCESSOR_UNKNOWN);
	//	-----------------------------------------
	m_bSupport = false;
	m_iStepping = 0;
	m_iModel = 0;
	m_iFamily = 0;
	m_iType = 0;
	//	-----------------------------------------
	m_bFPU	 = false;
	m_bMMX	 = false;
	m_b3DNow = false;
	m_bTSC	 = false;
	m_bSIMD  = false;
	m_bCMOV  = false;
	//	��������� ���������� � ����������
	Info();
}

void CpuInfo::Info(void)
{
	unsigned int cpuid_max_info = 0;
	unsigned int cpuid_max_extinfo = 0;

	//	��������� ��������� ������� CPUID
	__asm
	{
		pushfd							; EFLAGS ���������� � ����
		pop eax							; ������� � EAX ���������� �����
		mov ebx,eax						; ��������� ��������� �������� ������
		xor eax,200000h					; �������� ��� 21 � ��������� �������� ������
		push eax						; ��������� ����� �������� ������ � ����
		popfd							; ������� � EFLAGS �� ����� ����� �������� ������
		pushfd							; EFLAGS ���������� � ����
		pop eax							; ������� � EAX ���������� �����
		xor eax,ebx						; �������� ��������� ����� �������� ������ � ����������� �����
		je no_cpuid						; �������, ���� ��� 21 �� ���������
		mov esi,this					; ������� � ������� ESI ����� ����� ������
		mov [esi].m_bSupport,1			; ������� � ���������� m_bSupport �������
no_cpuid:
	}

	//	���� CPUID ��������������, ����������
	if(m_bSupport)
	{
		//	���������� ������������ �������� ���������� �������� CPUID
		//	� ��� �������������
		for(int i=0;i<4;i++)
		{
			switch(i)
			{
			case 0:
				__asm
				{
					mov eax,0					; ������� � EAX ����
					cpuid						; �������� ������� CPUID
					mov cpuid_max_info,eax		; ���������� ������������ �������� ���������� �������� CPUID
					mov esi,this				; ������� � ������� ESI ����� ����� ������
					lea esi,[esi].m_sVendor		; ������� � ������� ESI ����� ����� ������ m_sVendor
					mov [esi],ebx
					mov [esi+4],edx
					mov [esi+8],ecx
					mov byte ptr[esi+12],0
				}
				break;
			case 1:
				if(cpuid_max_info>0)
				{
					__asm
					{
						//	���������� ��������� ����������
						mov eax,1						; ������� � EAX �������
						cpuid							; �������� ������� CPUID
						mov esi,this					; ������� � ������� ESI ����� ����� ������
						mov [esi].m_iStepping,eax		; 
						and [esi].m_iStepping,0000000fh	; ��� ����� ������ [3:0] ����
						mov [esi].m_iModel,eax			; 
						shr [esi].m_iModel,4			; 
						and [esi].m_iModel,0000000fh	; ��� ����� ������ [7:4] ����
						mov [esi].m_iFamily,eax			; 
						shr [esi].m_iFamily,8			; 
						and [esi].m_iType,0000000fh		; ��� ����� ������ [11:8] ����
						shr [esi].m_iType,12			; 
						and [esi].m_iType,00000003h		; ��� ����� ������ [13:12] ����
						//	���������� ����������� ����������						
						bt  edx,0h						; �������� ���� [1]  
						adc [esi].m_bFPU,0				;
						bt  edx,4h						; �������� ���� [4]  
						adc [esi].m_bTSC,0				;
						bt  edx,0fh						; �������� ���� [15]  
						adc [esi].m_bCMOV,0				;
						bt  edx,17h						; �������� ���� [23]  
						adc [esi].m_bMMX,0				;
						bt  edx,19h						; �������� ���� [25]  
						adc [esi].m_bSIMD,0				;
					}
					//	��������� ���������� AMD
					if(strcmp(m_sVendor,VENDOR_AMD)==0)
					{
						//	�������� �� ������� ���������� 3DNow!
						__asm
						{
							//	�������� ��������� ����������� �������
							mov eax,80000000h			; ������� � EAX 0x80000000
							cpuid						; �������� ������� CPUID
							mov cpuid_max_extinfo,eax	; ���������� ������������ �������� ���������� �������� CPUID
						}
						if(cpuid_max_extinfo>0x80000000)
						{
							__asm
							{
								mov esi,this				; ������� � ������� ESI ����� ����� ������
								mov eax,80000001h			; ������� � EAX 0x80000001
								cpuid						; �������� ������� CPUID
								bt edx,1fh					; �������� ���� 31
								adc [esi].m_b3DNow,0		;
							}
						}
						//	��������� ������������ ����������
						GetCPUIDStringAMD(0x80000002,m_sCpuName);
						GetCPUIDStringAMD(0x80000003,m_sCpuName+16);
						GetCPUIDStringAMD(0x80000004,m_sCpuName+32);
					}
					//	��������� ���������� Intel
					if(strcmp(m_sVendor,VENDOR_INTEL)==0)
					{
						//	��������� ������������ ����������
						strcpy(m_sCpuName,GetIntelProcName());
					}
				}
				break;
			}
		}
	}
}

CpuInfo::~CpuInfo()
{
}

void CpuInfo::GetCPUIDStringAMD(unsigned long funcNum, char *String)
{
	unsigned long retval;

	_asm
	{
			mov eax,funcNum
			cpuid
			mov	retval,eax
			mov	eax,String
			mov	dword ptr[eax+4],ebx
			mov	dword ptr[eax+8],ecx
			mov	ebx,retval
			mov	dword ptr[eax+12],edx
			mov	dword ptr[eax],ebx
	}
}

const char* CpuInfo::GetIntelProcName(void)
{
	switch(m_iFamily)
	{
		case 4: if (m_iModel == 4) return INTEL_486SL; 
				if (m_iModel == 7) return INTEL_DX2WB; 
				if (m_iModel == 8) return INTEL_DX4; 
				if ((m_iModel == 8)&&(m_iType==1)) return INTEL_DX4O;
				break;
		case 5:	if (m_iModel == 1) return INTEL_P;
				if (m_iModel == 2) return INTEL_P;
				if ((m_iModel == 1)&&(m_iType==1)) return INTEL_PO;
				if ((m_iModel == 2)&&(m_iType==1)) return INTEL_PO;
				if ((m_iModel == 3)&&(m_iType==1)) return INTEL_PO;
				if (m_iModel == 3) return INTEL_P; 
				if (m_iModel == 4) return INTEL_P; 
				if ((m_iModel == 4)&&(m_iType==1)) return INTEL_PO; 
				break;
		case 6: if (m_iModel == 1) return INTEL_PPRO;
				if (m_iModel == 3) return INTEL_PII; 
				if ((m_iModel == 3)&&(m_iType==1)) return INTEL_PIIO;
				if (m_iModel == 5) return INTEL_PII;
				if (m_iModel == 6) return INTEL_CELERON;
				if (m_iModel == 7) return INTEL_PIII; 
				if (m_iModel == 8) return INTEL_PIII; 
				if (m_iModel == 10) return INTEL_XEON;
				break;
	}

	return PROCESSOR_UNKNOWN; 
}