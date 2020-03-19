#if !defined(__ZIP_LOCAL_ENTRY_H_INCLUDED_5992692519605117__)
#define __ZIP_LOCAL_ENTRY_H_INCLUDED_5992692519605117__


#include "dos_datetime.h"


namespace mll
{

namespace ml_encrypted_zip
{


#pragma pack(push,1)
//=====================================================================================//
//                               struct zip_local_header                               //
//=====================================================================================//
struct zip_local_header
{
	//	long integer - 4 bytes, short integer - 2 bytes
	unsigned long	m_signature;
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
};
#pragma pack(pop)




//=====================================================================================//
//                                class zip_local_entry                                //
//=====================================================================================//
class zip_local_entry
{
private:
	zip_local_header m_header;
	std::string m_filepath, m_extra;

public:
	zip_local_entry() {}
	zip_local_entry(std::istream &file);
	zip_local_entry(const zip_local_entry &another);
	~zip_local_entry() {}

	zip_local_entry &operator=(const zip_local_entry &a);

public:
	unsigned compressed_size() const { return m_header.m_compressed_size; }
	unsigned uncompressed_size() const { return m_header.m_uncompressed_size; }
	unsigned compression_method() const { return m_header.m_compression_method; }

private:
	friend std::istream &operator >> (std::istream &is, zip_local_entry &entry);
};


//=====================================================================================//
//                            std::istream &operator >> ()                             //
//=====================================================================================//
inline std::istream &operator >> (std::istream &is, zip_local_entry &entry)
{
	is.read(reinterpret_cast<char *>(&entry.m_header), sizeof(entry.m_header));
	return is;
}

}

}

#endif // !defined(__ZIP_LOCAL_ENTRY_H_INCLUDED_5992692519605117__)