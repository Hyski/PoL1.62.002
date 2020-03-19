/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

extern CLog logFile;

extern const float VirtualResX;
extern const float VirtualResY;

//	������� �� ����������� ���������
inline float ToVirtualX(float x)
{
	return (float)((float)VirtualResX*x)/((float)D3DKernel::ResX());
}

inline float ToVirtualY(float y)
{
	return (float)((float)VirtualResY*y)/((float)D3DKernel::ResY());
}

inline float ToScreenX(float x)
{
	return (float)((float)D3DKernel::ResX()*x)/VirtualResX;
}

inline float ToScreenY(float y)
{
	return (float)((float)D3DKernel::ResY()*y)/VirtualResY;
}

#endif	//_GLOBALS_H_