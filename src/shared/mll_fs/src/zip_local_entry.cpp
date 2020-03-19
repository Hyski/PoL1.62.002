#pragma warning (disable: 4786)

#include "precomp.h"

#include "zip_local_entry.h"



using namespace mll::ml_encrypted_zip;


//=====================================================================================//
//                         zip_local_entry::zip_local_entry()                          //
//=====================================================================================//
zip_local_entry::zip_local_entry(std::istream &file)
{
	file >> *this;
	assert(m_header.m_signature == 0x04034b50);

	unsigned len, i;
	len = m_header.m_filename_length;
	for (i = 0; i < len; ++i)
		m_filepath.insert(m_filepath.end(), file.get());
	len = m_header.m_extra_field_length;
	for (i = 0; i < len; ++i)
		m_extra.insert(m_extra.end(), file.get());
}


//=====================================================================================//
//                         zip_local_entry::zip_local_entry()                          //
//=====================================================================================//
zip_local_entry::zip_local_entry(const zip_local_entry &a)
:	m_filepath(a.m_filepath), m_extra(a.m_extra), m_header(a.m_header)
{
}


//=====================================================================================//
//                    zip_local_entry &zip_local_entry::operator=()                    //
//=====================================================================================//
zip_local_entry &zip_local_entry::operator=(const zip_local_entry &a)
{
	if (this != &a)
	{
		this->~zip_local_entry();
		new(this) zip_local_entry(a);
	}
	return *this;
}