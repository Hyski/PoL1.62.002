#pragma once

namespace PoL
{

//=====================================================================================//
//                                  class JournalMgr                                   //
//=====================================================================================//
class JournalMgr
{
	struct JournalRecord
	{
		int m_headerkey;
		int m_textkey;
		std::string m_header;
		std::string m_text;
	};

	typedef std::hash_map<std::string,JournalRecord> Records_t;

	Records_t m_records;

public:
	JournalMgr();
	~JournalMgr();

	void addRecord(const std::string &id, const std::string &hdr, const std::string &text);
	void removeRecord(const std::string &id);

	void store(mll::io::obinstream &) const;
	void restore(mll::io::ibinstream &);
	void reset();

	static JournalMgr *instance();
};

}
