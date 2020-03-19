#pragma once

namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                  class AnimTrack : public Storage::PersistentBase                   //
//=====================================================================================//
class VIS_IMPORT AnimTrack : public Storage::PersistentBase
{
	std::string m_nodeName;

public:
	typedef animation< mll::utils::l_interpolator<point3> > OriginAnim_t;
	typedef animation< mll::utils::l_interpolator<quaternion> > OrientAnim_t;

private:
	OriginAnim_t m_originAnim;
	OrientAnim_t m_orientAnim;

public:
	ML_RTTI_CUSTOM_SINGLE(AnimTrack,"[Vis.AnimTrack]");
	ML_PERSISTENT2(AnimTrack,VIS_BASIC_PTAG);

	AnimTrack();
	AnimTrack(const AnimTrack &);
	AnimTrack(Storage::ReadSlot &);

	/// ¬озвращает название узла, который анимируетс€ этим треком
	const std::string &getNodeName() const { return m_nodeName; }
	/// ”станавливает название анимируемого узла
	void setNodeName(const std::string &name) { m_nodeName = name; }

	/// ¬озвращает анимацию позиции
	const OriginAnim_t &getOriginAnim() const { return m_originAnim; }
	/// ¬озвращает анимацию ориентации
	const OrientAnim_t &getOrientAnim() const { return m_orientAnim; }

	/// ”становить анимацию позиции
	void setOriginAnim(const OriginAnim_t &oa) { m_originAnim = oa; }
	/// ”становить анимацию ориентации
	void setOrientAnim(const OrientAnim_t &oa) { m_orientAnim = oa; }

	void store(Storage::WriteSlot &) const;
};

#pragma warning(pop)

ML_PERSISTENT_HIMPL2(AnimTrack,VIS_BASIC_PTAG);

}