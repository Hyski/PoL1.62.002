#if !defined(__VIS_CHANNEL_BASE_H_INCLUDED_3259699232931665__)
#define __VIS_CHANNEL_BASE_H_INCLUDED_3259699232931665__

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class Piece;

struct GrowChannelTag {};
struct TransformChannelTag {};
struct NonMutatingChannelTag {};

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                                  class ChannelBase                                  //
//=====================================================================================//
class VIS_IMPORT ChannelBase : public Storage::PersistentBase
{
	std::string m_name;
	Storage::Ptr<Piece> m_piece;

public:
	ML_RTTI_CUSTOM_ROOT(ChannelBase,"[Vis.ChannelBase]");

	ChannelBase() : m_name("**wrong_channel_name**"), m_piece(0) {}
	ChannelBase(Storage::ReadSlot &slot) : Storage::PersistentBase(slot)
	{
		slot.getStream() >> m_name;
		m_piece = slot.restoreWeakReference<Piece>();
	}
	ChannelBase(const ChannelBase &c) : m_name(c.getName()), m_piece(c.getPiece()) {}
	ChannelBase(const std::string &name, Piece *piece) : m_name(name), m_piece(piece) {}
	virtual ~ChannelBase() {}

	Piece *getPiece() const { return m_piece.get(); }
	const std::string &getName() const { return m_name; }

	virtual ChannelBase *clone(Piece *, Storage::Repository *) const = 0;
	virtual void appendDataFrom(const ChannelBase *c) = 0;
	virtual unsigned int getSize() const = 0;

	bool isIdentical(const ChannelBase *c) const;

protected:
	void store(Storage::WriteSlot &slot) const
	{
		slot.getStream() << getName();
		slot.storeReference(m_piece);
	}

};

//=====================================================================================//
//                    inline bool ChannelBase::isIdentical() const                     //
//=====================================================================================//
inline bool ChannelBase::isIdentical(const ChannelBase *c) const
{
	return rtti_dynamic() == c->rtti_dynamic()
		&& getName() == c->getName();
}

//=====================================================================================//
//                             struct ChannelsByPiecePred                              //
//=====================================================================================//
struct ChannelsByPiecePred
{
	bool operator()(const ChannelBase *l, const ChannelBase *r) const
	{
		return l->getPiece() < r->getPiece();
	}
	bool operator()(const Piece *l, const ChannelBase *r) const
	{
		return l < r->getPiece();
	}
	bool operator()(const ChannelBase *l, const Piece *r) const
	{
		return l->getPiece() < r;
	}
};

#pragma warning(pop)

}

#endif // !defined(__VIS_CHANNEL_BASE_H_INCLUDED_3259699232931665__)