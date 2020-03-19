#include "Vis.h"

namespace Vis
{

//=====================================================================================//
//                        void ChannelMerger::operator() const                         //
//=====================================================================================//
void ChannelMerger::operator()(ChannelBase *to, const ChannelBase *from) const
{
	if(to->rtti_dynamic() != from->rtti_dynamic()) return;
	if(to->rtti_dynamic() == ChannelPosition3::rtti_static())
	{
		merge<ChannelPosition3>(to, from);
	}
	else if(to->rtti_dynamic() == ChannelFloat::rtti_static())
	{
		merge<ChannelFloat>(to, from);
	}
	else if(to->rtti_dynamic() == ChannelNormal3::rtti_static())
	{
		merge<ChannelNormal3>(to, from);
	}
	else if(to->rtti_dynamic() == ChannelIndex16::rtti_static())
	{
		merge<ChannelIndex16>(to, from);
	}
	else
	{
		doMerge(to,from);
	}
}

//=====================================================================================//
//                 matrix3 ChannelMerger::getMatrixDifference() const                  //
//=====================================================================================//
matrix3 ChannelMerger::getMatrixDifference(ChannelBase *l, const ChannelBase *r) const
{
	matrix3 result = matrix3::identity();
	Node *lnode = l->getPiece()->getNode();
	Node *rnode = r->getPiece()->getNode();

	if(lnode->getParent() == rnode->getParent())
	{
		result = rnode->getTransformation() * lnode->getTransformation().invert();
	}
	else if(lnode == rnode->getParent())
	{
		result = rnode->getTransformation();
	}

	return result;
}

//=====================================================================================//
//                         void ChannelMerger::doMerge() const                         //
//=====================================================================================//
void ChannelMerger::doMerge(ChannelFloat *l, const ChannelFloat *r) const
{
	ChannelFloat::Data_t new_data(r->getData());
	const matrix3 transform = getMatrixDifference(l,r);

	for(ChannelFloat::Data_t::iterator i = new_data.begin(); i != new_data.end(); ++i)
	{
		*i = (vector3(*i,0.0f,0.0f)*transform).length();
	}

	new_data.insert(new_data.begin(),l->getData().begin(),l->getData().end());
	l->swapData(new_data);
}

//=====================================================================================//
//                         void ChannelMerger::doMerge() const                         //
//=====================================================================================//
void ChannelMerger::doMerge(ChannelPosition3 *l, const ChannelPosition3 *r) const
{
	ChannelPosition3::Data_t new_data(r->getData());
	const matrix3 transform = getMatrixDifference(l,r);

	for(ChannelPosition3::Data_t::iterator i = new_data.begin(); i != new_data.end(); ++i)
	{
		*i = *i * transform;
	}

	new_data.insert(new_data.begin(),l->getData().begin(),l->getData().end());
	l->swapData(new_data);
}

//=====================================================================================//
//                         void ChannelMerger::doMerge() const                         //
//=====================================================================================//
void ChannelMerger::doMerge(ChannelNormal3 *l, const ChannelNormal3 *r) const
{
	ChannelNormal3::Data_t new_data(r->getData());
	const matrix3 transform = getMatrixDifference(l,r);

	for(ChannelNormal3::Data_t::iterator i = new_data.begin(); i != new_data.end(); ++i)
	{
		*i = *i * transform;
	}

	new_data.insert(new_data.begin(),l->getData().begin(),l->getData().end());
	l->swapData(new_data);
}

//=====================================================================================//
//                         void ChannelMerger::doMerge() const                         //
//=====================================================================================//
void ChannelMerger::doMerge(ChannelIndex16 *l, const ChannelIndex16 *r) const
{
	ChannelIndex16::Data_t new_data(r->getData());
	unsigned short max_index = 0;
	if(!l->getData().empty())
	{
		max_index = *std::max_element(l->getData().begin(),l->getData().end()) + 1;
	}

	std::transform(new_data.begin(),new_data.end(),new_data.begin(),
		std::bind2nd(std::plus<unsigned short>(),max_index));
	new_data.insert(new_data.begin(),l->getData().begin(),l->getData().end());
	l->swapData(new_data);
}

//=====================================================================================//
//                         void ChannelMerger::doMerge() const                         //
//=====================================================================================//
void ChannelMerger::doMerge(ChannelBase *l, const ChannelBase *r) const
{
	l->appendDataFrom(r);
}

}