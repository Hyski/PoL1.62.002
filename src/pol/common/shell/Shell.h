/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _SHELL_H_
#define _SHELL_H_

#include <undercover/MasterPtr.h>
#include <undercover/ddframe/ddframe.h>

class GraphPipe;
class ModalLoop;

//=====================================================================================//
//                                   class ModalLoop                                   //
//=====================================================================================//
class ModalLoop
{
	ModalLoop *m_parent;
	bool m_needExit;

protected:
	ModalLoop() : m_parent(0), m_needExit(false) {}
	virtual ~ModalLoop() {}

public:
	/// ���������� true, ���� �� ����� ���������� �����
	bool needExit() const { return m_needExit || (m_parent && m_parent->needExit()); }
	/// ����� �� �����
	void exit() { m_needExit = true; }

	/// ������� ���������
	void render();

	/// ���������� ����
	void normalThink();
	/// ���� ��� ������� ����������
	void idleThink();

	/// ���������� �����
	void endFrame();

	/// ���������� ������������ �� �����
	void noExit() { m_needExit = false; }

	/// ���������� ������������ ����
	void setParent(ModalLoop *p) { m_parent = p; }

private:
	/// ������� �����
	virtual void think();
};

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class Shell    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
class Shell : public DDFrame::Controller, private ModalLoop
{
	friend class ModalLoop;
	typedef std::list<ModalLoop*> Loops_t;
	Loops_t m_loops;

	struct Container;
	class FPSometer;
public:
	static const char* m_AppName;

private:
	//	������������ ��������� ������
	static Shell* m_Instance;
	//	��������� ��� ������
	MasterPtr<Container> m_Container;
	//	�������� ���� ����������
	typedef void (Shell::*Loop)(void);
	Loop m_Loop;
	//	���� ������ �� ����������
	bool m_Exit;
	//	���� �������� � ��������� �����
	bool m_IsModalMode;

public:
	Shell();
	virtual ~Shell();

public:
	//	�������� ������ ����������
	DDFrame* Frame(void);
	//	�������� ��������� �� GraphPipe
	GraphPipe* GetGraphPipe(void);
	//	�������� ���-�� ������ � �������
	float FPS(void) const;
	//	����� � ��������� ����
	void DoModal(ModalLoop *);
	//	���������: �������� �� ��������� �����
	bool IsModal(void) const;
	//	��������� ������ ����������
	void Exit(void);
	//	������!
	void Go(void);

private:
	//	���� ����������
	void PerformanceLoop(void);
	//	���� ��������
	void IdleLoop(void);

private:
	//	��������� �������
	void OnClose(void);
	void OnActivate(void);
	void OnDeactivate(void);

public:
	void OnStartChangeVideoMode(void);
	void OnFinishChangeVideoMode(void);

public:
	static Shell* Instance(void);
};

//=====================================================================================//
//                           inline Shell* Shell::Instance()                           //
//=====================================================================================//
inline Shell* Shell::Instance()
{
	return m_Instance;
}

//=====================================================================================//
//                              inline void Shell::Exit()                              //
//=====================================================================================//
inline void Shell::Exit()
{
	exit();
}

//=====================================================================================//
//                               inline DDFrame* Frame()                               //
//=====================================================================================//
inline DDFrame* Frame()
{
	return Shell::Instance()->Frame();
}

#endif	//_SHELL_H_	