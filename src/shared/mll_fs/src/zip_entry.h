#if !defined(__ZIP_ENTRY_H_INCLUDED_5992692519605117__)
#define __ZIP_ENTRY_H_INCLUDED_5992692519605117__


#include "dos_datetime.h"

#include <iostream>


namespace mll
{

namespace ml_encrypted_zip
{


#pragma pack(push,1)
//=====================================================================================//
//                              struct zip_central_header                              //
//=====================================================================================//
struct zip_central_header
{
	//	long integer - 4 bytes, short integer - 2 bytes
	unsigned long	m_signature;
	unsigned short	m_version_made;
	unsigned short	m_version_need;
	unsigned short	m_genbitflag;
	unsigned short	m_compression_method;
	dos_time		m_last_mod_time;
	dos_date		m_last_mod_date;
	unsigned long	m_crc32;
	unsigned long	m_compressed_size;
	unsigned long	m_uncompressed_size;
	unsigned short	m_filename_length;
	unsigned short	m_extra_field_length;
	unsigned short	m_comment_length;
	unsigned short	m_disk_number_start;
	unsigned short	m_internal_attr;
	unsigned long	m_external_attr;
	unsigned long	m_local_header_offset;
};
#pragma pack(pop)






//=====================================================================================//
//                                   class zip_entry                                   //
//=====================================================================================//
class zip_entry
{
private:
	std::string m_filepath, m_extra, m_comment;
	zip_central_header m_header;
	bool m_is_dir;
	time_t m_date;
	
public:
	zip_entry() {}
	zip_entry(std::istream &file);
	zip_entry(const zip_entry &another);
	~zip_entry() {}

	zip_entry &operator=(const zip_entry &a);

public:
	const std::string& filepath() const;
	const std::string& extra_data() const;
	const std::string& comment() const;
	unsigned int size() const;
	time_t date() const;
	bool is_dir() const;
	unsigned int local_offset() const;

private:
	friend std::istream &operator >> (std::istream &is, zip_entry &entry);
	void zeroHeader();
};

////////////////////////////////////////////////////////
inline const std::string& zip_entry::filepath() const
{
	return m_filepath;
}
////////////////////////////////////////////////////////
inline const std::string& zip_entry::extra_data() const
{
	return m_extra;
}
////////////////////////////////////////////////////////
inline const std::string& zip_entry::comment() const
{
	return m_comment;
}
////////////////////////////////////////////////////////
inline unsigned int zip_entry::size() const
{
	return m_header.m_uncompressed_size;
}
////////////////////////////////////////////////////////
inline time_t zip_entry::date() const
{
	return m_date;
}
////////////////////////////////////////////////////////
inline bool zip_entry::is_dir() const
{
	return m_is_dir;
}
////////////////////////////////////////////////////////
inline unsigned int zip_entry::local_offset() const
{
	return m_header.m_local_header_offset;
}


//=====================================================================================//
//                            std::istream &operator >> ()                             //
//=====================================================================================//
inline std::istream &operator >> (std::istream &is, zip_entry &entry)
{
	is.read(reinterpret_cast<char *>(&entry.m_header), sizeof(entry.m_header));
	return is;
}


}

}

#endif // !defined(__ZIP_ENTRY_H_INCLUDED_5992692519605117__)