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
\brief ��������� �������

����� ����������� ��� ������ ���������� � ������������ �������, ���������������.
��� �������� ������� ����� ������������ � ���� html ���� � �� ������� ����������� �������
����� - �������� ���������. instance() �������� ������������ � ��������.

module - ������ ��������, �� �������� �������������� ���������� (�������� '�������� ����')
message - ���������� ������ ��� ��������������, �� ���� ���� ���� ���������� (�������� '�� ������ ����')
params - ���������� ���������� (�������� '����: p_engine.dat') 
*/

//=====================================================================================//
//                                   class reporter                                    //
//=====================================================================================//
class MLL_EXPORT reporter
{
private:

	class isolation;
	std::auto_ptr<isolation> m_isolation;		///< ������ ���������� ����������

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
	///	�������� ���������
	static void set_parameters(const std::string& file_name, bool auto_start);
	/// �������� ������ � �������� ���
	static void flush();

private:
	static void do_add_report(report_kind, const std::string &, const std::string &, const std::string &);

	///	�������� ��������� ������
	static reporter* instance(void);

	friend utils::oneobj<reporter>;
};

/// �������� � ����� ��������������
#define MLL_REPORT_WARNING(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_warning,module,message,parameter)
/// �������� � ����� ������
#define MLL_REPORT_ERROR(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_error,module,message,parameter)
/// �������� � ����� ���������
#define MLL_REPORT_MESSAGE(module,message,parameter)  ::mll::debug::reporter::add_report(::mll::debug::reporter::rk_message,module,message,parameter)

#define MLL_REPORT(module,message,parameter) MLL_REPORT_ERROR(module,message,parameter)

}	//	namespace debug

}	//	namespace mll
