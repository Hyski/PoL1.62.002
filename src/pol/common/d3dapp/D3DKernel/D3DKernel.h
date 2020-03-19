/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: ������ �������� ����� ������������� Direct3D.
                 �������� ������ ��������� ����������:
				 - GetDD();         // ��������� ������� DirectDraw
				 - GetPS();         // ��������� ������� PrimarySurface
				 - GetBB();         // ��������� ������� BackBuffer
				 - GetD3D();	    // ��������� ������� Direct3D
				 - GetD3DDevice();  // ��������� ������� Direct3DDevice

   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   09.05.2000

************************************************************************/
#ifndef _D3D_KERNEL_H_
#define _D3D_KERNEL_H_

namespace D3DKernel
{
	//	�������������
	BOOL Init(HWND hWnd,
			  GUID* lpDriverGUID,
			  GUID* lpDeviceGUID,
			  int width,
			  int height,
			  int depth,
			  int freq,
			  bool bWindowed);
	//	���������� ������������� TripleBuffering'�
	void SetTriple(BOOL bTriple);
	//	����������� �������
	void Release(void);
	//	�������������� ������������ �������
	void UpdateFrame(void);
	//	������������ �������
	void Flip(void);
	//	��������
	void Blt(void);
	//	������������� ������������
	void RestoreSurfaces(void);
	//	����������� ViewPort'�
	void MoveViewPort(int x,int y);
	//	��������� ����������
	LPDIRECTDRAW7		 GetDD(void);
	LPDIRECTDRAWSURFACE7 GetPS(void);
	LPDIRECTDRAWSURFACE7 GetBB(void);
	LPDIRECT3D7          GetD3D(void);
	LPDIRECT3DDEVICE7    GetD3DDevice(void);
	int ResX(void);
	int ResY(void);
	BOOL IsFullScreen(void);
}

#endif	//_D3D_KERNEL_H_