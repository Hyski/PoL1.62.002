#if !defined(__BINARY_STORAGE_INCLUDED__)
#define __BINARY_STORAGE_INCLUDED__

#include <algorithm>
#include <istream>
#include <ostream>
#include <list>

#include <mll/utils/lcrn_tree.h>
#include <mll/io/binstream.h>

namespace mll
{

namespace utils
{

template<typename ID>
class _binary_storage_path;

template<typename ID>
class _binary_storage_node;

class _binary_storage_version;
class _binary_storage_node_data;


template<typename ID>
struct _binary_storage_traits
{
	typedef ID id;
	typedef _binary_storage_path<id> path;
	typedef _binary_storage_version version;
	typedef typename lcrn_tree<_binary_storage_node<id> >::iterator iterator;

protected:

	typedef _binary_storage_node_data node_data_t;
	typedef _binary_storage_node<id> node;
	typedef lcrn_tree<node> bs_tree_t;

};

//=====================================================================================//
//                                class binary_storage                                 //
//=====================================================================================//
template<typename ID>
class binary_storage : public _binary_storage_traits<ID>
{
private:

	bs_tree_t m_bsTree;							//	������				
	version m_version;							//	������

public:
	explicit binary_storage(mll::io::ibinstream& istream);
	binary_storage(const version& v);
	~binary_storage();
public:
	///	��������� �� ������
	void load(mll::io::ibinstream& istream);
	///	�������� � �����
	void save(mll::io::obinstream& ostream);

public:
	///	������� �������� ����
	iterator create_root(const id& _id);
	///	�������� ���� �� ����
	iterator get_node(const path& p);
	///	�������� �������� �� ������
	iterator begin(void);
	///	�������� �������� �� ��������� ���������
	iterator end(void);

public:
	///	������� ����
	void erase(iterator it);
	///	�������� ������
	void clear(void);

public:
	///	�������� ������ 
	const version& get_version(void) const;
	///	�������� ������ 
	version& get_version(void);

};

//=====================================================================================//
//                            class _binary_storage_version                            //
//=====================================================================================//
class _binary_storage_version
{
private:
	unsigned int m_major;					//	�������� ������� �������� ��������������� ������
	unsigned int m_minor;					//	��������� ��������������� ��������� (������ ���������� ����� �����)
public:
	_binary_storage_version() : m_major(0),m_minor(0) {}
	_binary_storage_version(mll::io::ibinstream& istream) : m_major(0),m_minor(0) { load(istream); }
	_binary_storage_version(const _binary_storage_version& v) : m_major(v.m_major),m_minor(v.m_minor) {}
	_binary_storage_version(unsigned int major,unsigned int minor) : m_major(major),m_minor(minor) {}
public:
	///	�������� �����������
	_binary_storage_version& operator=(const _binary_storage_version& v)
	{
		m_major = v.m_major;
		m_minor = v.m_minor;

		return *this;
	}
	///	��������� �� ������������� � ������ �������
	bool is_compatible(const _binary_storage_version& v) const
	{
		return (m_major==v.m_major) && (m_minor>=v.m_minor);
	}
	///	��������� � �����
	void save(mll::io::obinstream& ostream) const 
	{
		ostream << m_major << m_minor;
	}
	///	��������� �� ������
	void load(mll::io::ibinstream& istream)
	{
		istream >> m_major >> m_minor;
	}
	///	�������� ����� ������
	unsigned int major(void) const
	{
		return m_major;
	}
	///	�������� ����� ������
	unsigned int minor(void) const
	{
		return m_minor;
	}

};

//=====================================================================================//
//                              inline bool operator==()                               //
//=====================================================================================//
inline bool operator==(const _binary_storage_version& a,const _binary_storage_version& b)
{
	return (a.major() == b.major()) && (a.minor() == b.minor());
}

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(const _binary_storage_version& a,const _binary_storage_version& b)
{
	if(a.major() < b.major()) return true;
	if(a.major() > b.major()) return false;

	return a.minor() < b.minor();
}

//=====================================================================================//
//                            class _binary_storage_node;                              //
//=====================================================================================//
template<typename ID>
class _binary_storage_node : public _binary_storage_traits<ID>
{
private:

	bs_tree_t& m_bsTree;
	node_data_t m_data;
	iterator m_self;
	id m_id;

public:
	_binary_storage_node(const id& _id,bs_tree_t& bs_tree) : m_id(_id),
														     m_bsTree(bs_tree)
														     {}
	_binary_storage_node(const _binary_storage_node& n) : m_id(n.m_id),
														  m_self(n.m_self),
														  m_bsTree(n.m_bsTree)
														  {}
	~_binary_storage_node() {}
public:
	//	�������� �����������
	_binary_storage_node& operator=(const _binary_storage_node& n)
	{
		if(&n != this)
		{
			m_id = n.m_id;
			m_self = n.m_self;
			m_bsTree = n.m_bsTree;
		}

		return *this
	}
	//	������� �������� ���� (���������� �������� �� ��������� ����)
	iterator create_node(const id& _id)
	{
		iterator it = m_bsTree.insert_son(m_self,node(_id,m_bsTree));
		return it->m_self = it;
	}
	//	��������� ������, ������� � ������� ����
	void save(mll::io::obinstream& out)
	{
		unsigned int counter = 0;
		iterator it;
		//	������������ ���-�� �����
		for(it=first_child();it!=end();it.brother()) counter++;
		//	��������� ������������� ����
		out << m_id;
		//	��������� ������
		m_data.save(out);
		//	��������� ���-�� �����
		out << counter;
		//	��������� �����
		for(it=first_child();it!=end();it.brother())
			it->save(out);
	}
	//	��������� ������, ������� � ������� ����
	void load(mll::io::ibinstream& in)
	{
		unsigned int counter = 0;
		//	��������� ������ ����
		m_data.load(in);
		//	��������� ���-�� �����
		in >> counter;
		//	������� �������� ����
		for(;counter>0;counter--)
		{
			//	��������� ������������� ����
			id _id;
			in >> _id;
			//	��������� � ��������� ����
			create_node(_id)->load(in);
		}
	}
	//	�������� ������������� ����
	const id& get_id() const
	{
		return m_id;
	}
	//	�������� ������ �� ����
	const node_data_t& get_data() const
	{
		return m_data;
	}
	//	�������� ������ �� ����
	node_data_t& get_data(void)
	{
		return m_data;
	}
	//	�������� �������� �� ������� �������
	iterator first_child(void)
	{
		return m_self.get_son();
	}
	//	�������� �������� �� ��������� �������
	iterator end(void)
	{
		return iterator();
	}


	friend binary_storage<ID>;
};

class _binary_storage_node_data
{
private:
	char* m_data;
	unsigned int m_size;
public:
	_binary_storage_node_data() : m_data(0),m_size(0) {}
	_binary_storage_node_data(const _binary_storage_node_data& nd) : m_data(0),m_size(0)
	{
		this->operator=(nd);
	}
	~_binary_storage_node_data() {	delete[] m_data; }
public:
	//	�������� �����������
	_binary_storage_node_data& operator=(const _binary_storage_node_data& nd)
	{
		if(&nd != this) set_data(nd.m_data,nd.m_size);
		return *this;
	}

	//	������� ������
	const void* set_data(const void* data,unsigned int size)
	{
		delete[] m_data; m_data = 0; m_size = 0;

		if(data && size)
		{
			m_size = size;
			m_data = new char[m_size];
			memcpy(m_data,data,m_size);
		}

		return m_data;
	}
	//	�������� ������
	const void* get_data(void) const { return m_data; }
	//	�������� ������ ������
	unsigned int size(void) const { return m_size; }
	//	��������� ������
	void save(mll::io::obinstream& out)
	{
		//	��������� ������ ������ ����
		out << m_size;
		//	��������� ������ ���������� � ����
		out.stream().write(m_data,m_size);
	}
	//	��������� ������
	void load(mll::io::ibinstream& in)
	{
		delete[] m_data; m_data = 0; m_size = 0;
		//	��������� ������ ������ ����
		in >> m_size;
		if(m_size)
		{
			m_data = new char[m_size];
			//	��������� ������ ���������� � ����
			in.stream().read(m_data,m_size);
		}
	}
};

//=====================================================================================//
//                                     class _path                                     //
//=====================================================================================//
template<typename ID>
class _binary_storage_path : public _binary_storage_traits<ID>
{
private:

	typedef std::list<id> path_list_t;
	path_list_t m_path;

public:
	_binary_storage_path() {}
	_binary_storage_path(const _binary_storage_path& p) : m_path(p.m_path) {}
	_binary_storage_path(const id& _id) 
	{ 
		m_path.push_back(_id); 
	}
public:
	//	�������� �����������
	_binary_storage_path& operator=(const _binary_storage_path& p)
	{ 
		m_path = p.m_path;
		return *this;
	}
	//	�������� ������� ���� �� ����
	_binary_storage_path& append(const id& _id) 
	{ 
		m_path.push_back(_id); 
		return *this; 
	}
	//	�������� �������� ���� �����
	_binary_storage_path& operator+=(const _binary_storage_path& p) 
	{ 
		std::copy(p.m_path.begin(),p.m_path.end(),std::back_inserter(m_path));
		return *this; 
	}
	//	�������� ���������
	bool operator==(const _binary_storage_path& p)
	{ 
		return m_path == p.m_path;
	}
	//	�������� ����������
	bool operator!=(const _binary_storage_path& p)
	{ 
		return m_path != p.m_path;
	}

	friend binary_storage<ID>;
};

//=====================================================================================//
//                     inline _binary_storage_path<ID> operator+()                     //
//=====================================================================================//
template<typename ID>	//	�������� �������� ���� �����
inline _binary_storage_path<ID> operator+(const _binary_storage_path<ID>& a,const _binary_storage_path<ID>& b)
{
	_binary_storage_path<ID> res(a);
	return res += b;
}

//=====================================================================================//
//                                class binary_storage                                 //
//=====================================================================================//
template<typename ID>
inline binary_storage<ID>::binary_storage(mll::io::ibinstream& istream)
{
	load(istream);
}

template<typename ID>
inline binary_storage<ID>::binary_storage(const version& v) : m_version(v)
{
}

template<typename ID>
inline binary_storage<ID>::~binary_storage()
{
}

template<typename ID>
inline void binary_storage<ID>::load(mll::io::ibinstream& in)
{
	//	������� ������
	m_bsTree.clear();
	//	��������� ������
	m_version.load(in);
	//	��������� ���� ��������� (������� ������)
	unsigned int state_flag = 0;
	in >> state_flag;
	if(state_flag)
	{//	������� �������� ����
		//	��������� ������������� ����
		id _id;
		in >> _id;
		//	������� ���� � ��������� ������
		create_root(_id)->load(in);
	}
} 

template<typename ID>
inline void binary_storage<ID>::save(mll::io::obinstream& out)
{
	//	��������� ������
	m_version.save(out);
	//	��������� ���� ������� ������ (������ ����� ������������� - �� ������ ��� �����) 
	unsigned int state_flag = m_bsTree.begin()==m_bsTree.end()?0:1;
	out << state_flag;
	if(state_flag)
	{//	��������� ������ ( <����><���-�� �������� �����><�������� ����> )
		m_bsTree.begin()->save(out);
	}
}

template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::create_root(const id& _id)
{
	if(m_bsTree.size()) return m_bsTree.begin();
	iterator it = m_bsTree.insert_son(m_bsTree.end(),node(_id,m_bsTree)); 
	return it->m_self = it;
}

template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::get_node(const path& p)
{
	if(!p.m_path.size() || m_bsTree.empty()) return m_bsTree.end();

	iterator itt = m_bsTree.begin();
	path::path_list_t::const_iterator itl = p.m_path.begin();

	if(!(itt->get_id() == *itl)) return m_bsTree.end();

	for(++itl;itl!=p.m_path.end();++itl)
	{
		for(itt.son();itt!=m_bsTree.end();itt.brother())
		{
			if(itt->get_id() == *itl) break;
		}
		if(itt==m_bsTree.end()) return m_bsTree.end();
	}

	return itt;
}
//	�������� �������� �� ������
template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::begin(void)
{
	return m_bsTree.begin();
}
//	�������� �������� �� ��������� ���������
template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::end(void)
{
	return m_bsTree.end();
}
//	������� ����
template<typename ID>
inline void binary_storage<ID>::erase(iterator it)
{
	m_bsTree.erase(it);
}
//	�������� ������
template<typename ID>
inline void binary_storage<ID>::clear(void)
{
	return m_bsTree.clear();
}
//	�������� ������ 
template<typename ID>
inline const typename binary_storage<ID>::version& binary_storage<ID>::get_version(void) const
{
	return m_version;
}
//	�������� ������ 
template<typename ID>
inline typename binary_storage<ID>::version& binary_storage<ID>::get_version(void)
{
	return m_version;
}


}	//	namespace utils

}	//	namespace mll

#endif