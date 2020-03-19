#pragma once

#include <Logic2/Phrase.h>

namespace PoL
{

//=====================================================================================//
//                                  class LocTextMgr                                   //
//=====================================================================================//
class LocTextMgr
{
	typedef std::hash_map<std::string,::phrase_s> Phrases_t;
	Phrases_t m_phrases;

public:
	LocTextMgr();
	~LocTextMgr();

	/// ���������� �������������� ������ �� �����
	std::string getString(const std::string &) const;
	/// ���������� �������������� ����� �� �����
	::phrase_s getPhrase(const std::string &) const;

	static LocTextMgr *instance();

private:
	void init();
};

}
