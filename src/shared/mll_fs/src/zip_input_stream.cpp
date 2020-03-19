
#include "precomp.h"

#include "zip_input_stream.h"


using namespace mll::ml_encrypted_zip;


//=====================================================================================//
//                        zip_input_stream::zip_input_stream()                         //
//=====================================================================================//
zip_input_stream::zip_input_stream(const std::string &name, decrypter &dec, unsigned offset, bool seekable)
:	istream(0)
{
	m_stream.reset(new std::ifstream(name.c_str(), std::ios::in | std::ios::binary));
	m_zsf.reset(new zip_input_streambuf(m_stream->rdbuf(), dec, offset, seekable));
	this->init(m_zsf.get());
}