#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                           class CamFollowAct : public Act                           //
//=====================================================================================//
class CamFollowAct : public Act, private ::EntityObserver
{
	std::string m_who;
	BaseEntity *m_entity;
	bool m_started;

public:
	CamFollowAct(const std::string &who);
	~CamFollowAct();

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() {}
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return false; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return false; }

private:
	void Update(BaseEntity *, event_t, info_t);
};

}
