/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
        ���������� � ��������� ����������� � �� �������.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   16.07.2000

************************************************************************/
#ifndef _D3D_INFO_H_
#define _D3D_INFO_H_

/*************************** class VideoMode *************************/
class VideoMode
{
public:
	unsigned int width;
	unsigned int height;   
	unsigned int bpp;  
	unsigned int freq;
public:
	VideoMode()
	{
		width = 0; height = 0; bpp = 0; freq = 0;
	};
	VideoMode(unsigned int w,unsigned int h,unsigned int b,unsigned int f)
	{
		width = w; height = h; bpp = b; freq = f;
	};
};

inline bool operator == (const VideoMode& x,const VideoMode& y)
{
	if((x.width==y.width) && (x.height==y.height) && (x.bpp==y.bpp) && (x.freq==y.freq))
		return true;
	return false;
}

inline bool operator < (const VideoMode& x,const VideoMode& y)
{
	if(x.width < y.width)
		return true;
	if(x.width > y.width)
		return false;
	if(x.height < y.height)
		return true;
	if(x.height > y.height)
		return false;
	if(x.freq < y.freq)
		return true;
	if(x.freq > y.freq)
		return false;
	if(x.bpp < y.bpp)
		return true;

	return false;
}

/*************************** class DeviceInfo *************************/
class DeviceInfo
{
public:
	std::string sDescription;		//	�������� �������
	std::string sName;				//	�������� �������
	GUID guid;						//	���������� �����������
	bool bHardware;					//	���� ��������� HAL
	bool bTransformAndLighting;		//	���� ��������� ������������� � �����
	std::vector<VideoMode> vMode;
};

inline bool operator < (const DeviceInfo& x,const DeviceInfo& y)
{
	if(x.bTransformAndLighting && x.bHardware)
		return true;
	if(y.bTransformAndLighting && y.bHardware)
		return false;
	if(x.bHardware)
		return true;
	if(y.bHardware)
		return false;
	if(x.bTransformAndLighting)
		return true;
	if(y.bTransformAndLighting)
		return false;
	
	return false;
}

/*************************** struct DriverInfo *************************/
struct DriverInfo 
{
	std::string sDescription;		//	�������� ��������
	std::string sName;				//	�������� ��������
	GUID guid;						//	���������� ������������� ��������
	bool bPrimary;					//	���� ���������� ��������
	DDCAPS	ddDriverCaps;			//	driver capabilities
	DDCAPS	ddHELCaps;				//	hardware emulation level capabilities
	unsigned long ulMonitorFreq;		//	������� ������� ����������� ��������
	std::vector<VideoMode> vMode;		//	��������� �����������
	std::vector<DeviceInfo> vDevice;	//	������ ��������� d3dDevice'�� � ���������� � ���
};
/*************************************************************************/

namespace D3DInfo
{
	extern std::vector<DriverInfo> m_vDriverInfo;
	//	-------------------------------------------------------------
	void Refresh(void);
	void CheckIndex(unsigned int *pDriver,unsigned int *pDevice);
	GUID* GetDriverGuidPtr(unsigned int iDriver);
	GUID* GetDeviceGuidPtr(unsigned int iDriver,unsigned int iDevice);
}

#endif	//_D3D_INFO_H_