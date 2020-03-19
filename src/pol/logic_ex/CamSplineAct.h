#pragma once

#include "Act.h"

class KeyAnimation;

namespace PoL
{

//=====================================================================================//
//                           class CamSplineAct : public Act                           //
//=====================================================================================//
class CamSplineAct : public Act
{
	std::string m_camera;
	float m_length;

	bool m_started;
	bool m_ended;
	float m_startTime;
	bool m_roofsVisible;

public:
	CamSplineAct(const std::string &camname, float dur = 0.0f);

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
