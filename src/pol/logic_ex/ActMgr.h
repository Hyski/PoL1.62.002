#pragma once

#include "FlexibleSSForward.h"

namespace PoL
{

//=====================================================================================//
//                                  struct EventData                                   //
//=====================================================================================//
struct EventData
{
	std::string m_level;		///< �������, �� ������� ���������� �������
	unsigned int m_phase;		///< ����
	unsigned int m_episode;		///< ������
	std::string m_object;		///< ������ �������, ���, ��� ���������
	std::string m_subject;		///< ������� �������, ���, �� �������� ���������� ��������

	/// ���������� �������������� ����� ������ �����, ����� ��� ������������ ��������
	::eid_t m_object_id;		///< ���������� ������������� �������
	::eid_t m_subject_id;		///< ���������� ������������� ��������

	EventData() : m_object_id(0), m_subject_id(0) {}
};

//=====================================================================================//
//                                    class ActMgr                                     //
//=====================================================================================//
class ActMgr
{
public:
	/// ���������� true, ���� ������ �������� ���������� �����
	static bool isPlayingScene();
	/// ���������� true, ���� ������ ��������� � play
	static bool isInPlay();
	/// ��� ��� ������������ �����
	static void play();
	/// ������ ����������� �����
	static void start(HAct);

	/// �������� � ��� ��������� ���� �������� ������
	static void dumpState();

	/// ������� ��� ������������� ������� �������
	static void onUseObject(const EventData &);
	/// ������� ��� ������������� ������� ��������
	static void onUseEntity(const EventData &);
	/// ������� � ������ �������� ��������
	static void onKillEntity(const EventData &);
	/// ������� ��� ������ ������
	static void onLevelEnter(const EventData &);
	/// ������� ��� ������ � ������
	static void onLevelExit(const EventData &);
	/// ������� ��� ������ ����
	static void onStartGame(const EventData &);
	/// ������� ��� �������� �� ����� ������
	static void onStartEpisode(const EventData &);
	/// ������� ��� �������� ��������
	static void onEntitySpawned(const EventData &);
	/// ������� ��� ����� ����
	static void onPhaseChanged(const EventData &);
	/// �������� ��� ��������� ������� ���������
	static void onEntityTeamChanged(const EventData &);
	/// �������� ��� ������ ���� ������
	static void onStartTurn(const EventData &);
	/// �������� ��� ��������� ���� ������
	static void onEndTurn(const EventData &);
	/// �������� ��� ��������� ���� ������
	static void onDestroyEntity(const EventData &);
	/// �������� ��� ��������� ���� ������
	static void onRelChanged(const EventData &);

	/// ��������� ��������� ���������� ����
	static void store(std::ostream &);
	/// ������������ ��������� ���������� ����
	static void restore(std::istream &, unsigned int );

	/// �������� �� ���� ��������� ������� ������ ��������
	static void dumpScriptParameters();

	/// ��� ������������ ��������� ���
	static void test();
	static void test(const std::string &);
	static void reset();
	
	static void cleanup();
};

}