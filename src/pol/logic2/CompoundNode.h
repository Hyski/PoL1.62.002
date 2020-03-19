#pragma once

//=====================================================================================//
//                                 class CompoundNode                                  //
//=====================================================================================//
template<typename T, typename B = AINode>
class CompoundNode : public B
{
	typedef std::list< boost::shared_ptr<T> > Nodes_t;
	Nodes_t m_nodes;
	typename Nodes_t::iterator m_cur;
	float m_complexity;

protected:
	CompoundNode() : m_cur(m_nodes.end()), m_complexity(0.0f) {}

	typedef boost::shared_ptr<T> Ptr_t;
	typedef typename Nodes_t::iterator Iterator_t;
	typedef typename Nodes_t::const_iterator ConstIterator_t;
	
	Iterator_t insert(const Ptr_t &p) { m_nodes.push_back(p); return ++end(); }
	Iterator_t erase(const Iterator_t &i) { return m_nodes.erase(i); }

	Iterator_t begin() { return m_nodes.begin(); }
	Iterator_t end() { return m_nodes.end(); }

	ConstIterator_t begin() const { return m_nodes.begin(); }
	ConstIterator_t end() const { return m_nodes.end(); }

	Iterator_t getCur() { return m_cur; }

	void resetCur() { m_cur = begin(); }
	void resetComplexity() { m_complexity = 0.0f; }
	void resetState() { resetCur(); resetComplexity(); }

	unsigned int size() const { return m_nodes.size(); }

	void killDeadOnes();
	virtual const char *getEndTurnMessage() const { return "Ход закончен"; }

public:
	virtual void deleteChildren(eid_t ID);
	virtual float Think(state_type *st);
	virtual float getComplexity() const;
};

//=====================================================================================//
//                      void CompoundNode<T,B>::deleteChildren()                       //
//=====================================================================================//
template<typename T, typename B>
inline void CompoundNode<T,B>::deleteChildren(eid_t ID)
{
	if(!ID) return;

	for(Iterator_t i = begin(); i != end();)
	{
		if((*i)->getEntityEID() != ID || !(*i)->die())
		{
			++i;
		}
		else
		{
			if(m_cur == i) ++m_cur;
			i = erase(i);
		}
	}

	// заставим детей сделать тоже самое
	for(Iterator_t i = begin(); i != end(); ++i)
	{
		(*i)->deleteChildren(ID);
	}
}

//=====================================================================================//
//                    inline void CompoundNode<T,B>::killDeadOnes()                    //
//=====================================================================================//
template<typename T, typename B>
inline void CompoundNode<T,B>::killDeadOnes()
{
	for(Iterator_t itor = begin(); itor != end();)
	{
		// удаляем мертвые узлы
		if((*itor)->need2Delete())
		{
			// удалим этот узел
			if(itor == m_cur) ++m_cur;
			itor = erase(itor);
		}
		else
		{
			if(itor != m_cur) (*itor)->Think(0);
			++itor;
		}
	}
}

//=====================================================================================//
//                       inline float CompoundNode<T,B>::Think()                       //
//=====================================================================================//
template<typename T, typename B>
inline float CompoundNode<T,B>::Think(state_type *st)
{
	killDeadOnes();

	if(st == 0) return m_complexity;

	//если обошли все узлы выход
	if(getCur() == end())
	{     
		*st = ST_FINISHED;
		//Print(getEndTurnMessage());
		return m_complexity;
	}

	//дадим подумать текущему узлу
	float tmp_complexity = (*getCur())->Think(st);

	//если узел закончил перейдем к следующему
	if(*st == ST_FINISHED)
	{
		m_complexity += (*getCur())->getComplexity();
		tmp_complexity = 0.0f;
		++m_cur;
	}

	*st = ST_INCOMPLETE;
	return m_complexity + tmp_complexity;
}

//=====================================================================================//
//                inline float CompoundNode<T,B>::getComplexity() const                //
//=====================================================================================//
template<typename T, typename B>
inline float CompoundNode<T,B>::getComplexity() const
{
	float complexity = 0.0f;

	// заставим своих детей рассчитать сложность
	for(ConstIterator_t i = begin(); i != end(); ++i)
	{
		complexity += (*i)->getComplexity();
	}

	return complexity;
}
