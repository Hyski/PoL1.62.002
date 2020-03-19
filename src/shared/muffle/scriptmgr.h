#if !defined(__SCRIPTMGR_H_INCLUDED_0442986472349163__)
#define __SCRIPTMGR_H_INCLUDED_0442986472349163__

#include <hash_map>
#include "Script2.h"

namespace Muffle
{

//=====================================================================================//
//                                   class ScriptMgr                                   //
//=====================================================================================//
class ScriptMgr : private noncopyable
{
	typedef std::hash_map<std::string,Muffle2::Script> Scripts_t;
	Scripts_t m_scripts;

	void dump() const;

public:
	ScriptMgr(SndServices* services);

	/// Возвращает скрипт по имени
	const Muffle2::Script *get(const std::string &) const;
	/// Внедрить скрипт
	const Muffle2::Script *embedScript(const ScriptInfo &);
};

}	//	namespace Muffle

#endif // !defined(__SCRIPTMGR_H_INCLUDED_0442986472349163__)