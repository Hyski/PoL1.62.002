#if !defined(AFX_SCATTEREDITEMS_H__28DB6F74_F97D_4AE3_8CC9_2929865F5C61__INCLUDED_)
#define AFX_SCATTEREDITEMS_H__28DB6F74_F97D_4AE3_8CC9_2929865F5C61__INCLUDED_
/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: ����� ��� ��������� ������������ �������������
������������ �� ������ ���������
Author: Grom 
Creation: 20 ������� 2000
***************************************************************/                

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SkSkin;
class SkAnim;
class SkState;
struct AnimaData;

//=====================================================================================//
//                                 class ScatteredItem                                 //
//=====================================================================================//
class ScatteredItem
{
private: 
	typedef std::map<std::string,SkSkin *> SkinCont;
	SkSkin *Item;
	point3 Pos;      //������������
	float Phase;     //������� ���� ��������/�����������
	float Vel;       //�������� ��������/�����������                        
	float Rad; //������ �������� ����� (������������ ��������� �����
	static SkinCont  Skins;
	static AnimaData Skel;
	static SkAnim* Anim;

public:
	std::string ItName;
	void Draw(float Time);
	static void Clear();
	point3 GetPos(){return Pos;};
	ScatteredItem(const point3 &pos,const std::string Name);
	virtual ~ScatteredItem();

private:
	BBox GetLocalBB();
};

#endif // !defined(AFX_SCATTEREDITEMS_H__28DB6F74_F97D_4AE3_8CC9_2929865F5C61__INCLUDED_)
