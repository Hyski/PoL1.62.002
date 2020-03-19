#pragma once

/////////////////////////////////////////////////////////////////////////
//	STL Library files
#include <sstream>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace utils { template<typename T> class oneobj; }

namespace debug
{

/**
\brief Создатель отчетов

класс накапливает для отчета информацию о произошедших ошибках, предупреждениях.
при удалении объекта отчет сбрасывается в виде html фала и по желанию запускается браузер
класс - типичный синглетон. instance() является одновременно и фабрикой.

module - первый параметр, по которому осуществляется сортировка (например 'Менеджер пути')
message - собственно ошибка или предупреждение, по нему тоже идет сортировка (например 'не найден файл')
params - конкретная информация (например 'файл: p_engine.dat') 
*/

//=====================================================================================//
//                                   class reporter                                    //
//=====================================================================================//
class MLL_EXPORT reporter
{
private:

	class isolation;
	std::auto_ptr<isolation> m_isolation;		///< объект внутренней реализации

private:
	reporter();
	~reporter();
	
public:
	enum report_kind
	{
		rk_error,
		rk_warning,
		rk_message,
		
		rk_count
	};
	
public:
	template<typename T1, typename T2, typename T3>
	static void add_report(report_kind kind, T1 mod, T2 mes, T3 parm)
	{
		std::ostringstream o_mod, o_mes, o_parm;
		o_mod << mod;
		o_mes << mes;
		o_parm << parm;
		do_add_report(kind,o_mod.str(),o_mes.str(),o_parm.str());
	}

public:
	///	Изменить параметры
	static void set_parameters(const std::string& file_name, bool auto_start);
	/// Показать репорт и очистить его
	static void flush();

private:
	static void do_add_report(report_kind, const std::string &, const std::string &, const std::string &);

	///	получить экземпляр класса
	static reporter* instance(void);

	friend utils::oneobj<reporter>;
};

/// добавить в отчет предупреждение
#define MLL_REPORT_WARNING(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_warning,module,message,parameter)
/// добавить в отчет ошибку
#define MLL_REPORT_ERROR(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_error,module,message,parameter)
/// добавить в отчет сообщение
#define MLL_REPORT_MESSAGE(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_message,module,message,parameter)

#define MLL_REPORT(module,message,parameter) MLL_REPORT_ERROR(module,message,parameter)

}	//	namespace debug

}	//	namespace mll
