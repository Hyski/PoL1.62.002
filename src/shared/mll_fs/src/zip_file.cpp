
#include "precomp.h"

#include <boost/utility.hpp>
#include <algorithm>
#include <mll/fs/fs_path.h>
#include <mll/fs/mask_regex.h>

#include "zip_input_stream.h"
#include "zip_local_entry.h"
#include "zip_file.h"
#include "zip_eocd.h"

#include <fstream>



using namespace mll::ml_encrypted_zip;


//=====================================================================================//
//                                zip_file::zip_file()                                 //
//=====================================================================================//
zip_file::zip_file(const std::string &filename, const std::string &pass /* = "" */)
:	m_filename(filename), m_dec(pass)
{
	std::ifstream _zipfile(m_filename.c_str(), std::ios::in | std::ios::binary);
	mll::io::ibinstream zipfile(_zipfile);
	find_eocd(zipfile);
	EndOfCentralDirEntry eocd(zipfile.stream());
	m_entries.reserve(eocd.count());
	zipfile.stream().seekg(eocd.CDir_offset(), std::ios_base::beg);
	read_CDir(zipfile.stream());
}

//=====================================================================================//
//                                zip_file::~zip_file()                                //
//=====================================================================================//
zip_file::~zip_file()
{
	std::for_each(m_entries.begin(), m_entries.end(), boost::checked_deleter<zip_entry>());
}

//=====================================================================================//
//              std::auto_ptr<std::istream> zip_file::get_input_stream()               //
//=====================================================================================//
std::auto_ptr<std::istream> zip_file::get_input_stream(const std::string &entry_name,
													 bool seekable)
{
	zip_entry* tmp_entry = 0;
	for (entries_t::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
	{
		if ((*i)->filepath() == entry_name)
			tmp_entry = *i;
	}
	assert(tmp_entry);
	return std::auto_ptr<std::istream>(new zip_input_stream(m_filename, m_dec,
		tmp_entry->local_offset(), seekable));
}









//=====================================================================================//
//                             void zip_file::find_eocd()                              //
//=====================================================================================//
void zip_file::find_eocd(mll::io::ibinstream &file)
{
	zip_local_entry tmp_local_entry;
	zip_entry tmp_CDir_entry;
	
	unsigned files = 0;
	unsigned long signature = 0;
	bool found = false;
	while (!file.eof())
	{
		file >> signature;

		switch (signature)
		{
		case 0x04034b50:	//	local header
			file.stream().seekg(-4, std::ios::cur);
			tmp_local_entry = zip_local_entry(file.stream());
			file.stream().seekg(tmp_local_entry.compressed_size(), std::ios::cur);
			break;
		case 0x08074b50:	//	split/span signature. After it follows data descriptor
			file.stream().seekg(12, std::ios::cur);
			break;
		case 0x02014b50:	//	CDir header
			file.stream().seekg(-4, std::ios::cur);
			tmp_CDir_entry = zip_entry(file.stream());
			break;
		case 0x06054b50:	//	eoCDir header
			file.stream().seekg(-4, std::ios::cur);
			found = true;
			break;
		default:
			assert(0);
			throw damaged_file("ml_encrypted_zip: archive file is damaged!");
		}
		if (found)
			break;
	}
	assert(found);
}

//=====================================================================================//
//                             void zip_file::read_CDir()                              //
//=====================================================================================//
void zip_file::read_CDir(std::istream &file)
{
	for (unsigned i = 0; i < m_entries.capacity(); ++i)
	{
		zip_entry* entry = new zip_entry(file);
		m_entries.push_back(entry);
		std::string full_path = entry->filepath();
		std::string name;
		std::string directory;
		unsigned int n = full_path.rfind('/');
		if (n != std::string::npos)
		{
			name = full_path.substr(n+1);
			directory = full_path.substr(0, n);
		}
		else
			name = full_path;
		if (directory == "missions")
			int q = 1;
		cache_t::iterator it = m_cache.find(directory);
		if (it != m_cache.end())
		{
			local_cache_t& local_cache = it->second;
			assert((local_cache.find(name) == local_cache.end()) && "Большой-большой косяк: попытка вставить в кэш существующий файл");
			local_cache[name] = entry;
		}
		else
		{
			local_cache_t local_cache;
			local_cache[name] = entry;
			m_cache[directory] = local_cache;
		}
	}







	/*	std::string tmp_level_name;
		//	первый элемент - делаем большое количество корней
		bool present = false;
		tmp_level_name = path.front();
		tree_t::iterator it = m_tree.begin();
		for (tree_t::iterator _brother = it; _brother != m_tree.end(); _brother.brother())
		{
			if (_brother->m_levelName == tmp_level_name)	
			{
				present = true;
				it = _brother;
				break;
			}
		}
		if (!present)
		{
			tree_data data(tmp_level_name, 0);
			it = m_tree.insert_son(m_tree.end(), data);
		}
		//	следующие элементы до конца
		for (fs_path::const_iterator path_it = ++path.begin(); path_it != path.end(); ++path_it)
		{
			tmp_level_name = *path_it;
			bool present = false;
			for (tree_t::iterator _son = it.son_begin(); _son != it.son_end(); _son.brother())
			{
				if (_son->m_levelName == tmp_level_name)
				{
					present = true;
					it = _son;
					break;
				}
			}
			if (!present)
			{
				tree_data data(tmp_level_name, 0);
				it = m_tree.insert_son(it, data);
			}
		}
		it->m_entry = entry;
	}*/
}

//=====================================================================================//
//                 const zip_file::entries_t& zip_file::get_entries()                  //
//=====================================================================================//
const zip_file::entries_t& zip_file::get_entries(const std::string& full_path)
{
	m_founded.clear();

	std::string name;
	std::string directory;
	unsigned int n = full_path.rfind('/');
	if (n != std::string::npos)
	{
		name = full_path.substr(n+1);
		directory = full_path.substr(0, n);
	}
	else
		name = full_path;
	
	bool masked = false;
	if (name.find_first_of("?*") != std::string::npos)
		masked = true;
	cache_t::iterator it = m_cache.find(directory);
	if (it != m_cache.end())
	{
		local_cache_t& local_cache = it->second;
		if (masked)
		{
			mll::fs::mask_regex mreg(name);
			for (local_cache_t::iterator it2 = local_cache.begin(); it2 != local_cache.end(); ++it2)
			{
				if (mreg.is_valid(it2->first))
					m_founded.push_back(it2->second);
			}
		}
		else
		{
			local_cache_t::iterator it2 = local_cache.find(name);
			if (it2 != local_cache.end())
				m_founded.push_back(it2->second);
		}
	}
/*	return m_founded;

	fs_path path(search_string);
	std::string tmp_level_name;
	//	первый элемент - из-за большого количества корней
	bool present = false;
	tmp_level_name = path.front();
	tree_t::iterator it = m_tree.begin();
	for (tree_t::iterator _brother = m_tree.begin(); _brother != m_tree.end(); _brother.brother())
	{
		if (_brother->m_levelName == tmp_level_name)	
		{
			present = true;
			it = _brother;
			break;
		}
	}
	if (present)
	{
		bool masked = false;
		std::string mask(path.back());
		if ((mask.find('/') != std::string::npos) || (mask.find('*') != std::string::npos))
			masked = true;
		int not_masked_level = path.length() - 1;
		for (fs_path::const_iterator i = ++path.begin(); i != path.end(); ++i)
		{
			if (masked)
				--not_masked_level;
			tmp_level_name = *i;
			bool founded = false;
			if (!not_masked_level)
			{
				mask_regex mreg(tmp_level_name);		
				for (tree_t::iterator _brother = it.son_begin(); _brother != it.son_end(); _brother.brother())
				{
					if (mreg.is_valid(_brother->m_levelName))
					{
						founded = true;
						it = _brother;
						break;
					}
				}
			}
			else
			{
				for (tree_t::iterator _brother = it.son_begin(); _brother != it.son_end(); _brother.brother())
				{
					if (_brother->m_levelName == tmp_level_name)
					{
						founded = true;
						it = _brother;
						break;
					}
				}
			}
			if (!founded)
				return m_founded;
		}
		//	наполняем m_founded из уровня it, если идет поиск по маске
		if (masked)
		{
			mask_regex mreg(mask);
			for (; it != m_tree.end(); it.brother())
			{
				if ((mreg.is_valid(it->m_levelName)) && (it->m_entry))
					m_founded.push_back(it->m_entry);
			}
		}
		else
		{
			if (it->m_entry)
				m_founded.push_back(it->m_entry);
		}
	}*/
	return m_founded;
}

