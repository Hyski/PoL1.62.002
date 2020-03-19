#if !defined(__EXCEPTION_INCLUDED__)
#define __EXCEPTION_INCLUDED__

#include <exception>
#include <string>
#include <mll/io/debug_ostream.h>

namespace mll 
{ 

namespace debug 
{

#if defined(_BREAK_ON_EXCEPTION)
#define MLL_MAKE_DBG_BREAK __asm { int 3 }
#else
#define MLL_MAKE_DBG_BREAK
#endif

//=====================================================================================//
//                                   class exception                                   //
//=====================================================================================//
//! ������� �����-����������
class exception : public std::exception
{
	std::string m_message;

public:
	//! \name ������������
	//! @{

	//! ����������� �� ���������
	exception() : m_message("Unknown Error.") { writeDebugOut(); }
	//! ����������� �����������
	exception(const exception& e) : m_message(e.m_message) {}
	//! ����� ����� ������ �� std::string msg
	exception(const std::string &msg) : m_message(msg) { writeDebugOut(); }
	//! ����� ����� ������ �� const char *msg
	exception(const char *msg) : m_message(msg) { writeDebugOut(); }

	//! @}

	//! ���� ����������� ������ � ���� ������
	//! \return ������ � ���������� �� ������
	virtual const char *what() const { return m_message.c_str(); }

	/// ��������� ���������� � ������ ��������� �� ������
	void prepend_info(const std::string &i)
	{
		m_message = i + m_message;
		MLL_DEBUG_OUT("mll::debug::exception: message changed [" << m_message << "]\n");
	}
	/// ��������� ���������� � ����� ��������� �� ������
	void append_info(const std::string &i)
	{
		m_message = m_message + i;
		MLL_DEBUG_OUT("mll::debug::exception: message changed [" << m_message << "]\n");
	}

	//! ������������� ����� ��������
	static void dbg_break() { __asm { int 3 } }

private:
	void writeDebugOut() const
	{
		MLL_DEBUG_OUT("mll::debug::exception: [" << m_message << "]\n");
	}
};

//=====================================================================================//
//                              class exception_template                              //
//=====================================================================================//
//! ��������� ����� ��� ��������� �������� ����������� ����� ����������
/*! \param B ������� �����-����������
 *  \param T ��� ���������� ��� �������������
 *
 *  �������� T ������������ ��� ����, ����� ����� ���� ������� ���������
 *  �������-����������, ������������ �� ������ ������-����������. 
 *  ��� ����������� ������������ ������-���������� ����������� ������:
 *  \code
 *  typedef exception_template<struct MyExceptionTag, exception> MyException;
 *  \endcode
 */
template<typename T, typename B = exception>
class exception_template : public B
{
public:
	exception_template() {}
	exception_template(const exception_template& e) : B(e) {}
	exception_template(const std::string &msg) : B(msg) {}
	exception_template(const char *msg) : B(msg) {}
};

#undef MLL_MAKE_DBG_BREAK

#define MLL_MAKE_EXCEPTION(Name,Parent)										\
class Name : public Parent													\
{																			\
public:																		\
	Name() {}																\
	Name(const Name& e) : Parent(e) {}										\
	Name(const std::string &msg) : Parent(msg) {}							\
	Name(const char *msg) : Parent(msg) {}									\
}

}

}

#endif