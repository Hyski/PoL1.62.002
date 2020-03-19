#ifndef _SKANIM
#define _SKANIM
/*********************************************************************\
* набор классов для скелетной анимации.                              * 
* author: Grom																											 *
\*********************************************************************/

#include <common/graphpipe/SimpleTexturedObject.h>
#include "keyanimation.h"

class OptSlot;


class Skeleton;	 //fixme: Эти классы в релизе вообще не должны существовать!
class Skin;   	 //fixme: Эти классы в релизе вообще не должны существовать!


class SkAnim;

//мгновенное состояние скелета
class SkState
{
	friend class SkAnim;
	friend class SkSkin;

public:
	struct BoneState
	{
		point3 Trans;
		Quaternion Orient;
		D3DMATRIX World;
	};

public:
	SkState() {}
	const D3DMATRIX *GetBone(int ID) const { return &m_SkelState_mod[ID].World; }
	point3 GetNearestBone(const point3& where) const;
	unsigned int GetBoneCount() const { return m_SkelState_mod.size(); }
	const BoneState &GetBoneEx(unsigned int i) const { return m_SkelState_mod[i]; }
	BoneState &GetBoneEx(unsigned int i) { return m_SkelState_mod[i]; }

private:
	
	std::vector<BoneState> m_SkelState_mod;

	void Alloc(int bone_num);
};

//сама скелетная анимация
struct AnimaData
{
	float StartTime;   //Время старта анимации
	float Int_Time;    //время на интерполяцию со старой анимацией
	SkState LastState; //скелет для интерполяции
	float EndTime;
};

//=====================================================================================//
//                                    class SkAnim                                     //
//=====================================================================================//
class SkAnim
{
	std::string m_name;

public:
	static float SkSpeed;

	const SkState& Get(const AnimaData *dta, float Time); //возвращает мгновенное состояние скелета в указанный момент
	AnimaData Start(float CurTime, float IntTime=0, const SkState* LastState=NULL); //возвращает время завершения анимации
	point3 GetOffset(const AnimaData *dta,float CurTime) const;//смещение скелета относительно нуля координат в указанный момент времени
	int GetBoneID(const std::string &BoneName)const;
	SkAnim(const std::string &name) : m_name(name) {}
	SkAnim(Skeleton *OldSkel, const std::string &name);
	~SkAnim(){Alloc(0);};

	const std::string &GetAnimName() const { return m_name; }

public: //утилитные функции
	SkAnim& operator= (const SkAnim &a);
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);	

private:
	struct Bone
	{
		std::string Name;
		KeyAnimation Anima;
	};

	std::vector<Bone> m_Skeleton;

	unsigned int GetBoneCount() const { return m_Skeleton.size(); }
	Bone &GetBone(unsigned int i) { return m_Skeleton[i]; }
	const Bone &GetBone(unsigned int i) const { return m_Skeleton[i]; }

	SkState m_InstantState; //здесь будет возвращаться состояние
	KeyAnimation m_Offset;

	std::map<std::string,int> m_Bones2ID; //карта отражения имен костей на индексы

private://переменные, нужные в процессе анимации
	float m_Duration;    //длительность анимации

private: //утилитные функции
	void Alloc(int bone_num) 
	{
		m_Skeleton.resize(bone_num);
	};

};

//=====================================================================================//
//                                 class SkBoneToPnts                                  //
//=====================================================================================//
class SkBoneToPnts
{
	friend class SkSkin;
	friend class SkPntToBones;

	typedef point3* PntPtr;

public://структуры данных
#pragma pack(push,1)
	struct Pnt
	{
		PntPtr Point;
		point3 Offset;
		float  Weight;
		int PartNum;
		int PntNum;

		Pnt() : Point(0), Offset(0.0f,0.0f,0.0f), Weight(0.0f), PartNum(0), PntNum(0) {}
	};
#pragma pack(pop)

public://утилитные функции
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);

	SkBoneToPnts& operator=(const SkBoneToPnts &a)
	{
		SkBoneToPnts tmp(a);
		Swap(tmp);
		return *this;
	}

public:
	std::string m_BoneName;
	std::vector<Pnt> m_Points;
	int m_BoneID;

public:
	SkBoneToPnts() : m_BoneID(0) {}
	~SkBoneToPnts() {}

private:
	void Swap(SkBoneToPnts &p)
	{
		assert( this != &p );
		m_Points.swap(p.m_Points);
		m_BoneName.swap(p.m_BoneName);
		std::swap(m_BoneID,p.m_BoneID);
	}
};

//=====================================================================================//
//                                 class SkPntToBones                                  //
//=====================================================================================//
class SkPntToBones
{
	friend class SkSkin;

	static const int MaxBones = 5;
	static const int Alignment = 16;

	struct SkPoint
	{
		point3 *m_ptr;

		SkPoint() : m_ptr(0)
		{
		}
	};

	struct SkInfluence
	{
		point3 m_offset;
		float m_weight;
		int m_boneId;

		SkInfluence() : m_offset(0.0f,0.0f,0.0f), m_weight(0.0f), m_boneId(-1) {}
	};

	struct SkPointStatic
	{
		std::string m_boneName[MaxBones];

		int m_partNum;
		int m_pntNum;

		SkPointStatic() : m_partNum(0), m_pntNum(0) {}
	};

	typedef std::vector<size_t> Offsets_t;
	typedef std::vector<SkPoint> Points_t;
	typedef std::vector<SkInfluence> Influences_t;
	typedef std::vector<SkPointStatic> StaticPoints_t;

	//Points_t m_points;
	//Offsets_t m_offsets;
	StaticPoints_t m_staticPoints;

	typedef std::vector<char> Data_t;
	Data_t m_data;

	char *m_alignedData;

	int m_pointsj[MaxBones];	///< Количество точек, привязанных к определенному количеству костей
	int m_pointsoff[MaxBones];	///< Смещение в массиве точек

public:
	SkPntToBones() {}
	SkPntToBones(const SkPntToBones &);

	~SkPntToBones() {}

	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);
	bool Load2(OptSlot &slot);

	SkPntToBones& operator=(const SkPntToBones &a)
	{
		SkPntToBones tmp(a);
		Swap(tmp);
		return *this;
	}

	void Swap(SkPntToBones &p)
	{
		assert( this != &p );
		std::swap_ranges(m_pointsj,m_pointsj+MaxBones,p.m_pointsj);
		std::swap_ranges(m_pointsoff,m_pointsoff+MaxBones,p.m_pointsoff);
		m_staticPoints.swap(p.m_staticPoints);

		m_data.swap(p.m_data);
		//m_offsets.swap(p.m_offsets);
		std::swap(m_alignedData,p.m_alignedData);
	}

private:
	void Convert(SkBoneToPnts *, int);
	void AlignData();

	template<int N, typename R, typename U>
	void AcceptPoints(const SkBoneToPnts *, int l, const R &cnts, U &influs);
};

//=====================================================================================//
//                                    class SkSkin                                     //
//=====================================================================================//
//Кожа, которая ложится на скелет
class SkSkin
{
	static const size_t LinkCount = 256;

	int m_lastFrameUpdate;
	BBox m_bbox;

public:
	void Update(const SkState *Skel); //обновить состояние кожи
	void ValidateLinks(const SkAnim *Anim); //ставит в соответствие кости скелета и кожи

	SkSkin() : m_BonesNum(0) {}
	SkSkin(Skin *oldskin);
	SkSkin(const SkSkin &skin) : m_Mesh(skin.m_Mesh), m_BonesNum(skin.m_BonesNum), m_links(skin.m_links)
	{
		//std::copy_n(skin.m_Links, LinkCount, m_Links);
		MakePointPtrs();
	}
	~SkSkin();

public: //утилитные функции
	const BBox &GetBBox() { return m_bbox; }
	const TexObject* GetMesh() {return &m_Mesh;};
	SkSkin& operator= (const SkSkin& a);

	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);
	bool Load2(OptSlot &slot);

private:
	TexObject m_Mesh;
	SkPntToBones m_links;
	//SkBoneToPnts m_Links[LinkCount]; //внимание: ограничение количества костей
	int m_BonesNum;

	void MakePointPtrs();
};

//=====================================================================================//
//                         inline SkAnim& SkAnim::operator= ()                         //
//=====================================================================================//
inline SkAnim& SkAnim::operator= (const SkAnim &a)
{
	if(this != &a)
	{
		m_name = a.m_name;
		m_InstantState = a.m_InstantState;
		m_Skeleton = a.m_Skeleton;
		m_Duration = a.m_Duration;  //длительность анимации
		m_Offset = a.m_Offset;
	}
	return *this;
}

#endif