#pragma once

class BaseEntity;

namespace PoL
{

//=====================================================================================//
//                                class MultiMoveUtils                                 //
//=====================================================================================//
class MultiMoveUtils
{
public:
	/// ����������� �����������������
	static void prepareMultiMove(std::vector<BaseEntity*> &, const ipnt2_t &to, std::vector<pnt_vec_t> &);
};


}
