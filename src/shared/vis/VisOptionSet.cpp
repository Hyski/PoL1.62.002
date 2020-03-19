#include "Vis.h"

namespace Vis
{

//=====================================================================================//
//                               OptionSet::OptionSet()                                //
//=====================================================================================//
OptionSet::OptionSet()
{
}

//=====================================================================================//
//                               OptionSet::OptionSet()                                //
//=====================================================================================//
OptionSet::OptionSet(const OptionSet &o)
{
	for(Options_t::const_iterator i = o.m_options.begin(); i != o.m_options.end(); ++i)
	{
		m_options.insert(std::make_pair(i->first,i->second->clone()));
	}
}

//=====================================================================================//
//                               OptionSet::~OptionSet()                               //
//=====================================================================================//
OptionSet::~OptionSet()
{
}

//=====================================================================================//
//                             void OptionSet::addOption()                             //
//=====================================================================================//
void OptionSet::addOption(const std::string &name, const HOption &value)
{
	assert( m_options.find(name) == m_options.end() );
	m_options[name] = value;
}

//=====================================================================================//
//                             void OptionSet::setOption()                             //
//=====================================================================================//
void OptionSet::setOption(const std::string &name, const HOption &value)
{
	assert( m_options.find(name) != m_options.end() );
	m_options[name] = value;
}

//=====================================================================================//
//                     const HOption &OptionSet::getOption() const                     //
//=====================================================================================//
const HOption &OptionSet::getOption(const std::string &name) const
{
	Options_t::const_iterator i = m_options.find(name);
	if(i == m_options.end()) throw OptionNotFound("Option [" + name + "] not found");
	return i->second;
}

//=====================================================================================//
//                           void OptionSet::removeOption()                            //
//=====================================================================================//
void OptionSet::removeOption(const std::string &name)
{
	assert( m_options.find(name) != m_options.end() );
	m_options.erase(name);
}

//=====================================================================================//
//                               void OptionSet::clear()                               //
//=====================================================================================//
void OptionSet::clear()
{
	m_options.clear();
}

//=====================================================================================//
//                        bool OptionSet::optionExists() const                         //
//=====================================================================================//
bool OptionSet::optionExists(const std::string &n) const
{
	return m_options.find(n) != m_options.end();
}

//=====================================================================================//
//                               void OptionSet::store()                               //
//=====================================================================================//
void OptionSet::store(mll::io::obinstream &out) const
{
	out << m_options.size();
	for(Options_t::const_iterator i = m_options.begin(); i != m_options.end(); ++i)
	{
		out << i->first;
		Option::storeOption(i->second,out);
	}
}

//=====================================================================================//
//                              void OptionSet::restore()                              //
//=====================================================================================//
void OptionSet::restore(mll::io::ibinstream &in)
{
	clear();
	unsigned int count;
	in >> count;

	for(unsigned int i = 0; i < count; ++i)
	{
		std::string name;
		in >> name;
		HOption opt = Option::restoreOption(in);
		addOption(name,opt);
	}
}

//=====================================================================================//
//                                  bool operator==()                                  //
//=====================================================================================//
bool operator==(const OptionSet &l, const OptionSet &r)
{
	if(l.m_options.size() != r.m_options.size()) return false;
	OptionSet::Options_t::const_iterator i = l.m_options.begin();
	OptionSet::Options_t::const_iterator j = r.m_options.begin();

	for(; i != l.m_options.end(); ++i, ++j)
	{
		if(i->first != j->first) return false;
		if(!i->second->compare(*j->second.get())) return false;
	}

	return true;
}

//=====================================================================================//
//                             std::ostream &operator<<()                              //
//=====================================================================================//
std::ostream &operator<<(std::ostream &out, const OptionSet &options)
{
	for(OptionSet::Options_t::const_iterator i = options.m_options.begin();
		i != options.m_options.end(); ++i)
	{
		out << "\t\t" << "[" << i->first << "] = [";
		if(i->second.get()) i->second->showSelf(out);
		out << "]\n";
	}
	return out;
}

}