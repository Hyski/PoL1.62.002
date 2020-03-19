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
	/// Возвращает true, если из цикла необходимо выйти
	bool needExit() const { return m_needExit || (m_parent && m_parent->needExit()); }
	/// Выйти из цикла
	void exit() { m_needExit = true; }

	/// Функция отрисовки
	void render();

	/// Нормальный цикл
	void normalThink();
	/// Цикл при простое приложения
	void idleThink();

	/// Завершение кадра
	void endFrame();

	/// Установить невыхождение из цикла
	void noExit() { m_needExit = false; }

	/// Установить родительский цикл
	void setParent(ModalLoop *p) { m_parent = p; }

private:
	/// Функция цикла
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
	//	единственный экземпляр класса
	static Shell* m_Instance;
	//	контейнер для данных
	MasterPtr<Container> m_Container;
	//	основной цикл приложения
	typedef void (Shell::*Loop)(void);
	Loop m_Loop;
	//	флаг выхода из приложения
	bool m_Exit;
	//	флаг вхождния в модальный режим
	bool m_IsModalMode;

public:
	Shell();
	virtual ~Shell();

public:
	//	получить каркас приложения
	DDFrame* Frame(void);
	//	получить указатель на GraphPipe
	GraphPipe* GetGraphPipe(void);
	//	получить кол-во кадров в секунду
	float FPS(void) const;
	//	войти в модальный цикл
	void DoModal(ModalLoop *);
	//	проверить: свободен ли модальный режим
	bool IsModal(void) const;
	//	завершить работу приложения
	void Exit(void);
	//	вперед!
	void Go(void);

private:
	//	цикл исполнения
	void PerformanceLoop(void);
	//	цикл ожидания
	void IdleLoop(void);

private:
	//	обработка событий
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