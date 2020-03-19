#if !defined(__ZIP_INPUT_STREAM_H_INCLUDED_7552370787048739__)
#define __ZIP_INPUT_STREAM_H_INCLUDED_7552370787048739__


#include "zip_input_streambuf.h"
#include "decrypter.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>



namespace mll
{

namespace ml_encrypted_zip
{

using std::istream;

//=====================================================================================//
//                     class zip_input_stream: public std::istream                     //
//=====================================================================================//
class zip_input_stream: public istream
{
private:
	std::auto_ptr<std::istream> m_stream;
	std::auto_ptr<zip_input_streambuf> m_zsf;
	
public:
	explicit zip_input_stream(const std::string &name, decrypter &dec, unsigned offset, bool seekable);
};

}

}

#endif // !defined(__ZIP_INPUT_STREAM_H_INCLUDED_7552370787048739__)