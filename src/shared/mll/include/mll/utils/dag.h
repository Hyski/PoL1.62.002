#if !defined(__DAG_INCLUDED__)
#define __DAG_INCLUDED__

#include <list>

namespace mll
{

namespace utils
{

template<typename V,typename E>
class dag_vertex_traverser;

template<typename V,typename E>
class dag_vertex_iterator;

template<typename V,typename E>
class dag_iedge_iterator;

template<typename V,typename E>
class dag_oedge_iterator;

template<typename V,typename E>
class dag_vertex;

template<typename V,typename E>
class dag_edge;

template<typename V,typename E>
class dag_traits
{
public:

	typedef dag_vertex_iterator<V,E> vertex_iterator;
	typedef dag_iedge_iterator<V,E> iedge_iterator;
	typedef dag_oedge_iterator<V,E> oedge_iterator;
	typedef dag_oedge_iterator<V,E> edge_iterator;
	typedef dag_vertex_traverser<V,E> traverser;

	typedef V vertex_type;
	typedef E edge_type;

protected:

	typedef std::list<dag_vertex<V,E> > vertices_t;
	typedef std::list<dag_edge<V,E> > edges_t;

	typedef edges_t::iterator edge_reference_t;
	typedef std::list<edge_reference_t> edge_references_t;
};


/////////////////////////////////////////////////////////////////////////
////////////////////////////    class dag    ////////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag : public dag_traits<V,E>
{
public:
	dag();
	~dag();
	//	�������� ������� (���������� �������� �� ��������� �������)		   
	vertex_iterator insert_vertex(const V& v);
	//	������� ������� (��������� ��� ����� �������� � ��������� �� �������)
	void remove_vertex(vertex_iterator& it);
	//	�������� ����� (���������� �������� �� �����)
	edge_iterator insert_edge(const E& e,vertex_iterator& from,vertex_iterator& to);
	//	������� �����
	void remove_edge(edge_iterator& it);
	//	�������� ���-�� ������ � �����
	unsigned int size(void) const;
	//	�������� �������� �� ������ �������
	vertex_iterator begin(void);
	// �������� �������� ��������
	vertex_iterator end(void);
	//	�������� ���-�� ����� � �����
	unsigned int edge_size(void) const;
	//	�������� �������� �� ������ �����
	edge_iterator ebegin(void);
	// �������� �������� �����
	edge_iterator eend(void);
	//	�������� ����
	void clear(void);

private:

	vertices_t m_vertices;						//	������ ������
	edges_t m_oedges;							//	������ ��������� �����
	edge_references_t m_iedges;					//	������ ������ �� �������� �����

};

/////////////////////////////////////////////////////////////////////////
//////////////////////    class vertex_iterator    //////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_vertex_iterator : public dag_traits<V,E>
{
public:

	dag_vertex_iterator() {}

	dag_vertex_iterator(const dag_vertex_iterator& it) : m_value(it.m_value) {}

	virtual ~dag_vertex_iterator() {}

	dag_vertex_iterator& operator=(const dag_vertex_iterator& it)
	{
		m_value = it.m_value;
		return *this;
	}

	vertex_type& operator*(void) { return m_value->get_data(); }

	vertex_type* operator->(void) { return &m_value->get_data(); }

	bool operator==(const vertex_iterator& it) { return m_value == it.m_value; }

	bool operator!=(const vertex_iterator& it) { return m_value != it.m_value; } 

	//=======================================================================
	//	�������� ��������� �������� �������� �����
	//=======================================================================

	iedge_iterator ibegin(void) { return m_value->ibegin(); }

	//=======================================================================
	//	�������� �������� �������� �������� �����
	//=======================================================================

	iedge_iterator iend(void) { return m_value->iend(); }

	//=======================================================================
	//	�������� ��������� �������� ��������� �����
	//=======================================================================

	oedge_iterator obegin(void) { return m_value->obegin(); }

	//=======================================================================
	//	�������� �������� �������� ��������� �����
	//=======================================================================

	oedge_iterator oend(void) { return m_value->oend(); }

protected:

	explicit dag_vertex_iterator(vertices_t::iterator& it) : m_value(it) {}

	vertices_t::iterator m_value;
	friend class dag<V,E>;
};

/////////////////////////////////////////////////////////////////////////
/////////////////////    class dag_oedge_iterator    /////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_oedge_iterator : public dag_traits<V,E>
{
private:
	edges_t::iterator m_value;
public:

	explicit dag_oedge_iterator(edges_t::iterator& it) : m_value(it) {}

	dag_oedge_iterator() {}

	dag_oedge_iterator(const dag_oedge_iterator& it) : m_value(it.m_value) {}

	~dag_oedge_iterator() {}

	dag_oedge_iterator& operator=(const dag_oedge_iterator& it)
	{
		m_value = it.m_value;
		return *this;
	}

	edge_type& operator*(void) { return m_value->get_data(); }

	edge_type* operator->(void) { return &m_value->get_data(); }

	bool operator==(const dag_oedge_iterator& it) { return m_value == it.m_value; }

	bool operator!=(const dag_oedge_iterator& it) { return m_value != it.m_value; }

	dag_oedge_iterator& operator++()
	{
		++m_value;
		return *this;
	}
	
	dag_oedge_iterator operator++(int)
	{
		dag_oedge_iterator it = *this;
		++(*this);
		return it;
	}

	vertex_iterator from(void) { return m_value->from(); }

	vertex_iterator to(void) { return m_value->to(); }

	friend class dag<V,E>;
};

/////////////////////////////////////////////////////////////////////////
/////////////////////    class dag_iedge_iterator    /////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_iedge_iterator : public dag_traits<V,E>
{
private:
	edge_references_t::iterator m_value;
public:

	explicit dag_iedge_iterator(edge_references_t::iterator& it) : m_value(it) {}

	dag_iedge_iterator() {}

	dag_iedge_iterator(const dag_iedge_iterator& it) : m_value(it.m_value) {}

	~dag_iedge_iterator() {}

	dag_iedge_iterator& operator=(const dag_iedge_iterator& it)
	{
		m_value = it.m_value;
		return *this;
	}

	edge_type& operator*(void) { return (*m_value)->get_data(); }

	edge_type* operator->(void) { return &(*m_value)->get_data(); }

	bool operator==(const dag_iedge_iterator& it) { return m_value == it.m_value; }

	bool operator!=(const dag_iedge_iterator& it) { return m_value != it.m_value; }

	dag_iedge_iterator& operator++()
	{
		++m_value;
		return *this;
	}
	
	dag_iedge_iterator operator++(int)
	{
		dag_iedge_iterator it = *this;
		++(*this);
		return it;
	}

	vertex_iterator from(void) { return (*m_value)->from(); }

	vertex_iterator to(void) { return (*m_value)->to(); }

	dag_oedge_iterator<V,E> oedge(void) { return dag_oedge_iterator<V,E>((*m_value)); }

	friend class dag<V,E>;
};

/////////////////////////////////////////////////////////////////////////
////////////////////    class dag_vertex_traverser    ///////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_vertex_traverser : public dag_vertex_iterator<V,E>
{
public:

	dag_vertex_traverser() : dag_vertex_iterator<V,E>() {}
	dag_vertex_traverser(const dag_vertex_iterator<V,E>& it) : dag_vertex_iterator<V,E>(it) {}
	dag_vertex_traverser(const dag_vertex_traverser<V,E>& it) : dag_vertex_iterator<V,E>(it) {}

	~dag_vertex_traverser() {}

	dag_vertex_traverser& operator=(const dag_vertex_traverser& it)
	{
		m_value = it.m_value;
		return *this;
	}

	dag_vertex_traverser& operator++()
	{
		++m_value;
		return *this;
	}
	
	dag_vertex_traverser operator++(int)
	{
		dag_vertex_traverser it = *this;
		++(*this);
		return it;
	}

};

/////////////////////////////////////////////////////////////////////////
////////////////////////////    class edge    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_edge : public dag_traits<V,E>
{
	edge_type m_data;							//	������
	
	vertex_iterator m_from;						//	��������� �������
	vertex_iterator m_to;						//	�������� �������
	
public:
	
	dag_edge(const edge_type& data,vertex_iterator& from,vertex_iterator& to);
	
	~dag_edge();
	
	edge_type& get_data(void);
	
	vertex_iterator& from(void);
	
	vertex_iterator& to(void);
};
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class vertex    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
class dag_vertex : public dag_traits<V,E>
{

	vertex_type m_data;							//	������

	edge_references_t::iterator m_ibegin;		//	��������� �������� �� �������� �����
	edge_references_t::iterator m_iend;			//	�������� �������� �� �������� �����

	edge_references_t& m_iedges;
	
	edges_t::iterator m_obegin;					//	��������� �������� �� ��������� �����
	edges_t::iterator m_oend;					//	�������� �������� �� ��������� �����
		
	edges_t& m_oedges;

public:
		
	//	����������� � ����������
	dag_vertex(const vertex_type& data,
			   edge_references_t& iedges,
			   edges_t& oedges);
		
	//	����������
	~dag_vertex();
		
	vertex_type& get_data(void);

	edge_references_t::iterator ibegin(void) {return m_ibegin;}
	edges_t::iterator obegin(void) {return m_obegin;}

	edge_references_t::iterator iend(void) 
	{
		if(m_iedges.end() == m_iend) return m_iend;
		else
		{
			edge_references_t::iterator tmp = m_iend;
			return ++tmp;
		}
	}

	edges_t::iterator oend(void)
	{
		if(m_oedges.end() == m_oend) return m_oend;
		else
		{
			edges_t::iterator tmp = m_oend;
		
			return ++tmp;
		}
	}

	const edge_references_t::iterator& iend_mm(void) const { return m_iend; }
	const edges_t::iterator& oend_mm(void) const { return m_oend; }

	void set_ibegin(edge_references_t::iterator it)
	{ 
		edge_references_t::iterator tmp = m_iend;
		if((m_iedges.end() != m_iend) && (it == ++tmp))
		{
			m_iend = m_ibegin = m_iedges.end();
		}
		else m_ibegin = it;
	}
	void set_iend(edge_references_t::iterator it) { m_iend = it; }
	void set_obegin(edges_t::iterator it)
	{
		edges_t::iterator tmp = m_oend;
		if((m_oedges.end() != m_oend) && (it == ++tmp))
		{
			m_oend = m_obegin = m_oedges.end();
		}
		else m_obegin = it;
	}
	void set_oend(edges_t::iterator it) { m_oend = it; }

		
};
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
////////////////////////////    class dag    ////////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
dag<V,E>::dag()
{
}

template<typename V,typename E>
dag<V,E>::~dag()
{
}

template<typename V,typename E>
inline dag_vertex_iterator<V,E> dag<V,E>::insert_vertex(const V& v)
{
	m_vertices.insert(m_vertices.end(),dag_vertex<V,E>(v,m_iedges,m_oedges));
	return dag_vertex_iterator<V,E>(--m_vertices.rbegin().base());
}

template<typename V,typename E>
inline void dag<V,E>::remove_vertex(vertex_iterator& it)
{
	while(it.ibegin()!=it.iend())
		remove_edge(it.ibegin().oedge());
	while(it.obegin()!=it.oend())
		remove_edge(it.obegin());

	m_vertices.erase(it.m_value);
}

template<typename V,typename E>
inline dag_oedge_iterator<V,E> dag<V,E>::insert_edge(const E& e,vertex_iterator& from,vertex_iterator& to)
{
	edges_t::iterator it_e = m_oedges.insert(from.m_value->oend(),dag_edge<V,E>(e,from,to));
	if(from.m_value->obegin() == from.m_value->oend()) from.m_value->set_obegin(it_e);
	from.m_value->set_oend(it_e);

	edge_references_t::iterator it_re = m_iedges.insert(to.m_value->iend(),it_e);
	if(to.m_value->ibegin() == to.m_value->iend()) to.m_value->set_ibegin(it_re);
	to.m_value->set_iend(it_re);

	return dag_oedge_iterator<V,E>(it_e);
}

template<typename V,typename E>
inline void dag<V,E>::remove_edge(edge_iterator& it)
{
	if(it.m_value != m_oedges.end())
	{//	�������������� �������� 

		vertices_t::iterator from_value = it.from().m_value; 
		vertices_t::iterator to_value = it.to().m_value; 
		
		if(from_value->obegin() == it.m_value)
		{
			from_value->set_obegin(++from_value->obegin());
		}
		else
		{//	�������� �� ���������� � �������� ����������
			if(from_value->oend_mm() == it.m_value)
			{
				edges_t::reverse_iterator it_r(it.m_value);
				from_value->set_oend((++it_r).base());
			}
		}
		
		if((*to_value->ibegin()) == it.m_value)
		{
			edge_references_t::iterator rit = to_value->ibegin();
			
			to_value->set_ibegin(++to_value->ibegin());
			
			m_iedges.erase(rit);
		}
		else
		{//	�������� �� ���������� � �������� ����������
			if((*to_value->iend_mm()) == it.m_value)
			{
				edge_references_t::reverse_iterator rit_r(to_value->iend_mm());
				to_value->set_iend((++rit_r).base());
			}
		}

		m_oedges.erase(it.m_value);
	}
}

template<typename V,typename E>
inline dag_vertex_iterator<V,E> dag<V,E>::begin(void)
{
	return vertex_iterator(m_vertices.begin());
}

template<typename V,typename E>
inline dag_vertex_iterator<V,E> dag<V,E>::end(void)
{
	return vertex_iterator(m_vertices.end());
}
//	�������� ���-�� ������ � �����
template<typename V,typename E>
inline unsigned int dag<V,E>::size(void) const
{
	return m_vertices.size();
}
//	�������� ���-�� ����� � �����
template<typename V,typename E>
inline unsigned int dag<V,E>::edge_size(void) const
{
	return m_oedges.size();
}
//	�������� �������� �� ������ �����
template<typename V,typename E>
inline dag<V,E>::edge_iterator dag<V,E>::ebegin(void)
{
	return m_oedges.begin();
}
// �������� �������� �����
template<typename V,typename E>
inline dag<V,E>::edge_iterator dag<V,E>::eend(void)
{
	return m_oedges.end();
}
//	�������� ����
template<typename V,typename E>
inline void dag<V,E>::clear(void)
{
	m_vertices.clear();
}

/////////////////////////////////////////////////////////////////////////
////////////////////////////    class edge    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>	
inline dag_edge<V,E>::dag_edge(const edge_type& data,vertex_iterator& from,vertex_iterator& to) : m_data(data),
																								  m_from(from),
																								  m_to(to)
{
}
	
template<typename V,typename E>	
inline dag_edge<V,E>::~dag_edge()
{
}
	
template<typename V,typename E>	
inline E& dag_edge<V,E>::get_data(void)
{
	return m_data;
}
template<typename V,typename E>	
inline dag_vertex_iterator<V,E>& dag_edge<V,E>::from(void)
{
	return m_from;
}
	
template<typename V,typename E>	
inline dag_vertex_iterator<V,E>& dag_edge<V,E>::to(void)
{
	return m_to;
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class vertex    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename V,typename E>
inline dag_vertex<V,E>::dag_vertex(const vertex_type& data,
								   edge_references_t& iedges,
								   edges_t& oedges) : m_data(data),
													  m_iedges(iedges),
													  m_oedges(oedges),
													  m_ibegin(iedges.end()),
													  m_iend(iedges.end()),
													  m_obegin(oedges.end()),
													  m_oend(oedges.end())
{
}

template<typename V,typename E>
inline dag_vertex<V,E>::~dag_vertex()
{
}

template<typename V,typename E>
inline V& dag_vertex<V,E>::get_data(void)
{
	return m_data;
}

/////////////////////////////////////////////////////////////////////////

}	// namespace utils

}	// namespace mll

#endif