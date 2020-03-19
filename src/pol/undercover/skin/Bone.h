// Bone.h: interface for the Bone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <common/3d/geometry.h>
#include <common/3d/quat.h>
#include "keyanimation.h"

typedef float ScelTime;

class Skeleton;
class Bone  
  {
  public:   

    int UseHierarchy;
    std::string Name;
    KeyAnimation Keys;
    //std::map<ScelTime,point3> Trans;       //����� ��� �������� �������
    //std::map<ScelTime,Quaternion> Angles;  //����� ��� �������� �������
    Bone *Parent;
    int ChildNum;
    Quaternion CurAngles,WorldAngles;
    point3 Translate;

  public:
    Transformation Transform;
    friend class Skeleton;
    Bone();
    virtual ~Bone();
     
    const std::string &GetName() const {return Name;}
    //��������������� ������� ��� ���������� �������
    void UpdateOnTime(ScelTime Time, float delay=0, Bone *Ancestor=NULL);
    //���������� ������������� ������������� ����
    void CalcTransform(Transformation *World=NULL);
    //��������������� ���� ������ �� �������� � 3D Max 3.0
#ifdef COMPILEFORMAX
    void ParseKeys(INode *node, SkeletonExporter *sc, bool First);
#endif
    //�������� �� ����
    int Save(FILE *f);
    //��������� � �����
    int Load(FILE *f);
    //�������� �� ����
    int SaveBin(FILE *f);
    //��������� � �����
    int LoadBin(VFile *f);
    //������������ �����, ���������� �������������
    void OptimizeKeys();
    ScelTime GetLastTime() const 
      {
      return Keys.GetLastTime();
      };
    Bone & operator =(const Bone &a)
      {
      UseHierarchy=a.UseHierarchy;
      Name=a.Name;
      Keys=a.Keys;
      //Trans=a.Trans;       //����� ��� �������� �������
      //Angles=a.Angles;  //����� ��� �������� �������
      CurAngles=a.CurAngles;
      WorldAngles=a.WorldAngles;
      Translate=a.Translate;
      Parent=NULL;
      Transform=a.Transform;
      
      return *this;
      }
  };
#endif // !defined(AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_)
