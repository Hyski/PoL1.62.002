/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   13.04.2000

************************************************************************/
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

class Interface
{
public:
	class ProgressBar;
private:
	struct Deleter 
	{
	public:
		Interface *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
	static bool m_IsInitialized;
private:
	ProgressBar* m_ProgressBar;
private:
	Interface();
	virtual ~Interface();
public:	
	//	������� �� ����� ����� ������
	void OnChangeVideoMode(void);	
	//	�������� ����������
	void Tick(void);
	//	����������� 2D ����������
	void Render2D(void);
	//	����������� 3D ����������
	void Render3D(void);
	//	������������ ���� �������� ����������
	//	����������� �������� ����� ��������� ����������
	void Release(void);
public:	//****  ���������� ���������� ����������� ****//
	//	�������� ����
	void RunMenuFlow(void);
	//	�������� ������
	void LoadGameLevel(const char* pLevelName);
	//	����� ����������� ���� yes/no
	int QuestionBox(const char* text);
	//	���������� �������c ���� �������� ������
	void UpdateLoadingProgressBar(float value);
	//	���������� ��������� �������� ���� �������� ������
	void SetProgressBarTitle(const char* title);
	//	��������/������� ������ ����������� ����������
	void MakeSaveLoad(GenericStorage& store);
	//	�������� ������ ���������� �������� �����
	ProgressBar* ProBar(void) const;
	//	�������� �����������, ��� ������ ����� ����������
	bool IsCameraMoveable(void);
public:
	static Interface *Instance(void);
	static bool IsInitialized(void) {return m_IsInitialized;}
	//	��������������� ������������� ����������
	static void PriorInitialize(void);
	//	��������� �������
	static void PlayBink(const char* file_name);		
	//	��������� ��������� ���������
	static void PlayBink(std::vector<std::string>& file_names);		
};

inline Interface* Interface::Instance(void)
{
	if(m_Deleter.m_pInstance && !IsInitialized()) throw CASUS("��������� �� ������������������!");

	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new Interface();
	return m_Deleter.m_pInstance;
}
//	�������� ������ ���������� �������� �����
inline Interface::ProgressBar* Interface::ProBar(void) const
{
	return m_ProgressBar;
}

/////////////////////////////////////////////////////////////////////////
///////////////////    class Interface::ProgressBar    //////////////////
/////////////////////////////////////////////////////////////////////////
class Interface::ProgressBar
{
private:
	float m_From;
	float m_To;
public:
	ProgressBar();
	~ProgressBar();
public:
	//	��������� ��������� ����������� �������� 0..1 �� �������� from..to
	void SetRange(float from, float to);
	//	��������� �������� ���� � ������������� ���������
	void SetPercent(float value); 
	//	��������� �������� �� ������� m/n
	void SetMofN(int m, int n); 
	//	��������� ������, ����������� ������� ����������� ������
	void SetTitle(const char* title); 
};

#endif	//_INTERFACE_H_