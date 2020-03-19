#pragma once

#include "Act.h"

namespace PoL
{

class SavedData;

//=====================================================================================//
//                          class ScriptSceneAct : public Act                          //
//=====================================================================================//
class ScriptSceneAct : public Act
{
	bool m_started;
	bool m_endable;
	bool m_ended;

	HAct m_act;

	std::auto_ptr<SavedData> m_savedData;

public:
	ScriptSceneAct(HAct act);
	~ScriptSceneAct();

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip();
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return m_endable; }
	/// Содержит ли акт выигрыщ или проигрыш
	virtual bool hasEndGame() const { return m_act->hasEndGame(); }

private:
	bool canStart() const;
	void doStart();
};

}