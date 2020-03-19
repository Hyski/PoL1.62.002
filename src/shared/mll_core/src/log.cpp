#include "precomp.h"

using namespace mll::debug;

namespace
{
/*=====================================================================================*\
 *                                 make_pretty_name()                                  * 
\*=====================================================================================*/
	void make_pretty_name(std::string &name)
	{
		std::transform(name.begin(),name.end(),name.begin(),tolower);
//		std::use_facet<std::ctype<char> >(std::locale()).tolower(&*name.begin(),&*name.end());
		std::replace(name.begin(),name.end(),'/','\\');

		if(!(
				(name.length() > 0 && name[0] == '\\') ||	// Абсолютный путь
				(name.length() > 1 && name[1] == ':')		// Имя диска
			))
		{
			name.insert(0, "logs\\");
		}
	}

/*=====================================================================================*\
 *                                     make_dirs()                                     * 
\*=====================================================================================*/
	bool make_dirs(const std::string &name)
	{
		// Создадим каталоги для лог-файла
		using std::string::size_type;
		size_type i = 0;

		while(i = name.find('\\',i+1), i != (size_type)std::string::npos)
		{
			std::string dirname(name,0,i);
			CreateDirectory(dirname.c_str(),NULL);
		}

		return true;
	}
}

//=====================================================================================//
//                              void log::set_debug_out()                              //
//=====================================================================================//
void log::set_debug_out()
{
	m_stream.reset(new mll::io::debug_ostream);
}

/*=====================================================================================*\
 *                              log_holder::log_holder()                               * 
\*=====================================================================================*/
log_holder::log_holder()
{
}

/*=====================================================================================*\
 *                              log_holder::~log_holder()                              * 
\*=====================================================================================*/
log_holder::~log_holder()
{
}

namespace
{
	struct mll_debug_out_inst
	{
		mll::debug::log m_log;
		mll_debug_out_inst() { m_log.set_debug_out(); }
	};
}

//=====================================================================================//
//                             static log &get_debug_out()                             //
//=====================================================================================//
static mll::debug::log &get_debug_out()
{
	static mll_debug_out_inst doi;
	return doi.m_log;
}

/*=====================================================================================*\
 *                               log_holder::instance()                                * 
\*=====================================================================================*/
log_holder &log_holder::instance()
{
	static log_holder instance;
	return instance;
}

/*=====================================================================================*\
 *                              log_holder::operator[]()                               * 
\*=====================================================================================*/
mll::debug::log &log_holder::operator[](const char *name)
{
	typedef std::map<std::string,log> logs_t;
	static logs_t logs;

	std::string name_key(name);
	make_pretty_name(name_key);

	logs_t::iterator i = logs.find(name_key);
	if(i != logs.end()) return i->second;

	return logs.insert(std::make_pair(name_key,log(name_key))).first->second;
}

/*=====================================================================================*\
 *                                     log::log()                                      * 
\*=====================================================================================*/
log::log(const std::string &name)
{
	if(make_dirs(name))
	{
		std::ofstream *stream = new std::ofstream(name.c_str());
		if(stream->is_open())
		{
			m_stream.reset(stream);
		}
		else
		{
			delete stream;
		}
	}
}

/*=====================================================================================*\
 *                                     log::log()                                      * 
\*=====================================================================================*/
log::log(const log &l)
:	m_stream(const_cast<std::auto_ptr<std::ostream> &>(l.m_stream).release())
{
}

/*=====================================================================================*\
 *                                    log::flush()                                     * 
\*=====================================================================================*/
void log::flush()
{
	if(m_stream.get()) m_stream->flush();
}

//=====================================================================================//
//                            log &log_holder::debug_out()                             //
//=====================================================================================//
mll::debug::log &log_holder::debug_out()
{
	return get_debug_out();
}