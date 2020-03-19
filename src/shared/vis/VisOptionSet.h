#if !defined(__VIS_OPTION_SET_H_INCLUDED_1071903162630071__)
#define __VIS_OPTION_SET_H_INCLUDED_1071903162630071__

#include "VisOption.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251)

//=====================================================================================//
//                                   class OptionSet                                   //
//=====================================================================================//
class VIS_IMPORT OptionSet
{
	typedef std::hash_map<std::string, HOption> Options_t;
	Options_t m_options;

public:
	OptionSet();
	OptionSet(const OptionSet &);
	~OptionSet();

#define VIS_MAKE_GET_OPTION(A)		\
	OptionTraits<ot##A>::reference_type get##A(const std::string &name) const		\
	{																				\
		return getOption(name)->get##A();											\
	}																				\
	void set##A(const std::string &n, OptionTraits<ot##A>::reference_type v) const	\
	{																				\
		return getOption(n)->set##A(v);												\
	}
	VIS_FOREACH_PARAMETER_TYPE(VIS_MAKE_GET_OPTION)
#undef VIS_MAKE_GET_OPTION

	bool optionExists(const std::string &) const;

	void addOption(const std::string &, const HOption &);
	void setOption(const std::string &, const HOption &);
	void removeOption(const std::string &);

	void clear();
	const std::string &getOptionName(unsigned int n) const;
	unsigned int optionCount() const { return m_options.size(); }

	void store(mll::io::obinstream &) const;
	void restore(mll::io::ibinstream &);

	friend VIS_IMPORT bool operator == (const OptionSet &, const OptionSet &);
	friend VIS_IMPORT bool operator != (const OptionSet &l, const OptionSet &r) { return !(l==r); }

	friend VIS_IMPORT std::ostream &operator<<(std::ostream &, const OptionSet &);

private:
	const HOption &getOption(const std::string &) const;
};

#pragma warning(pop)

//=====================================================================================//
//             inline const std::string &OptionSet::getOptionName() const              //
//=====================================================================================//
inline const std::string &OptionSet::getOptionName(unsigned int n) const
{
	Options_t::const_iterator i(m_options.begin());
	std::advance(i,n);
	return i->first;
}

}

#endif // !defined(__VIS_OPTION_SET_H_INCLUDED_1071903162630071__)