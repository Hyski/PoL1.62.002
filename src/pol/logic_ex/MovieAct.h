#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class MovieAct : public Act                             //
//=====================================================================================//
class MovieAct : public Act
{
	std::string m_bik;
	std::string m_levl;
	bool m_started;
	bool m_ended;
	bool m_firstTime;

public:
	MovieAct(const std::string &bik, const std::string &levl);

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
