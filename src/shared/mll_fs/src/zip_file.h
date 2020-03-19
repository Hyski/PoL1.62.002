#if !defined(__ZIP_FILE_H_INCLUDED_8093570542662288__)
#define __ZIP_FILE_H_INCLUDED_8093570542662288__


#include "zip_entry.h"
#include "decrypter.h"

#include <mll/debug/exception.h>
#include <mll/io/binstream.h>
#include <hash_map>
#include <vector>



namespace mll
{

namespace ml_encrypted_zip
{

using namespace mll::io;


//=====================================================================================//
//                                   class zip_file                                    //
//=====================================================================================//
class zip_file
{
public:
	typedef std::vector<zip_entry*> entries_t;

private:
	typedef std::hash_map<std::string, zip_entry*> local_cache_t;
	typedef std::hash_map<std::string, local_cache_t> cache_t;
	cache_t m_cache;

private:
	entries_t m_entries;
	entries_t m_founded;
	std::string m_filename;
	decrypter m_dec;

private:
	void find_eocd(ibinstream &file);
	void read_CDir(std::istream &file);
	
public:
	zip_file(const std::string &filename, const std::string &pass = "");
	~zip_file();

public:
	std::auto_ptr<std::istream> get_input_stream(const std::string &entry_name,
		bool seekable = false);
	///	получить список файлов, удовлетворяющих переданной маске. Дальнейшее хранение этого массива внутри zip_file не гарантируется
	const entries_t &get_entries(const std::string& search_string);
};


typedef mll::debug::exception_template<struct zip_failure> failure;
typedef mll::debug::exception_template<struct zip_unknown_method, failure> unknown_method;
typedef mll::debug::exception_template<struct zip_damaged_file, failure> damaged_file;
typedef mll::debug::exception_template<struct zip_zlib_problem, failure> zlib_problem;
typedef mll::debug::exception_template<struct zip_buf_restrict, failure> buf_restrict;
typedef mll::debug::exception_template<struct zip_no_seekable, failure> no_seekable;
typedef mll::debug::exception_template<struct zip_seek_error, failure> seek_error;


}

}

#endif // !defined(__ZIP_FILE_H_INCLUDED_8093570542662288__)