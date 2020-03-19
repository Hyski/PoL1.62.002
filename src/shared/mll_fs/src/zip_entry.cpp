
#include "precomp.h"
#include "zip_entry.h"

#include <locale>



namespace mll
{
	
namespace ml_encrypted_zip
{

namespace
{

//=====================================================================================//
//                                std::string tolower()                                //
//=====================================================================================//
std::string tolower(const std::string& str)
{
	std::string temp = str;
	std::use_facet<std::ctype<char> >(std::locale()).tolower(&temp[0], &temp[temp.size()]);
	return temp;
}

}

//=====================================================================================//
//                               zip_entry::zip_entry()                                //
//=====================================================================================//
zip_entry::zip_entry(std::istream &file)
{
	zeroHeader();
	file >> *this;
	assert(m_header.m_signature == 0x02014b50);

	unsigned len, i;
	len = m_header.m_filename_length;
	for (i = 0; i < len; ++i)
		m_filepath.insert(m_filepath.end(), file.get());
	m_filepath = tolower(m_filepath);
	len = m_header.m_extra_field_length;
	for (i = 0; i < len; ++i)
		m_extra.insert(m_extra.end(), file.get());
	len = m_header.m_comment_length;
	for (i = 0; i < len; ++i)
		m_comment.insert(m_comment.end(), file.get());

	tm tmp_time;
	tmp_time.tm_hour = m_header.m_last_mod_time.hours;
	tmp_time.tm_mday = m_header.m_last_mod_date.day;
	tmp_time.tm_min = m_header.m_last_mod_time.minutes;
	tmp_time.tm_mon = m_header.m_last_mod_date.month - 1;
	tmp_time.tm_sec = m_header.m_last_mod_time.seconds * 2;
	tmp_time.tm_year = m_header.m_last_mod_date.year + 80;
	m_date = mktime(&tmp_time);

	assert(m_filepath.size() != 0);
	m_is_dir = (m_filepath[m_filepath.size() - 1] == '/');
	
	if (m_is_dir)
		m_filepath.erase(m_filepath.length()-1, m_filepath.length());
}

//=====================================================================================//
//                               zip_entry::zip_entry()                                //
//=====================================================================================//
zip_entry::zip_entry(const zip_entry &a)
:	m_filepath(a.m_filepath), m_extra(a.m_extra),
	m_header(a.m_header), m_is_dir(a.m_is_dir), m_date(a.m_date)
{
}


//=====================================================================================//
//                          zip_entry &zip_entry::operator=()                          //
//=====================================================================================//
zip_entry &zip_entry::operator=(const zip_entry &a)
{
	if (this != &a)
	{
		this->~zip_entry();
		new(this) zip_entry(a);
	}
	return *this;
}

//=====================================================================================//
//                            void zip_entry::zeroHeader()                             //
//=====================================================================================//
void zip_entry::zeroHeader()
{
	m_header.m_signature = 0;
	m_header.m_version_made = 0;
	m_header.m_version_need = 0;
	m_header.m_genbitflag = 0;
	m_header.m_compression_method = 0;
//	m_header.m_last_mod_time
//	m_header.m_last_mod_date
	m_header.m_crc32 = 0;
	m_header.m_compressed_size = 0;
	m_header.m_uncompressed_size = 0;
	m_header.m_filename_length = 0;
	m_header.m_extra_field_length = 0;
	m_header.m_comment_length = 0;
	m_header.m_disk_number_start = 0;
	m_header.m_internal_attr = 0;
	m_header.m_external_attr = 0;
	m_header.m_local_header_offset = 0;
}

}

}