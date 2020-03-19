#if !defined(__VIS_PARTICLES_H_INCLUDED_7552370787048739__)
#define __VIS_PARTICLES_H_INCLUDED_7552370787048739__

#include "VisPiece.h"

namespace Vis
{

class Material;

//=====================================================================================//
//                      class VIS_IMPORT Particles : public Piece                      //
//=====================================================================================//
class VIS_IMPORT Particles : public Piece
{
public:
	ML_RTTI_CUSTOM(Particles,"[Vis.Particles]",Piece);
	ML_PERSISTENT2(Particles,VIS_BASIC_PTAG);

	Particles();
	Particles(const Particles &);
	Particles(Storage::ReadSlot &);
	Particles(const std::string &, Node *, const OptionSet & = OptionSet());
	virtual ~Particles();

	bool isIdentical(const Piece *) const;
	Particles *clone(Node *newParent, Storage::Repository *) const;
};

ML_PERSISTENT_HIMPL2(Particles,VIS_BASIC_PTAG);

}

#endif // !defined(__VIS_PARTICLES_H_INCLUDED_7552370787048739__)