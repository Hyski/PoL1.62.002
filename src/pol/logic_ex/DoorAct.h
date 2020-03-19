#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class DoorAct : public Act                              //
//=====================================================================================//
class DoorAct : public Act
{
	std::string m_doorId;
	bool m_open;

	bool m_started;
	bool m_ended;

	float m_duration;

public:
	DoorAct(const std::string &doorId, bool open);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() {}
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }
};

}
