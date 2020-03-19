#pragma once

namespace PoL
{

//=====================================================================================//
//                           class Act : public MlRefCounted                           //
//=====================================================================================//
/// ���� ��� ���������� �����
class Act : public MlRefCounted
{
public:
	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() = 0;
	/// ������� �� ���
	virtual bool isStarted() const = 0;
	/// ������ ���
	virtual void start() = 0;
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau) = 0;
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const = 0;
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }
	/// �������� �� ��� ������� ��� ��������
	virtual bool hasEndGame() const { return false; }
};

}