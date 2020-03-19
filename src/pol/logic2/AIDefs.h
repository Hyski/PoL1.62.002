#pragma once

struct AIDefs
{
	// �������� ��������� ����� ��������
	static const float angle_eps;
	// ����������, �� ������� ���� ��������� �� ������ � ��������
	static const float alarm_dist;
	// ����������, �� ������� ���������� ������ ����� �������
	static const float signal_dist;
	static const float fixed_enemy_dist;
	static const float fixed_cure_dist;
	static const int fixed_turns;
	static const float patrol_cure_dist;
	static const float patrol_enemy_dist;
	static const unsigned int patrol_enemy_noLOF_move;
	static const float assault_cure_dist;
	static const float assault_enemy_dist;
	static const int assault_turns;
	static const float fixed_tech_enemy_dist;
	static const float patrol_tech_enemy_dist;
	static const float assault_tech_enemy_dist;
	static float norm_angle(float a);
	// �������� � ���� ������ �������� (�� ������ ��������) ����� ����� �����
	// ������� �������� eid_t
	typedef std::set<eid_t> eidlist_t;
	static void setDifferenceBetweenLists(const eidlist_t&, const eidlist_t&, eidlist_t*);
};
