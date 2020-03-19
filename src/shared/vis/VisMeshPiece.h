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
/// Промежуточные результаты трассировки луча
struct IntermediateTraceResult
{
	float m_pos;
	PolyId m_polyId;
};

//=====================================================================================//
//                                   enum TraceType                                    //
//=====================================================================================//
/// Тип трассировки
enum TraceType
{
	ttCursor,		///< Курсор
	ttVisibility,	///< Видимость
	ttPhysics		///< Физика
};

//=====================================================================================//
//                                   class MeshPiece                                   //
//=====================================================================================//
/// Кусок меша
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
	/// Итератор по каналам
	typedef Channels_t::const_iterator iterator;

public:
	/// Конструктор. Используется мешем, не рекомендуется использовать в приложении напрямую
	MeshPiece(Mesh *, Piece *, AppFront *, std::list<ChannelBase *> &, HObject userdata);
	/// Деструктор
	~MeshPiece();

	/// Возвращает меш, в котором содержится
	Mesh *getMesh() const { return m_mesh; }
	/// Возвращает кусок, который хранит
	Piece *getPiece() const { return m_piece; }
	/// Возвращает PieceBuffer, созданный для этого куска
	PieceBuffer *getBuffer() const { return m_buffer.get(); }

	/// Возвращает итератор на первый канал
	iterator begin() const { return m_channels.begin(); }
	/// Возвращает итератор на позицию за последним каналом
	iterator end() const { return m_channels.end(); }

	/// Возвращает канал по идентификатору
	template<typename T> typename T::Channel_t *getChannel(const T &) const;
	/// Добавляет канал по идентификатору
	template<typename T> typename T::Channel_t *addChannel(const T &);

	/// Возвращает итератор на канал с указанным именем
	iterator getChannelByName(const std::string &) const;

	/// Трассировать луч
	IntermediateTraceResult traceRay(const ray3 &);
	/// Трассировать луч с учетом физических особенностей куска
	IntermediateTraceResult traceRay(const ray3 &, TraceType);

	/// Возвращает ограничивающий прямоугольный параллелепипед
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