#if !defined(__VIS_SHAPE_H_INCLUDED_1259698041368663__)
#define __VIS_SHAPE_H_INCLUDED_1259698041368663__

#include "VisPiece.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class Material;

//=====================================================================================//
//                             class Shape : public Piece                              //
//=====================================================================================//
class VIS_IMPORT Shape : public Piece
{
public:
	ML_RTTI_CUSTOM(Shape,"[Vis.Shape]",Piece);
	ML_PERSISTENT2(Shape,VIS_BASIC_PTAG);

	Shape();
	Shape(const Shape &);
	Shape(Storage::ReadSlot &);
	Shape(const std::string &, Node *, const OptionSet & = OptionSet());
	virtual ~Shape();

	bool isIdentical(const Piece *) const;
	Shape *clone(Node *newParent, Storage::Repository *) const;
};

ML_PERSISTENT_HIMPL2(Shape,VIS_BASIC_PTAG);

}

#endif // !defined(__VIS_SHAPE_H_INCLUDED_1259698041368663__)