/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: �����, ���������� �� ��������� � ���������                                                                  
�������� ����� � ����

Creation Date: 16.06.00

Author: Grom
***************************************************************/                
#pragma once

#include <common/graphpipe/simpletexturedobject.h>

class GraphPipe;
class Camera;

//=====================================================================================//
//                          class LongShot : public TexObject                          //
//=====================================================================================//
class LongShot : public TexObject
{
protected:
	point3 CamOffset;//�����, �� ������� �������� ������
	bool Enabled;

public:
	void Enable(bool Flag);
	void Draw(GraphPipe *Pipe);
	void Update(const Camera *Cam);

	LongShot();
	virtual ~LongShot();
};
