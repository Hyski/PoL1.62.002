#include "precomp.h"

using namespace mll;
using namespace mll::utils;

/*namespace
{
	struct _control_symbol
	{
		const char m_symbol;
		const char *m_sign;
		_control_symbol(char sym, const char *sign) : m_symbol(sym), m_sign(sign) {}
	};

	const _control_symbol symbols[] =	{
											_control_symbol('\"',"\\\""),
											_control_symbol('\\',"\\\\"),
											_control_symbol('\n',"\\n"),
											_control_symbol('\t',"\\t"),
										};

//=====================================================================================//
//                                    _is_control()                                    //
//=====================================================================================//
	const _control_symbol *_is_control(char c)
	{
		static const int count = sizeof(symbols)/sizeof(_control_symbol);
		for(int i = 0; i < count; ++i)
		{
			if(c == symbols[i].m_symbol) return symbols+i;
		}
		return 0;
	}

	char _to_char(const std::string &sign)
	{
		static const int count = sizeof(symbols)/sizeof(_control_symbol);
		for(int i = 0; i < count; ++i)
		{
			if(sign == symbols[i].m_sign) return symbols[i].m_symbol;
		}
		return '@';
	}

//=====================================================================================//
//                                   _filtered_out()                                   //
//=====================================================================================//
	std::ostream &_filtered_out(std::ostream &stream, const std::string &str)
	{
		const char *i = str.begin();

		for(; i != str.end(); ++i)
		{
			const _control_symbol *result = _is_control(*i);
			if(result)
			{
				stream << result->m_sign;
			}
			else
			{
				stream << *i;
			}
		}

		return stream;
	}

//=====================================================================================//
//                                  _read_variable()                                   //
//=====================================================================================//
	std::string _read_variable(std::istream &stream)
	{
		std::string result;
		stream.ignore();

		while(stream.good())
		{
			char c;
			stream >> c;

			if(c == '\"') return result;

			if(c == '\\')
			{
				std::string ctrl_sym;
				ctrl_sym += c;
				stream >> c;
				ctrl_sym += c;
				result += _to_char(ctrl_sym);				
			}
			else
			{
				result += c;
			}
		}
		return result;
	}
}*/

namespace mll
{
namespace utils
{

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
std::ostream &operator<<(std::ostream &stream, const named_vars &vars)
{
	stream << "{\n";
	for(named_vars::const_iterator i = vars.begin(); i != vars.end(); ++i)
	{
		stream << io::exact(i->name()) << " " << io::exact(i->value()) << "\n";
	}
	stream << "}";

	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
std::istream &operator>>(std::istream &stream, named_vars &vars)
{
	vars.clear();

	stream >> std::noskipws;
	stream >> std::ws >> io::verify("{");
	
	while(true)
	{
		stream >> std::ws;

		if(stream.peek() == '}')
		{
			stream.ignore();
			return stream;
		}

		if(stream.peek() != '\"')
		{
			throw std::runtime_error("Ошибка при чтении named_vars");
		}

		std::string var_name,var_value;
		stream >> io::exact(var_name) >> std::ws >> io::exact(var_value) >> std::ws;

		vars[var_name] = var_value;
	}

	return stream;
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
io::obinstream &operator<<(io::obinstream &stream, const named_vars &vars)
{
	stream << vars.size();
	for(named_vars::const_iterator i = vars.begin(); i != vars.end(); ++i)
	{
		stream << i->name() << i->value();
	}
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
io::ibinstream &operator>>(io::ibinstream &stream, named_vars &vars)
{
	vars.clear();

	unsigned count;
	stream >> count;

	for(unsigned i = 0; i < count; ++i)
	{
		std::string var_name;
		stream >> var_name;
		std::string var_value;
		stream >> var_value;
		vars[var_name] = var_value;
	}

	return stream;
}

} // namespace 'utils'
} // namespace 'mll'