#include "precomp.h"
#include "Stream.h"
#include "RealFile.h"

namespace Muffle
{

//=====================================================================================//
//                                  Stream::Stream()                                   //
//=====================================================================================//
Stream::Stream()
:	m_file(0)
{
}

//=====================================================================================//
//                                  Stream::Stream()                                   //
//=====================================================================================//
Stream::Stream(RealFile *file)
:	m_file(file)
{
	if(m_file)
	{
		m_stream = m_file->getStream();
		m_binstream.reset(new ibinstream(*m_stream));
		m_file->addRef();
	}
}

//=====================================================================================//
//                                  Stream::Stream()                                   //
//=====================================================================================//
Stream::Stream(Stream &s)
:	m_file(s.m_file),
	m_stream(s.m_stream),
	m_binstream(s.m_binstream)
{
	if(m_file) m_file->addRef();
}

//=====================================================================================//
//                                  Stream::~Stream()                                  //
//=====================================================================================//
Stream::~Stream()
{
	if(m_file) m_file->release();
}

//=====================================================================================//
//                             Stream &Stream::operator=()                             //
//=====================================================================================//
Stream &Stream::operator=(Stream &s)
{
	if(&s != this)
	{
		if(m_file) m_file->release();
		m_file = s.m_file;
		if(m_file)
		{
			m_stream = s.m_stream;
			m_binstream = s.m_binstream;
			m_file->addRef();
		}
	}
	return *this;
}

//=====================================================================================//
//                       const std::string &Stream::name() const                       //
//=====================================================================================//
const std::string &Stream::name() const
{
	return m_file->name();
}

}	//	namespace Muffle