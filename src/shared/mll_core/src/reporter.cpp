#include "precomp.h"
#include <mll/debug/reporter.h>
#include <mll/utils/oneobj.h>

#include <string>
#include <map>
#include <set>


namespace mll
{

namespace debug
{

#pragma warning ( disable:4503 )

//=====================================================================================//
//                              class reporter::isolation                              //
//=====================================================================================//
class reporter::isolation
{
public:

	typedef std::set<std::string> parameters_t;
	typedef std::map<std::string,parameters_t> messages_t;
	typedef std::map<std::string,messages_t> modules_t;

	modules_t m_reports[reporter::rk_count];

	std::string m_file_name;		//	имя выходного файла
	bool m_auto_start;				//	флаг автоматического открытия выходного файла

public:
	isolation();
	~isolation();
public:
	/// Добавить отчет
	void do_add_report(reporter::report_kind, const std::string& module, const std::string& message, const std::string& param);
	///	установить параметры
	void set_parameters(const std::string& file_name,bool auto_start);
	///	сбросить информацию в файл отчета
	void flush(void);

};

reporter::isolation::isolation()
: m_file_name("report.html"),
  m_auto_start(true)
{
}

reporter::isolation::~isolation()
{
	flush();
}

void reporter::isolation::do_add_report(reporter::report_kind kind, const std::string& module, const std::string& message, const std::string& param)
{
	m_reports[kind][module][message].insert(param);
}

///	установить параметры
void reporter::isolation::set_parameters(const std::string& file_name,bool auto_start)
{
	m_file_name = file_name;
	m_auto_start = auto_start;
}
///	сбросить информацию в файл отчета
void reporter::isolation::flush(void)
{
	bool empty = m_file_name.empty();
	for(int i = 0; i != reporter::rk_count; ++i)
	{
		empty = empty && m_reports[i].empty();
	}

	if(empty) return;

	{
		std::ofstream file(m_file_name.c_str());

		file << "<head>"
				"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1251\">"
				"</head>"
				"<body bgColor=\"#cc9999\">"
				"<STYLE>"
				".warning_head {PADDING: 3px; MARGIN: 5px; BACKGROUND-COLOR: khaki}"
				".error_head {PADDING: 3px; MARGIN: 5px; BACKGROUND-COLOR: tomato}"
				".message_head {PADDING: 3px; MARGIN: 5px; BACKGROUND-COLOR: cyan}"
				".mess_title {WIDTH: 100%; BORDER-BOTTOM: black 2px solid}"
				".module_name {PADDING-LEFT: 50px; WIDTH: 100%; text-decoration:underline }"
				".mess_text {PADDING-LEFT: 100px; WIDTH: 100%}"
				".params {PADDING-LEFT: 150px; WIDTH: 100%;font-family=Courier}"
				"</STYLE>\n";

		const std::string classes[] = {"error_head", "warning_head", "message_head"};
		const std::string headers[] = {"Errors", "Warnings", "Messages"};

		for(int ii = 0; ii != reporter::rk_count; ++ii)
		{
			if(!m_reports[ii].empty())
			{
				file << "<div class=" << classes[ii] << ">\n"
						"<div class=mess_title>" << headers[ii] << "</div>\n";
					    
				for(modules_t::iterator i = m_reports[ii].begin(); i != m_reports[ii].end(); ++i)
				{
					file << "<div class=module_name>" << i->first << "</div>\n";
					for(messages_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
					{
						file << "<div class=mess_text>" << j->first << "</div>\n";
						for(parameters_t::iterator k = j->second.begin(); k != j->second.end(); ++k)
						{
							file << "<div class=params>" << *k << "</div>\n";
						}
					}
				}

				file <<"</div>\n";
			}

			m_reports[ii].clear();
		}

		file << "</body>";
		file.flush();
	}

	if(m_auto_start)
	{
		std::string cmdname = "start " + m_file_name;
		_flushall();
		system(cmdname.c_str());
	}
}

//=====================================================================================//
//                                   class reporter                                    //
//=====================================================================================//
reporter::reporter()
: m_isolation(new isolation())
{
}

reporter::~reporter()
{
}

//	добавить предупреждение в отчет
void reporter::do_add_report(report_kind kind, const std::string& module, const std::string& message, const std::string& params)
{
	instance()->m_isolation->do_add_report(kind,module,message,params);
}

//	изменить параметры
void reporter::set_parameters(const std::string& file_name,bool auto_start)
{
	instance()->m_isolation->set_parameters(file_name,auto_start);
}

// Показать репорт и очистить его
void reporter::flush()
{
	instance()->m_isolation->flush();
}

//	получить экземпляр класса
reporter* reporter::instance(void)
{
	static mll::utils::oneobj<reporter> _proxy_instance;
	return _proxy_instance.get();
}

}	//	namespace debug

}	//	namespace mll
