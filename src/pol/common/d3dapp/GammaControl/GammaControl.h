/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   05.07.2000

************************************************************************/
#ifndef _GAMMA_CONTROL_H_
#define _GAMMA_CONTROL_H_

namespace GammaControl
{
	void Init(LPDIRECTDRAWSURFACE7 pPrimarySurface);
	void Release(void);
	//	���������
	void IncBrightness(void);
	void DecBrightness(void);
	void IncContrast(void);
	void DecContrast(void);
	void IncGamma(void);
	void DecGamma(void);
	void SetGamma(float g);
	//	��������� ����������
	DDGAMMARAMP GammaRamp(void);
	float GetGamma(void);
}

#endif	//_GAMMA_CONTROL_H_