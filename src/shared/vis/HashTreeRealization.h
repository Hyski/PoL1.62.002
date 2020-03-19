/**********************************************************************************************/
template<typename O>
class lnr_iterator_traverser
	:public HashTreeTraverser<O>
{
public:
	lnr_iterator_traverser(_linear_node_ray_iterator<O> *it, const mll::geometry::ray3 &cur_ray)
		:m_Ray(cur_ray), m_MyIterator(it)
	{};
	order_t visit(HNode node)
	{
//		MLL_PROFILE("lnr_iterator_traverser::visit");
		static const order_t null_order((HNode()), (HNode()));
		const float bad_intersect = mll::geometry::aabb3::bad_result();


		if( node->getBox().intersect(m_Ray) != bad_intersect )
		{
			if(!node->getPositive() && !node->getNegative())
			{
				m_MyIterator->addNode(node);
				return null_order;
			}

			float dotproduct = node->getPlane().m_normal.dot(m_Ray.direction);

			if(node->getPlane().classify(m_Ray.origin) > 0.f )
			{
				if(dotproduct >= 0.f)
					return order_t( node->getPositive(), HNode() );
				else
					return order_t( node->getPositive(), node->getNegative() );
			}
			else
			{
				if(dotproduct <= 0.f)
					return order_t( node->getNegative(), HNode() );
				else
					return order_t( node->getNegative(), node->getPositive() );
			}
		}
		else
			return null_order;
	}
private:
	_linear_node_ray_iterator<O> *m_MyIterator;
	mll::geometry::ray3 m_Ray;
};
/**********************************************************************************************/
template<typename O>
class obj_remove_traverser
	:public HashTreeTraverser<O>
{
public:
	obj_remove_traverser(const O* obj)
		:m_Object(obj)
	{};
	order_t visit(HNode node)
	{
		node->removeObject(m_Object);
		return order_t( node->getNegative(), node->getPositive() );
	}
private:
	const O* m_Object;
};
/**********************************************************************************************/
template<typename O>
class calc_stat_traverser
	:public HashTreeTraverser<O>
{
public:
	calc_stat_traverser()
		:m_NodeNum(0),m_LeafNum(0)
	{};
	order_t visit(HNode node)
	{
		++m_NodeNum;
		if(!node->getPositive() && !node->getNegative())
			++m_LeafNum;

		return order_t( node->getNegative(), node->getPositive() );
	}
public:
	unsigned m_LeafNum;
	unsigned m_NodeNum;
};



template<typename O>
_linear_node_ray_iterator<O>::_linear_node_ray_iterator(HNode Root, const ray& cur_ray)
:m_Ray(cur_ray)
{
//	MLL_PROFILE("_linear_node_ray_iterator()");
	lnr_iterator_traverser<O> traverser(this, m_Ray);
	HashTreeNode<O>::traverse(Root, &traverser);
}





/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
template<typename O>
HashTreeIterator<O>::HashTreeIterator(HNode Root, const ray& ray)
:m_NodeIter(Root,ray)
{
//	MLL_PROFILE("HashTreeIterator()");
	if(!m_NodeIter.isDone())
		getObjectsFromNode();

	if(m_Objects.empty()) next();
}




/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
template<typename O>
HashTreeNode<O>::HashTreeNode(const box_t &box, PNode Parent, const typename HashTree<O>::obj_cont_t &objects)
:m_Bound(box),m_Parent(Parent),
					   m_Items(objects)

{
}

template<typename O>
HashTreeNode<O>::HashTreeNode(const box_t &box, PNode Parent, const plane &p)
:m_Bound(box),m_Parent(Parent), m_Plane(p)
{
}

template<typename O>
void HashTreeNode<O>::traverse(HNode node, HashTreeTraverser<O> *t)
{
	HashTreeTraverser<O>::order_t order = t->visit(node);

	if(order.first)		traverse(order.first, t);
	if(order.second)	traverse(order.second, t);
}


/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
template<typename O>
HashTreeStat HashTree<O>::getStat()
{
	calc_stat_traverser<O> traverser;
	HashTreeNode<O>::traverse(m_Root, &traverser);
	HashTreeStat stat;
	stat.LeafNum = traverser.m_LeafNum;
	stat.NodeNum = traverser.m_NodeNum;
	return stat;
}


template<typename O>
void HashTree<O>::remove(const ObjType *o)
{
	obj_remove_traverser<ObjType> traverser(o);
	HashTreeNode<O>::traverse(m_Root, &traverser);
}

template<typename O>
typename HashTree<O>::iterator HashTree<O>::traceRay(const ray& r)
{
//	MLL_PROFILE("traceray");
	return HashTreeIterator<O>(m_Root, r);
}



/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
template<typename O, typename C>
void ClassifiedHashTree<O,C>::build(const obj_cont_t &objects)
{
//	MLL_PROFILE("tree build");
	mll::geometry::aabb3 box = calcBBox(objects);
	plane p = selectPlane(HNode((Node*)NULL), box);

	m_Root = boost::shared_ptr<Node>( new Node(box, 0, p) );
	rec_build(m_Root, objects);
}

class recursionHolder
{
public:
	recursionHolder(int &i)
		:val(i)
	{++val;}
	~recursionHolder(){--val;}
private:
	int &val;
};

inline void splitBox(const mll::geometry::aabb3&from, const mll::geometry::plane& p, mll::geometry::aabb3* posbox, mll::geometry::aabb3* negbox)
{
	if(p.m_normal == mll::algebra::vector3(0,0,1))
	{
		if(from.min().z> -p.m_distance)
		{
			*posbox=from;
			 negbox->degenerate();
		}
		else if(from.max().z< -p.m_distance)
		{
			*negbox=from;
			 posbox->degenerate();
		}
		else
		{
			posbox->degenerate();
			mll::algebra::point3 pt(from.min());
			pt.z = -p.m_distance;
			posbox->grow(from.max()).grow(pt);
			negbox->degenerate();
			mll::algebra::point3 pt1(from.max());
			pt1.z = -p.m_distance;
			negbox->grow(from.min()).grow(pt1);
		}
	}
	if(p.m_normal == mll::algebra::vector3(0,1,0))
	{
		if(from.min().y> -p.m_distance)
		{
			*posbox=from;
			 negbox->degenerate();
		}
		else if(from.max().y< -p.m_distance)
		{
			*negbox=from;
			 posbox->degenerate();
		}
		else
		{
			posbox->degenerate();
			mll::algebra::point3 pt(from.min());
			pt.y = -p.m_distance;
			posbox->grow(from.max()).grow(pt);
			negbox->degenerate();
			mll::algebra::point3 pt1(from.max());
			pt1.y = -p.m_distance;
			negbox->grow(from.min()).grow(pt1);
		}
	}
	if(p.m_normal == mll::algebra::vector3(1,0,0))
	{
		if(from.min().x> -p.m_distance)
		{
			*posbox=from;
			 negbox->degenerate();
		}
		else if(from.max().x< -p.m_distance)
		{
			*negbox=from;
			 posbox->degenerate();
		}
		else
		{
			posbox->degenerate();
			mll::algebra::point3 pt(from.min());
			pt.x = -p.m_distance;
			posbox->grow(from.max()).grow(pt);
			negbox->degenerate();
			mll::algebra::point3 pt1(from.max());
			pt1.x = -p.m_distance;
			negbox->grow(from.min()).grow(pt1);
		}
	}

}

template<typename O, typename C>
void ClassifiedHashTree<O,C>::rec_build(typename ClassifiedHashTree<O,C>::HNode root, const typename ClassifiedHashTree<O,C>::obj_cont_t &objects)
{
	static const int MAX_RECURSION = 20;
	static int rec = 0;
	recursionHolder __t(rec);
	obj_cont_t pos, neg;
	pos.reserve(objects.size()>>1);
	neg.reserve(objects.size()>>1);
	splitList(objects, root->getPlane(), &pos, &neg);

	mll::geometry::aabb3 posbox, negbox;

	splitBox(root->getBox(), root->getPlane(), &posbox, &negbox);

	//Обработка положительного списка
	{
		size_t size = pos.size();
		//mll::geometry::aabb3 box = calcBBox(pos);
		mll::geometry::aabb3 box = posbox;
		if(box.diag().length()<m_BoxMinSize || pos.size()<m_LeafMinItemsCount || rec>MAX_RECURSION)
		{
			HNode Pos = boost::shared_ptr<Node>(new Node(box, root.get(), pos));
			root->setPositive(Pos);
		}
		else
		{
			plane p =   selectPlane(root,box);
			HNode Pos = boost::shared_ptr<Node>(new Node(box, root.get(), p));
			root->setPositive(Pos);
			rec_build(Pos,pos);
		}
	}
	//Обработка отрицательного списка
	{
		size_t size = neg.size();
		//mll::geometry::aabb3 box = calcBBox(neg);
		mll::geometry::aabb3 box = negbox;
		if(box.diag().length()<m_BoxMinSize || neg.size()<m_LeafMinItemsCount ||rec>MAX_RECURSION)
		{
			HNode Neg = boost::shared_ptr<Node>(new Node(box, root.get(), neg));
			root->setNegative(Neg);
		}
		else
		{
			plane p =   selectPlane(root,box);
			HNode Neg = boost::shared_ptr<Node>(new Node(box, root.get(), p));
			root->setNegative(Neg);
			rec_build(Neg, neg);
		}
	}
}

template<typename O, typename C>
mll::geometry::aabb3 ClassifiedHashTree<O,C>::calcBBox(const obj_cont_t &objects)
{
//	MLL_PROFILE("calc bbox");
	mll::geometry::aabb3 box;
	obj_cont_t::const_iterator it = objects.begin(), ite = objects.end();
	for(; it!=ite; ++it)
	{
		box.grow( m_Functor.getGrowing(*it) );
	}
	return box;
}

template<typename O, typename C>
void ClassifiedHashTree<O,C>::splitList(const obj_cont_t &objects, const plane &plane, obj_cont_t *pos, obj_cont_t *neg)
{
//	MLL_PROFILE("split list");
	obj_cont_t::const_iterator it = objects.begin(), ite = objects.end();
	for(; it!= ite; ++it)
	{
		float dist = m_Functor.getDistance(*it, plane);
		if(dist>=0.f)
			pos->push_back(*it);
		if(dist<=0.f)
			neg->push_back(*it);
	}
}

template<typename O, typename C>
mll::geometry::plane ClassifiedHashTree<O,C>::selectPlane(HNode node, const mll::geometry::aabb3 &box)
{
//	MLL_PROFILE("select plane");
	static const mll::algebra::vector3 PlaneZ = mll::algebra::vector3(0,0,1);
	static const mll::algebra::vector3 PlaneY = mll::algebra::vector3(0,1,0);
	static const mll::algebra::vector3 PlaneX = mll::algebra::vector3(1,0,0);

	mll::algebra::point3 center = box.min()+box.diag()*0.5f;

	if(!node.get() || !node->getParent()) return mll::geometry::plane(PlaneX, center);
	else
	{	
#if 1
		mll::algebra::vector3 norm = node->getParent()->getPlane().m_normal;
		if(norm==PlaneX) return mll::geometry::plane(PlaneY, center);
		if(norm==PlaneY) return mll::geometry::plane(PlaneZ, center);
		if(norm==PlaneZ) return mll::geometry::plane(PlaneX, center);
#else
		float box_max = std::max(std::max(box.diag().y,box.diag().z),box.diag().x);
		if(box_max==box.diag().y) return mll::geometry::plane(PlaneY, center);
		if(box_max==box.diag().z) return mll::geometry::plane(PlaneZ, center);
		if(box_max==box.diag().x) return mll::geometry::plane(PlaneX, center);
#endif
		assert(!"error selecting plane!");
	}
	return mll::geometry::plane(PlaneX, center);
}
