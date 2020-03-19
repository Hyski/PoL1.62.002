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

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip();
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return m_endable; }
	/// �������� �� ��� ������� ��� ��������
	virtual bool hasEndGame() const { return m_act->hasEndGame(); }

private:
	bool canStart() const;
	void doStart();
};

}