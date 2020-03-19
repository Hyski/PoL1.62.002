#pragma once

//=====================================================================================//
//                                 class LevelCaseInfo                                 //
//=====================================================================================//
struct LevelCaseInfo
{
	int m_weight;			///< ������������ �������� ���
	int m_respawn;			///< ??? ���������������� ����� �� ������������
	std::string m_setName;	///< �������� ������ ���������

	LevelCaseInfo()
	:	m_weight(0),
		m_respawn(0)
	{
	}
};