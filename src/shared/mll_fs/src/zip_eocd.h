#if !defined(__ZIP_EOCD_H_INCLUDED_7552370787048739__)
#define __ZIP_EOCD_H_INCLUDED_7552370787048739__



namespace mll
{

namespace ml_encrypted_zip
{


#pragma pack(push,1)
//=====================================================================================//
//                               struct zip_EOCD_header                                //
//=====================================================================================//
struct zip_EOCD_header
{
	//	long integer - 4 bytes, short integer - 2 bytes
	unsigned long	m_signature;
	unsigned short	m_curr_disk_num;
	unsigned short	m_CDir_disk_num;
	unsigned short	m_entries_on_curr_disk;
	unsigned short	m_entries_total;
	unsigned long	m_CDir_size;
	unsigned long	m_CDir_start_offset;
	unsigned short	m_comment_length;
};
#pragma pack(pop)








//=====================================================================================//
//                              class zip_endOfCentralDir                              //
//=====================================================================================//
class EndOfCentralDirEntry
{
private:
	zip_EOCD_header m_header;
	std::string m_zip_comment;
	
public:
	EndOfCentralDirEntry(std::istream &file);
	EndOfCentralDirEntry(const EndOfCentralDirEntry &a);
	~EndOfCentralDirEntry() {}

public:
	unsigned count() { return m_header.m_entries_total; }
	unsigned CDir_offset() { return m_header.m_CDir_start_offset; }
	const std::string &zipfile_comment() { return m_zip_comment; }

private:
	friend std::istream &operator >> (std::istream &is, EndOfCentralDirEntry &entry);
};





//=====================================================================================//
//                            std::istream &operator >> ()                             //
//=====================================================================================//
inline std::istream &operator >> (std::istream &is, EndOfCentralDirEntry &entry)
{
	is.read(reinterpret_cast<char *>(&entry.m_header), sizeof(entry.m_header));
	return is;
}

}

}

#endif // !defined(__ZIP_EOCD_H_INCLUDED_7552370787048739__)