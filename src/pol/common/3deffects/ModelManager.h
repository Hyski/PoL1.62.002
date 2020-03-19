/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� �������� � ����������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#if !defined(__MODELMANAGER_H__)
#define __MODELMANAGER_H__

#include "Effect.h"
class GraphPipe;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// ����� ��������� �������� � ����������
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelManager
{
	typedef std::list<BaseEffect*> EffectList_t;
	EffectList_t m_effects;
public:
	ModelManager();
	~ModelManager();
	// ��������� ���
	void NextTick(float time);
	// ���������
	void Draw();
	// �������� ������ � ������
	void AddEffect(BaseEffect* effect);
	// �������� ��� �������
	void clear();
};


#endif // !defined(__MODELMANAGER_H__)
