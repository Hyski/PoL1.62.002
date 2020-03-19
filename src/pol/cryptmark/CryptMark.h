#pragma once

#include <string>

namespace SecurityMark
{

/// ����� singelton'�
class CryptMark
{
/// �����������
	CryptMark();

/// ����������
	~CryptMark(){};

/// ����������� �������
	void decrypt();

/// ����������������� ����������
	std::string m_label;

/// GUID
	std::string m_guid;

/// ��� ����������
	std::string m_comp;

/// �����
	time_t m_time;

public:
/// �������� ����� �� ������� ����������
	/** �������� ���������:
	  * - true  - ���� �������
	  * - false - ���� �� ������� */
	bool isMarked() const;

/// ��������� �����
	const std::string &getLabel() const {return m_label;}

/// ��������� GUID'�
	const std::string &getGuid() const {return m_guid;}

/// ��������� ����� ���������� �� ������� ������������� ����������
	const std::string &getComp() const {return m_comp;}

/// ��������� ������� ����������
	const time_t &getTime() const {return m_time;}

/// ��������/������� ������� CryptMark (singelton'�)
	static const CryptMark *getInstance();

/// ����� ������������ ��� ������������ ��������� ������ � exe'��
	static int forceInstantiate();
};

/// ������������ ��������� ������ � exe'��
static const int __instantiator_of_mark_for_security__ = CryptMark::forceInstantiate();

}