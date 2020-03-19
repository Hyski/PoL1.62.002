#if !defined(__VIS_APP_FRONT_H_INCLUDED_7483562881392863__)
#define __VIS_APP_FRONT_H_INCLUDED_7483562881392863__

#include "Vis.h"
#include <mll/fs/fs.h>

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class MeshPiece;

//=====================================================================================//
//                                  class PieceBuffer                                  //
//=====================================================================================//
/// Класс, который содержит определяемую приложением информацию для рендеринга
class PieceBuffer
{
public:
	ML_RTTI_ROOT(PieceBuffer);
	virtual ~PieceBuffer() {}
};

//=====================================================================================//
//                                   class AppFront                                    //
//=====================================================================================//
/// Интерфейс приложения, необходимый для работы мешей 
class AppFront
{
protected:
	virtual ~AppFront() {}

public:
	/// Открывает файл на чтение
	/// \param name Имя файла
	virtual std::auto_ptr<mll::fs::i_file> openFile(const std::string &name) = 0;

	/// Создает экземпляр PieceBuffer для куска геометрии
	/*! \param piece Кусок геометрии, для которого нужно создать PieceBuffer
	 *  \param userdata Пользовательские данные, переданные в конструктор меша
	 *  \return Указатель на PieceBuffer */
	virtual std::auto_ptr<PieceBuffer> createPieceBuffer(MeshPiece *piece, HObject userdata) = 0;
};

}

#endif // !defined(__VIS_APP_FRONT_H_INCLUDED_7483562881392863__)