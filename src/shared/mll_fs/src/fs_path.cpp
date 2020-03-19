

#include "precomp.h"

#include <locale>
#include <algorithm>

#include <mll/fs/system.h>


namespace mll
{
	
namespace fs
{


//=====================================================================================//
//                                    class fs_path                                    //
//=====================================================================================//
/////////////////////////////////////////////////////////////////////////
fs_path::fs_path()
{
}
/////////////////////////////////////////////////////////////////////////
fs_path::fs_path(const std::string& data)
{
	m_data = data;
	analyzeRawData();
}
/////////////////////////////////////////////////////////////////////////
fs_path::fs_path(const char* data)
{
	m_data = data;
	analyzeRawData();
}
/////////////////////////////////////////////////////////////////////////
fs_path::fs_path(const fs_path& another)
{
	operator=(another);
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator fs_path::begin() const
{
	const_iterator temp;
	temp.m_data = &m_data;
	temp.m_separators = &m_separators;
	temp.m_sepIt = m_separators.begin();
	temp.m_selecting = front();
	return temp;
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator fs_path::end() const
{
	const_iterator temp;
	temp.m_data = &m_data;
	temp.m_separators = &m_separators;
	temp.m_sepIt = m_separators.end();
	return temp;
}
/////////////////////////////////////////////////////////////////////////
std::string fs_path::front() const
{
	int n = m_separators.front();
	return m_data.substr(0, n);
}
/////////////////////////////////////////////////////////////////////////
std::string fs_path::back() const
{
	int n = static_cast<int>(m_data.rfind('/'));
	if (n != m_data.npos)
		return m_data.substr(n+1);
	return m_data;
}
/////////////////////////////////////////////////////////////////////////
void fs_path::pop_front()
{
	int n = m_separators.front();
	m_data.erase(0, n + 1);
	m_separators.clear();
	analyzeRawData();
}
/////////////////////////////////////////////////////////////////////////
void fs_path::pop_back()
{
	int n = static_cast<int>(m_data.rfind('/'));
	if (n != m_data.npos)
		m_data.erase(n);
	else
		m_data.clear();
	m_separators.pop_back();
}
/////////////////////////////////////////////////////////////////////////
int fs_path::length() const
{
	return static_cast<int>(m_separators.size());
}
/////////////////////////////////////////////////////////////////////////
void fs_path::clear()
{
	m_data.clear();
	m_separators.clear();
}
/////////////////////////////////////////////////////////////////////////
const std::string& fs_path::to_str() const
{
	return m_data;
}
/////////////////////////////////////////////////////////////////////////
fs_path& fs_path::operator+=(const fs_path& another)
{
	m_data.append("/");
	m_data.append(another.m_data);
	m_separators.clear();
	analyzeRawData();
	return *this;
}
/////////////////////////////////////////////////////////////////////////
fs_path& fs_path::operator=(const fs_path& another)
{
	m_data = another.m_data;
	m_separators = another.m_separators;
	return *this;
}
/////////////////////////////////////////////////////////////////////////
void fs_path::analyzeRawData()
{
	//	"причесываем"
	std::use_facet<std::ctype<char> >(std::locale()).tolower(&m_data[0], &m_data[m_data.size()]);
	std::replace(m_data.begin(), m_data.end(), '\\', '/');

	std::string::size_type n;

	//	удаляем первый и последний слеши, если они есть
	if (m_data[0] == '/')
		m_data.erase(0, 1);
	n = m_data.length() - 1;
	if (m_data[n] == '/')
		m_data.erase(n);

	n = m_data.find('/');
	while (n != m_data.npos)
	{
		m_separators.push_back(n);
		n = m_data.find('/', n + 1);
	}

	//	после строки "есть" слеш
	if (!m_data.empty())
		m_separators.push_back(static_cast<int>(m_data.length()));
}


//=====================================================================================//
//                                 fs_path operator+()                                 //
//=====================================================================================//
fs_path mll::fs::operator+(const fs_path& first, const fs_path& second)
{
	fs_path temp(first);
	temp += second;
	return temp;
}


//=====================================================================================//
//                            class fs_path::const_iterator                            //
//=====================================================================================//
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator::const_iterator()
:	m_data(0), 
	m_separators(0), 
	m_dataIndex(0)
{
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator::const_iterator(const const_iterator& another)
{
	operator=(another);
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator::~const_iterator()
{
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator& fs_path::const_iterator::operator++()
{
	m_dataIndex = *m_sepIt + 1;
	++m_sepIt;
	m_selecting.clear();
	if (m_sepIt != m_separators->end())
		m_selecting = m_data->substr(m_dataIndex, *m_sepIt - m_dataIndex);
	else
		m_dataIndex = 0;
	return *this;
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator fs_path::const_iterator::operator++(int)
{
	const_iterator a(*this);
	++a;
	return *this;
}

/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator& fs_path::const_iterator::operator--()
{
	--m_sepIt;
	if (m_sepIt != m_separators->begin())
	{
		--m_sepIt;
		m_dataIndex = *m_sepIt + 1;
		++m_sepIt;
	}
	else
		m_dataIndex = 0;
	m_selecting.clear();
	m_selecting = m_data->substr(m_dataIndex, *m_sepIt - m_dataIndex);
	return *this;
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator fs_path::const_iterator::operator--(int)
{
	const_iterator a(*this);
	--a;
	return *this;
}
/////////////////////////////////////////////////////////////////////////
const std::string& fs_path::const_iterator::operator*() const
{
	return m_selecting;
}
/////////////////////////////////////////////////////////////////////////
const std::string* fs_path::const_iterator::operator->() const
{
	return &m_selecting;
}
/////////////////////////////////////////////////////////////////////////
fs_path::const_iterator& fs_path::const_iterator::operator=(const const_iterator& another)
{
	m_data = another.m_data;
	m_separators = another.m_separators;
	m_dataIndex = another.m_dataIndex;
	m_sepIt = another.m_sepIt;
	m_selecting = another.m_selecting;
	return *this;
}

}

}

