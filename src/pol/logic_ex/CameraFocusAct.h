#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class CameraFocusAct : public Act                          //
//=====================================================================================//
class CameraFocusAct : public Act
{
	std::string m_subj;
	::point3 m_target;
	bool m_canFocus;
	bool m_started;
	bool m_ended;
	float m_startTime;

public:
	CameraFocusAct(const std::string &);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() { m_ended = true; }
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
