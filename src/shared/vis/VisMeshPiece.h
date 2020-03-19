#if !defined(__VIS_MESH_PIECE_H_INCLUDED_9474860070375691__)
#define __VIS_MESH_PIECE_H_INCLUDED_9474860070375691__

#include "Vis.h"
#include "VisPiece.h"
#include "VisAppFront.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class Mesh;
class ChannelBase;

//=====================================================================================//
//                           struct IntermediateTraceResult                            //
//=====================================================================================//
/// ������������� ���������� ����������� ����
struct IntermediateTraceResult
{
	float m_pos;
	PolyId m_polyId;
};

//=====================================================================================//
//                                   enum TraceType                                    //
//=====================================================================================//
/// ��� �����������
enum TraceType
{
	ttCursor,		///< ������
	ttVisibility,	///< ���������
	ttPhysics		///< ������
};

//=====================================================================================//
//                                   class MeshPiece                                   //
//=====================================================================================//
/// ����� ����
class VIS_MESH_IMPORT MeshPiece
{
	typedef std::vector<ChannelBase *> Channels_t;

	Mesh *m_mesh;
	Piece *m_piece;
	std::auto_ptr<PieceBuffer> m_buffer;
	Channels_t m_channels;

	class Impl;
	std::auto_ptr<Impl> m_pimpl;

	obb3 m_obb;

	MeshPiece(const MeshPiece &);
	MeshPiece &operator=(const MeshPiece &);

public:
	/// �������� �� �������
	typedef Channels_t::const_iterator iterator;

public:
	/// �����������. ������������ �����, �� ������������� ������������ � ���������� ��������
	MeshPiece(Mesh *, Piece *, AppFront *, std::list<ChannelBase *> &, HObject userdata);
	/// ����������
	~MeshPiece();

	/// ���������� ���, � ������� ����������
	Mesh *getMesh() const { return m_mesh; }
	/// ���������� �����, ������� ������
	Piece *getPiece() const { return m_piece; }
	/// ���������� PieceBuffer, ��������� ��� ����� �����
	PieceBuffer *getBuffer() const { return m_buffer.get(); }

	/// ���������� �������� �� ������ �����
	iterator begin() const { return m_channels.begin(); }
	/// ���������� �������� �� ������� �� ��������� �������
	iterator end() const { return m_channels.end(); }

	/// ���������� ����� �� ��������������
	template<typename T> typename T::Channel_t *getChannel(const T &) const;
	/// ��������� ����� �� ��������������
	template<typename T> typename T::Channel_t *addChannel(const T &);

	/// ���������� �������� �� ����� � ��������� ������
	iterator getChannelByName(const std::string &) const;

	/// ������������ ���
	IntermediateTraceResult traceRay(const ray3 &);
	/// ������������ ��� � ������ ���������� ������������ �����
	IntermediateTraceResult traceRay(const ray3 &, TraceType);

	/// ���������� �������������� ������������� ��������������
	const obb3 &getObb() const { return m_obb; }

private:
	void calcObb();
	void makeOctTree();
};

//=====================================================================================//
//                typename T::Channel_t *MeshPiece::getChannel() const                 //
//=====================================================================================//
template<typename T>
typename T::Channel_t *MeshPiece::getChannel(const T &channelId) const
{
	iterator i = getChannelByName(T::getChannelName());
	return i != end() ? static_cast<typename T::Channel_t *>(*i) : 0;
}

//=====================================================================================//
//                   typename T::Channel_t *MeshPiece::addChannel()                    //
//=====================================================================================//
template<typename T>
typename T::Channel_t *MeshPiece::addChannel(const T &channelId)
{
	assert( getChannelByName(T::getChannelName()) == end() );
	Storage::Repository *rep = getMesh()->getStorage();
	typename T::Channel_t *chan = rep->construct<typename T::Channel_t>(T::getChannelName(),getPiece());
	m_channels.push_back(chan);
	return chan;
}

}

#endif // !defined(__VIS_MESH_PIECE_H_INCLUDED_9474860070375691__)