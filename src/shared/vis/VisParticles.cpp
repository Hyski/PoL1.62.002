#include "Vis.h"

namespace Vis
{

ML_PERSISTENT_IMPL2(Particles,VIS_BASIC_PTAG);

//=====================================================================================//
//                                   Shape::Shape()                                    //
//=====================================================================================//
Particles::Particles()
{
}

//=====================================================================================//
//                                   Shape::Shape()                                    //
//=====================================================================================//
Particles::Particles(const Particles &s)
:	Piece(s)
{
}

//=====================================================================================//
//                               Particles::Particles()                                //
//=====================================================================================//
Particles::Particles(Storage::ReadSlot &slot)
:	Piece(slot)
{
}

//=====================================================================================//
//                                   Shape::Shape()                                    //
//=====================================================================================//
Particles::Particles(const std::string &name, Node *parent, const OptionSet &options)
:	Piece(name,parent,options)
{
}

//=====================================================================================//
//                                   Shape::~Shape()                                   //
//=====================================================================================//
Particles::~Particles()
{
}

//=====================================================================================//
//                           bool Shape::isIdentical() const                           //
//=====================================================================================//
bool Particles::isIdentical(const Piece *piece) const
{
	if(piece->rtti_dynamic() != rtti_dynamic()) return false;
	if(piece->getMaterial() != getMaterial()) return false;
	return true;
}

//=====================================================================================//
//                             Piece *Shape::clone() const                             //
//=====================================================================================//
Particles *Particles::clone(Node *newParent, Storage::Repository *rep) const
{
	Particles *copy = rep->construct<Particles>(getName(),newParent,getOptions());
	copy->setMaterial(getMaterial());
	return copy;
}


}