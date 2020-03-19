#if !defined(__VIS_CHANNEL_MERGER_H_INCLUDED_1600292169706150__)
#define __VIS_CHANNEL_MERGER_H_INCLUDED_1600292169706150__

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                 class ChannelMerger                                 //
//=====================================================================================//
class VIS_IMPORT ChannelMerger
{
public:
	void operator()(ChannelBase *, const ChannelBase *) const;

private:
	template<typename T>
	void merge(ChannelBase *l, const ChannelBase *r) const
	{
		doMerge(static_cast<T*>(l), static_cast<const T *>(r));
	}

	void doMerge(ChannelFloat *, const ChannelFloat *) const;
	void doMerge(ChannelPosition3 *, const ChannelPosition3 *) const;
	void doMerge(ChannelNormal3 *, const ChannelNormal3 *) const;
	void doMerge(ChannelIndex16 *, const ChannelIndex16 *) const;
	void doMerge(ChannelBase *, const ChannelBase *) const;

	matrix3 getMatrixDifference(ChannelBase *, const ChannelBase *) const;
};

}

#endif // !defined(__VIS_CHANNEL_MERGER_H_INCLUDED_1600292169706150__)