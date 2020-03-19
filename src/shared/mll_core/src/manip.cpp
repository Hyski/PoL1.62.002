#include "precomp.h"

#include <vector>

namespace
{

struct _control_symbol
{
	const char m_symbol;
	const char *m_sign;
	_control_symbol(char sym, const char *sign) : m_symbol(sym), m_sign(sign) {}
};

std::pair<const _control_symbol *,int> _get_symbols()
{
	static const _control_symbol symbols[] =	{
		_control_symbol('\"',"\\\""),
			_control_symbol('\\',"\\\\"),
			_control_symbol('\n',"\\n"),
			_control_symbol('\r',"\\r"),
			_control_symbol('\t',"\\t"),
	};
	return std::pair<const _control_symbol *,int>(symbols,sizeof(symbols)/sizeof(_control_symbol));
}

const _control_symbol *_symbols()
{
	return _get_symbols().first;
}

int _symbols_size()
{
	return _get_symbols().second;
}

//=====================================================================================//
//                                    _is_control()                                    //
//=====================================================================================//
const _control_symbol *_is_control(char c)
{
	static const int count = _symbols_size();
	for(int i = 0; i < count; ++i)
	{
		if(c == _symbols()[i].m_symbol) return _symbols()+i;
	}
	return 0;
}

char _to_char(const std::string &sign)
{
	static const int count = _symbols_size();
	for(int i = 0; i < count; ++i)
	{
		if(sign == _symbols()[i].m_sign) return _symbols()[i].m_symbol;
	}
	return '@';
}

//=====================================================================================//
//                                   _filtered_out()                                   //
//=====================================================================================//
std::ostream& _filtered_out(std::ostream& stream, const std::string &str)
{
	std::string::const_iterator i = str.begin();

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
//                                _load_exact_string()                                 //
//=====================================================================================//
std::string _load_exact_string(std::istream &stream)
{
	std::string result;
	std::ios::iostate stream_state = stream.rdstate();

	stream >> std::noskipws >> mll::io::verify("\"");

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

	stream.clear(stream_state);

	return result;
}

//=====================================================================================//
//                              void _save_exact_string()                              //
//=====================================================================================//
void _save_exact_string(std::ostream &stream,const std::string& str)
{
	stream << "\"";
	_filtered_out(stream,str) << "\"";
}

//=====================================================================================//
//                                 bool find_ws_pred()                                 //
//=====================================================================================//
bool find_ws_pred(char c)
{
	return std::isspace(c,std::locale());
}

//=====================================================================================//
//                               void _exact_verify_sq()                               //
//=====================================================================================//
void _exact_verify_sq(std::istream &stream,const std::string& sq)
{
	for(unsigned int i=0;i<sq.size();++i)
	{
		if(stream.good())
		{
			char ch;
			stream >> ch;
			if(ch == sq[i]) continue;
		}

		throw mll::io::exact_manip_failure("mll::io::exact : string don't valid");
	}
}

//=====================================================================================//
//                             void _save_exact_wstring()                              //
//=====================================================================================//
void _save_exact_wstring(std::ostream &stream,const std::wstring& str)
{
	const unsigned int size = str.size()+1;
	const char* ptr = reinterpret_cast<const char*>(str.c_str());

	stream << "L(";
	stream.write(reinterpret_cast<const char*>(&size),sizeof(size));
	stream.write(ptr,size*sizeof(wchar_t));
	stream << ")";
}

std::wstring _load_exact_wstring(std::istream &stream)
{
	_exact_verify_sq(stream,"L(");

	unsigned int size;
	stream.read(reinterpret_cast<char*>(&size),sizeof(size));

	std::vector<wchar_t> result(size);
	stream.read(reinterpret_cast<char*>(&result[0]),size*sizeof(wchar_t));

	_exact_verify_sq(stream,")");
	return std::wstring(&result[0]);
}

}	//	namespace

namespace mll
{

namespace io
{

//=====================================================================================//
//             std::ostream& _const_exact_string_manipulator::save() const             //
//=====================================================================================//
std::ostream& _const_exact_string_manipulator::save(std::ostream& stream) const
{
	_save_exact_string(stream,m_value);
	return stream;
}

//=====================================================================================//
//                   std::istream& _exact_string_manipulator::load()                   //
//=====================================================================================//
std::istream& _exact_string_manipulator::load(std::istream& stream)
{
	m_value = _load_exact_string(stream);
	return stream;
}

//=====================================================================================//
//            std::ostream& _const_exact_wstring_manipulator::save() const             //
//=====================================================================================//
std::ostream& _const_exact_wstring_manipulator::save(std::ostream& stream) const
{
	_save_exact_wstring(stream,m_value);
	return stream;
}

//=====================================================================================//
//                  std::istream& _exact_wstring_manipulator::load()                   //
//=====================================================================================//
std::istream& _exact_wstring_manipulator::load(std::istream& stream)
{
	m_value = _load_exact_wstring(stream);
	return stream;
}


//=====================================================================================//
//                             _text_manipulator::verify()                             //
//=====================================================================================//
void _text_manipulator::do_it(std::istream &str)
{
	std::locale loc;
	str >> std::ws;

	const char *start = m_value;
	const char *end = m_value+strlen(m_value);
	bool prev_white = false;

	for(; start != end; ++start)
	{
		if(std::isspace(*start,loc))
		{
			if(!prev_white)
			{
				str >> std::ws;
			}
			prev_white = true;
		}
		else
		{
			prev_white = false;
			if(*start != str.get())
			{
				throw mll::io::verify_not_same_error("mll::io::text - Ќесовпадение провер€емой строки с контрольной");
			}
		}
	}

	str >> std::ws;
}

} // namespace 'io'

} // namespace 'mll'
