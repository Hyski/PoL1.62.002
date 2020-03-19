// KeyAnimation.h: interface for the KeyAnimation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_)
#define AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class SavSlot;
class OptSlot;

//=====================================================================================//
//                                 class KeyAnimation                                  //
//=====================================================================================//
class KeyAnimation  
{
protected:
	typedef std::pair<float,point3> TransKey;
	typedef std::pair<float,Quaternion> RotKey;
	typedef std::vector<TransKey> TransKeys;
	typedef std::vector<RotKey> RotKeys;
	//typedef std::map<float, point3> TransKeys;       //����� ��� �������� �������
	//typedef std::map<float, Quaternion> RotKeys;  //����� ��� �������� �������
protected:
	TransKeys Trans;
	RotKeys   Angles;
public: //    Construction/Destruction
	KeyAnimation(const KeyAnimation &a){*this=a;}
	KeyAnimation();
	virtual ~KeyAnimation();
public:
	//��������� �������� �������� � ��������� ������ �������
	void GetTrans(point3 *Trans,float Time) const;
	//��������� �������� �������� � ��������� ������ �������
	void GetAngle(Quaternion *Quat,float Time) const;
	//��������� ����� ��������
	void SetKey(float Time, const point3 &Trans);
	//��������� ����� ��������
	void SetKey(float Time, const Quaternion &Quat);
	//��������� ����� ���������
	void SetKey(float Time, const Quaternion &Quat, const point3 &Trans);
	//����� ���������� �����
	float GetLastTime() const
	{
		float a=0,b=0;
		if(Angles.size())	a=Angles.rbegin()->first;
		if(Trans.size())	b=Trans.rbegin()->first;
		return a>b?a:b;
	}
	//���������� ������ ������������ ������ � ��������
	//��������: ����� ����������� � ���������, ���� �����, �������� ��� ����
	void GetKeyTimes(std::set<float> *times); 
	//�������� ����� ������, ���� �� ��������� �������� �����������
	void 	Decimate(float _err);
	virtual void Load(FILE *f);
	virtual void Save(FILE *f) const;
	virtual void SaveBin(FILE *f) const;
	virtual void LoadBin(VFile *f);
	virtual void Load(std::istream &stream);
	virtual void Save(std::ostream &stream) const;
	bool MakeSaveLoad(SavSlot &slot);
	bool Save(OptSlot &slot);
	bool Load(OptSlot &slot);
	KeyAnimation &operator=(const KeyAnimation &a){Trans=a.Trans;Angles=a.Angles;return *this;}
};

#endif // !defined(AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_)
