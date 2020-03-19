/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _GAME_H_
#define _GAME_H_

class GraphPipe; 

namespace Game
{
	//	������� �����
	extern float m_fGameTime;
	//	�������������
	void Init(GraphPipe *lpGraphPipe);
	//	�������� �������� ����������
	void PriorInterfaceInitialize(void);
	void CreateInterface(void);
	//	������������ ��������
	void PlayBink(const char *pFileName);
	//	����� �����������
	void Tune(void);
	void Close(void);
	//	�������� ���������� ����
	void Tick(void);
	void ProcessInput(void);
	void Render(void);
	//	���������� �����
	void SetGameLevelRender(bool render);
	void SetGameLevelUpdate(bool update);
	void LoadGameLevel(const char *pLevelName);
	void UnloadGameLevel(void);
	//	��������� ���������
	void MakeScreenShot(void);
	//	����� ����� Alt-Tab'�
	void AdjustHW(void);
	//	��������� �������� ��������
	void SetEffectsQuality(float q);
	//	���������/��������� ����
	void MakeSaveLoad(const char* pFileName,bool bSave);
	bool RenderEnabled();

	const std::string &GetAddonName();
}

#endif