#if !defined(__VIS_CHANNEL_H_INCLUDED_0243736855124350__)
#define __VIS_CHANNEL_H_INCLUDED_0243736855124350__

#include "VisChannelBase.h"
#include "VisChannelHelper.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable:4251)
//=====================================================================================//
//                                    class Channel                                    //
//=====================================================================================//
template<typename T, typename Tag>
class VIS_IMPORT Channel : public ChannelBase
{
public:
	typedef std::vector<T> Data_t;
	typedef T Value_t;

private:
	Data_t m_data;

public:
#ifdef ML_STATIC
	ML_RTTI_CUSTOM(Channel,getChannelName(),ChannelBase);
#else
	ML_RTTI_CUSTOM_IFACE(Channel,getChannelName(),ChannelBase);
#endif
	ML_PERSISTENT2(Channel,VIS_BASIC_PTAG);

	Channel() {}
	Channel(Storage::ReadSlot &slot) : ChannelBase(slot)
	{
		m_data.swap(Data_t());
		unsigned int count;
		slot.getStream() >> count;
		m_data.resize(count);
		for(unsigned int i = 0; i < count; ++i)
		{
			restoreElement(m_data[i],slot);
		}
	}
	Channel(const Channel &c) : ChannelBase(c), m_data(c.m_data) {}
	Channel(const std::string &name, Piece *piece) : ChannelBase(name,piece) {}
	~Channel() {}

	const Data_t &getData() const { return m_data; }
	void setData(const Data_t &data) { m_data = data; }
	void swapData(Data_t &data) { m_data.swap(data); }

	void store(Storage::WriteSlot &slot) const
	{
		ChannelBase::store(slot);
		slot.getStream() << m_data.size();
		for(Data_t::const_iterator i = m_data.begin(); i != m_data.end(); ++i)
		{
			storeElement(*i,slot);
		}
	}

	virtual void appendDataFrom(const ChannelBase *c)
	{
		assert( c->rtti_dynamic() == rtti_dynamic() );
		const Channel *cc = static_cast<const Channel *>(c);
		m_data.insert(m_data.end(),cc->getData().begin(),cc->getData().end());
	}

	virtual ChannelBase *clone(Piece *piece, Storage::Repository *rep) const
	{
		return rep->construct<Channel>(getName(),piece);
	}

	virtual unsigned int getSize() const
	{
		return getData().size();
	}

private:
	static std::string getChannelName()
	{
		return "[Vis.Channel<" + detail::GetTypeName< detail::ChannelTraits<T,Tag> >::get() + ">]";
	}

	void storeElement(const T &e, Storage::WriteSlot &slot) const
	{
		slot.getStream() << e;
	}

	void restoreElement(T &e, Storage::ReadSlot &slot) const
	{
		slot.getStream() >> e;
	}
};

#pragma warning(pop)

#define VIS_GEN_CHANNEL_TYPE(A)		\
	typedef Channel< VIS_CHANNEL_TRAITS##A :: Value_t, VIS_CHANNEL_TRAITS##A :: Tag_t > Channel##A;
	VIS_FOREACH_CHANNEL_TYPE(VIS_GEN_CHANNEL_TYPE)
#undef VIS_GEN_CHANNEL_TYPE

#define VIS_MAKE_CHANNEL_ID(N,C,T)									\
	class ChannelId##N##_t											\
	{																\
	public:															\
		typedef Channel##C Channel_t;								\
		static std::string getChannelName() { return T; }			\
	};																\
	static ChannelId##N##_t channelId##N

	VIS_MAKE_CHANNEL_ID(Pos,Position3,"xyz");
	VIS_MAKE_CHANNEL_ID(Normal,Normal3,"normal");
	VIS_MAKE_CHANNEL_ID(Tangent,Normal3,"tangent");
	VIS_MAKE_CHANNEL_ID(TexCoord,TexCoord2,"uv");
	VIS_MAKE_CHANNEL_ID(Diffuse,UInt,"diffuse");
	VIS_MAKE_CHANNEL_ID(Index,Index16,"indices");

#undef VIS_MAKE_CHANNEL_ID

#define VIS_GEN_PERS(A)		\
	ML_PERSISTENT_TEMPLATE_HIMPL2( Channel##A, VIS_BASIC_PTAG );
	VIS_FOREACH_CHANNEL_TYPE(VIS_GEN_PERS)
#undef VIS_GEN_PERS

}

#endif // !defined(__VIS_CHANNEL_H_INCLUDED_0243736855124350__)