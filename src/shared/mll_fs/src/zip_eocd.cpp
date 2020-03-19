
#include "precomp.h"

#include "zip_eocd.h"



using namespace mll::ml_encrypted_zip;


//=====================================================================================//
//                    EndOfCentralDirEntry::EndOfCentralDirEntry()                     //
//=====================================================================================//
EndOfCentralDirEntry::EndOfCentralDirEntry(std::istream &file)
{
	file >> *this;
	assert(m_header.m_signature == 0x06054b50);

	unsigned len, i;
	len = m_header.m_comment_length;
	for (i = 0; i < len; ++i)
		m_zip_comment.insert(m_zip_comment.begin(), file.get());
}


//=====================================================================================//
//                    EndOfCentralDirEntry::EndOfCentralDirEntry()                     //
//=====================================================================================//
EndOfCentralDirEntry::EndOfCentralDirEntry(const EndOfCentralDirEntry &a)
:	m_header(a.m_header), m_zip_comment(a.m_zip_comment)
{
}