/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������                                                                 
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined (__PARTICLE_H__)
#define __PARTICLE_H__
             
                                        
//
// ������� ����� ������� BaseParticle
//
class BaseParticle
{
public:
	// ����������
	point3 coords;
	// ����
	unsigned int color;
	// ������ ��������
	float size;

	// ����������� �� ���������
	BaseParticle(): coords(0, 0, 0), color(0), size(0)
	{}
	virtual ~BaseParticle() {}
};

#endif // __PARTICLE_H__