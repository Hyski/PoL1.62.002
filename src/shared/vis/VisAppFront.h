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
/// �����, ������� �������� ������������ ����������� ���������� ��� ����������
class PieceBuffer
{
public:
	ML_RTTI_ROOT(PieceBuffer);
	virtual ~PieceBuffer() {}
};

//=====================================================================================//
//                                   class AppFront                                    //
//=====================================================================================//
/// ��������� ����������, ����������� ��� ������ ����� 
class AppFront
{
protected:
	virtual ~AppFront() {}

public:
	/// ��������� ���� �� ������
	/// \param name ��� �����
	virtual std::auto_ptr<mll::fs::i_file> openFile(const std::string &name) = 0;

	/// ������� ��������� PieceBuffer ��� ����� ���������
	/*! \param piece ����� ���������, ��� �������� ����� ������� PieceBuffer
	 *  \param userdata ���������������� ������, ���������� � ����������� ����
	 *  \return ��������� �� PieceBuffer */
	virtual std::auto_ptr<PieceBuffer> createPieceBuffer(MeshPiece *piece, HObject userdata) = 0;
};

}

#endif // !defined(__VIS_APP_FRONT_H_INCLUDED_7483562881392863__)