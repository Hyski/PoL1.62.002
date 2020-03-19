#ifndef __HASH_TREE_H_INCLUDED_4278840784508418__
#define __HASH_TREE_H_INCLUDED_4278840784508418__

#include <vector>
#include <list>
#include <map>

#include <boost/shared_ptr.hpp>

#include <mll/geometry/ray3.h>
#include <mll/geometry/aabb3.h>
#include <mll/geometry/plane.h>

template<typename O> class HashTreeNode;
template<typename O> class HashTreeIterator;
template<typename O> class HashTreeTraverser;

struct HashTreeStat
{
	unsigned NodeNum;
	unsigned LeafNum;
};

///основной интерфейс для работы с разбиением пространства
/**
ссылки на объекты размещаются в узлах дерева.
Требование к классу O:
наличие функции float intersect(const mll::geometry::ray3 &)
*/
template<typename O>
class HashTree
{
public:
	typedef O ObjType;
	typedef std::vector<ObjType*>  obj_cont_t;
	typedef mll::geometry::ray3 ray;
	typedef HashTreeNode<O> Node;
	typedef HashTreeIterator<O> iterator;
	typedef boost::shared_ptr<Node> HNode;
public:
	HashTree(){};
	~HashTree(){};

	template<class T> void construct(T begin_iterator, T end_iterator)
	{
		obj_cont_t objects;
		T it = begin_iterator;

		for(; it!=end_iterator; ++it)
			objects.push_back(&*it);
		build(objects);

	}
	virtual void update(const ObjType *o)=0;  ///<не меняет структуру дерева


	void remove(const ObjType *o);

	iterator traceRay(const ray& r);

	HashTreeStat getStat();
protected:
	virtual void build(const obj_cont_t &objects)=0;
protected:
	HNode m_Root;
};

//! Реальное дерево инстанцируется типом объекта и типом-классификатором в пространстве
/**
Требования к функтору C 
д.б. функция float getGrowing(O*) - она возвращает любой тип, которым может расти aabb3
д.б. функция float getDistance(O*, mll::geometry::plane) - она возвращает любой тип, которым может расти aabb3
*/
template<typename O, typename C>
class ClassifiedHashTree
	: public HashTree<O>
{
public:
	ClassifiedHashTree(const C& functor, float BoxMinSize, unsigned LeafMinItemsCount)
		:m_Functor(functor), m_BoxMinSize(BoxMinSize), m_LeafMinItemsCount(LeafMinItemsCount)
	{}
	void update(const ObjType *o){};
protected:
	void build(const obj_cont_t &objects);
private:
	typedef mll::geometry::plane plane;


	mll::geometry::aabb3 calcBBox(const obj_cont_t &objects);
	mll::geometry::plane selectPlane(HNode node, const mll::geometry::aabb3 &box);
	void rec_build(HNode root, const obj_cont_t &objects);
	void splitList(const obj_cont_t &objects, const plane &plane, obj_cont_t *pos, obj_cont_t *neg);

private:
	float m_BoxMinSize;
	unsigned m_LeafMinItemsCount;
	C m_Functor; ///< классификатор объектов в пространстве.
};



//! узел дерева
template<typename O>
class HashTreeNode
{
public:
	typedef mll::geometry::aabb3 box_t;
	typedef mll::geometry::plane plane;
	typedef boost::shared_ptr<HashTreeNode> HNode;
	typedef HashTreeNode* PNode;

public:
	HashTreeNode(const box_t &box, PNode Parent, const typename HashTree<O>::obj_cont_t &objects);
	HashTreeNode(const box_t &box, PNode Parent, const plane &);

	const box_t& getBox()const {return m_Bound;}
	const typename HashTree<O>::obj_cont_t& getObjects() const {return m_Items;}

	PNode getParent()const   {return m_Parent;}
	HNode getPositive()const {return m_Positive;}
	HNode getNegative()const {return m_Negative;}
	const plane& getPlane() const {return m_Plane;}
	static void traverse(HNode node, HashTreeTraverser<O> *t);

	void setPositive(HNode Pos){m_Positive = Pos;};
	void setNegative(HNode Neg){m_Negative = Neg;};

private:
	box_t						m_Bound;
	typename HashTree<O>::obj_cont_t		m_Items;
	plane						m_Plane;
	HNode				m_Positive;
	HNode				m_Negative;
	PNode				m_Parent;
};


template<typename O> class lnr_iterator_traverser;

//! итератор, позволяет последовательно обойти все узлы(листья) дерева, которые пересекаются лучом
/**
коструируется  деревом и лучом, собирает список узлов в отсортированном порядке, далее двигается по этому списку.
Если дерево меняется в момент существования итератора, поведение непредсказуемо (HashTree::update, HashTree::remove)
*/
template<typename O>
class _linear_node_ray_iterator
{
	typedef typename HashTreeNode<O>::HNode HNode;
	typedef typename HashTreeNode<O> Node;
	typedef mll::geometry::ray3 ray;
public:
	//friend class lnr_iterator_traverser;
public:
	_linear_node_ray_iterator(HNode Root, const ray& cur_ray);
	void next()			{if(!m_Nodes.empty()) m_Nodes.erase(m_Nodes.begin());}
	bool isDone() const {return m_Nodes.empty();}
	Node& operator*()   {return m_Nodes.begin();}
	Node* operator->()  {return m_Nodes.begin()->get();}
	const ray& getRay() const {return m_Ray;}
public: ///Внимание! под интелом здесь не получается сделать protected
	void addNode(HNode node) 
	{
		m_Nodes.push_back(node);
	}
private:
	ray      m_Ray;
	std::vector<HNode> m_Nodes; ///< сортированный список узлов по которым необходимо пройти
};


//! итератор для обхода всех объектов, которые попадают на луч
/**
Если дерево меняется в момент существования итератора, поведение непредсказуемо (HashTree::update, HashTree::remove)
*/
template<typename O>
class HashTreeIterator
{
	typedef typename HashTreeNode<O>::HNode HNode;
	typedef HashTreeNode<O> Node;
	typedef _linear_node_ray_iterator<O> node_iterator;
	typedef mll::geometry::ray3 ray;
	typedef std::multimap<float, O*> obj_vec_t;
public:
	HashTreeIterator(HNode Root, const ray& ray);
	O& operator*()  {assert(!m_Objects.empty()); return *(m_Objects.begin()->second); }
	O* operator->() {assert(!m_Objects.empty()); return m_Objects.begin()->second;};
	float distance(){assert(!m_Objects.empty()); return m_Objects.begin()->first;};
	void next() 
	{
		//MLL_PROFILE("HashTreeIterator::next");

		if(!m_Objects.empty())
			m_Objects.erase(m_Objects.begin());

		while(m_Objects.empty() && !m_NodeIter.isDone())
			{
				m_NodeIter.next();
				if(!m_NodeIter.isDone()) getObjectsFromNode();

			}
	}
	bool isDone() const {return m_Objects.empty() && m_NodeIter.isDone();}
private:
	void getObjectsFromNode()
	{
		//assert(!m_NodeIter.isDone());
				HashTree<O>::obj_cont_t::const_iterator it = m_NodeIter->getObjects().begin(),ite = m_NodeIter->getObjects().end();
				for(;it!=ite;++it)
				{
					float dist = (*it)->intersect(m_NodeIter.getRay());
					 if(dist >=0 )
					 {
						 m_Objects.insert(std::make_pair(dist,*it));
					 }

				}
	}
private:
	node_iterator		m_NodeIter;
	obj_vec_t			m_Objects;	///< список объектов в этом узле, которые еще не обошли (первый == текущий)
};

template<typename O>
class HashTreeTraverser
{
public:
	typedef typename HashTreeNode<O>::HNode HNode;
	typedef HashTreeNode<O> Node;
	typedef std::pair<HNode, HNode> order_t;
public:
	virtual order_t visit(HNode node)=0; ///<возвращает пару, которую нужно обойти в указанном порядке
};


#include "HashTreeRealization.h"


#endif //__HASH_TREE_H_INCLUDED_4278840784508418__
