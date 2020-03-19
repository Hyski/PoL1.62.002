#if !defined(__VIS_POLY_GROUP_H_INCLUDED_9847526135018653__)
#define __VIS_POLY_GROUP_H_INCLUDED_9847526135018653__

#include "VisChannel.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                    struct PolyId                                    //
//=====================================================================================//
struct PolyId
{
	Storage::Ptr<ChannelIndex16> m_channel;
	unsigned short m_index;

	PolyId() : m_channel(0), m_index(0) {}
	PolyId(ChannelIndex16 *c, unsigned short i) : m_channel(c), m_index(i) {}

	friend bool operator<(const PolyId &l, const PolyId &r)
	{
		if(l.m_channel == r.m_channel) return l.m_index<r.m_index;
		return l.m_channel < r.m_channel;
	}

	friend bool operator==(const PolyId &l, const PolyId &r)
	{
		return l.m_channel == r.m_channel && l.m_index == r.m_index;
	}

	struct Compare
	{
		bool operator()(const ChannelIndex16 *c, const PolyId &r) const
		{
			return c < r.m_channel;
		}

		bool operator()(const PolyId &l, const ChannelIndex16 *c) const
		{
			return l.m_channel < c;
		}
	};
};

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                                   class PolyGroup                                   //
//=====================================================================================//
class VIS_IMPORT PolyGroup : public Storage::PersistentBase
{
public:
	typedef std::vector<PolyId> Polys_t;
	typedef Polys_t::const_iterator iterator;
	typedef std::pair<iterator,iterator> PolyRange_t;

private:
	std::string m_name;

	bool m_isSorted;
	Polys_t m_polys;

public:
	ML_RTTI_CUSTOM_SINGLE(PolyGroup,"[Vis.PolyGroup]");
	ML_PERSISTENT2(PolyGroup,VIS_BASIC_PTAG);

	PolyGroup();
	PolyGroup(const PolyGroup &);
	PolyGroup(Storage::ReadSlot &);
	PolyGroup(const std::string &name);

	void addPoly(const PolyId &);
	bool isYourPoly(const PolyId &);

	const std::string &getName() const { return m_name; }
	const Polys_t &getPolyIds() const { return m_polys; }
	PolyRange_t getPolyIdsOfPiece(ChannelIndex16 *) const;

	void store(Storage::WriteSlot &) const;

	void migrate(Vis::ChannelBase *from, Vis::ChannelBase *to, unsigned short base);

private:
	void sort();
};

#pragma warning(pop)

ML_PERSISTENT_HIMPL2(PolyGroup,VIS_BASIC_PTAG);

}

#endif // !defined(__VIS_POLY_GROUP_H_INCLUDED_9847526135018653__)