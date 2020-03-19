#pragma once

namespace PoL
{

//=====================================================================================//
//                                   class RankTable                                   //
//=====================================================================================//
class RankTable
{
	typedef std::vector<std::string> Ranks_t;
	Ranks_t m_ranks;

public:
	/// ���������� �������������� ��� ������ �� ��������������
	const std::string &getLocName(int id) const { return m_ranks[id]; }
	/// ���������� ���������� ������
	int getCount() const { return m_ranks.size(); }

	static RankTable *instance();

private:
	RankTable();
	~RankTable(){}
};

}
