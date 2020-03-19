#include "precomp.h"

using namespace mll::io;

namespace
{

//=====================================================================================//
//                                class stream_returner                                //
//=====================================================================================//
class stream_returner
{
	std::istream &m_str;
	std::streampos m_pos;
	bool m_doNothing;

public:
	stream_returner(std::istream &str) : m_str(str), m_pos(m_str.tellg()), m_doNothing(false) {}
	~stream_returner()
	{
		if(!m_doNothing)
		{
			if(!m_str.good()) m_str.clear(); m_str.seekg(m_pos);
		}
	}

	void do_nothing() { m_doNothing = true; }
};

}

//=====================================================================================//
//                   MLL_EXPORT std::istream &mll::io::operator>>()                    //
//=====================================================================================//
MLL_EXPORT std::istream &mll::io::operator>>(std::istream &stream, const _i_verifier &vf)
{
	if(!stream.good()) throw verify_stream_failure("mll::io::verify : the stream isn't good");
	stream_returner sr(stream);

	for(const char *itor = vf.m_str; *itor != '\0'; ++itor)
	{
		if(stream.get() != *itor)
		{
			if(stream.good())
			{
				std::ostringstream sstr;
				sstr << "mll::io::verify : string read from stream does not match pattern \"" << vf.m_str << "\"";
				sstr << std::ends;
				throw verify_not_same_error(sstr.str());
			}
		}

		if(!stream.good())
		{
			throw verify_stream_failure("mll::io::verify : the stream isn't good");
		}
	}

	sr.do_nothing();
	return stream;
}