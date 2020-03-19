#include "precomp.h"
#include "skanim.h"
#include "skeleton.h"
#include "skin.h"
#include <numeric>
#include <common/graphpipe/simpletexturedobject.h>
#include <common/utils/d3dutil.h>
#include <common/utils/optslot.h>
#include <common/utils/profiler.h>

#include <xmmintrin.h>

/***********************************************************\
|*          реализация класса SkState	      			   *|
\***********************************************************/
static const int SKVERSION=0x8001;
extern int ShellFrameCounter;

//=====================================================================================//
//                                void SkState::Alloc()                                //
//=====================================================================================//
void SkState::Alloc(int bone_num) 
{
	m_SkelState_mod.resize(bone_num);
	
	for(unsigned int i=0; i<GetBoneCount(); i++)
	{
		D3DUtil_SetIdentityMatrix(GetBoneEx(i).World);
	}
};

/***********************************************************\
|*          реализация класса SkAnim	      			   *|
\***********************************************************/
float SkAnim::SkSpeed = 2.0f;

//=====================================================================================//
//                          point3 SkAnim::GetOffset() const                           //
//=====================================================================================//
//смещение скелета относительно нуля координат в указанный момент времени
point3 SkAnim::GetOffset(const AnimaData *dta,float CurTime) const
{
	point3 t; 
	m_Offset.GetTrans(&t,SkSpeed*(CurTime-dta->StartTime)); 
	return t;
}

//=====================================================================================//
//                            const SkState& SkAnim::Get()                             //
//=====================================================================================//
//возвращает мгновенное состояние скелета в указанный момент
const SkState& SkAnim::Get(const AnimaData *dta, float Time)
{
	const float worldtime = std::max(Time - dta->StartTime,0.0f);
	const float skeltime = SkSpeed*worldtime;
	for(unsigned int i=0; i<GetBoneCount(); i++)
	{
		SkState::BoneState &bone_st = m_InstantState.GetBoneEx(i);
		GetBone(i).Anima.GetAngle(&bone_st.Orient,skeltime);
		GetBone(i).Anima.GetTrans(&bone_st.Trans,skeltime);

		if(worldtime < dta->Int_Time)
		{
			if(dta->LastState.GetBoneCount()<m_InstantState.GetBoneCount())
			{
				throw CASUS ("error Bones number!!!!");
			}
			const SkState::BoneState &last_bone=dta->LastState.GetBoneEx(i);
			const float alpha = worldtime/dta->Int_Time;
			bone_st.Orient=Quaternion::Slerp(alpha, last_bone.Orient, bone_st.Orient);
			Interpolate(&bone_st.Trans, bone_st.Trans, last_bone.Trans, alpha);
		}
		bone_st.Orient.ToRotationMatrix(bone_st.World.m);
		*(point3 *)&bone_st.World._41=bone_st.Trans;
	}
	return m_InstantState;
}

//=====================================================================================//
//                       point3 SkState::GetNearestBone() const                        //
//=====================================================================================//
point3 SkState::GetNearestBone(const point3& where) const
{
	int best =-1;
	float bestfunc=0;
	for(int i=0; i<GetBoneCount(); i++)
	{
		point3 dir = GetBoneEx(i).Trans-where;
		float dist = dir.Length();
		dir = Normalize(dir);
		point3 flatdir = dir;flatdir.z=0;
		flatdir = Normalize(flatdir);
		float angle = flatdir.Dot(dir);
		float func = -dist-angle;
		if(best==-1 || func>bestfunc)
		{
			best = i;
			bestfunc=func;
		}

	}
	if(best==-1) return where;
	else		 return GetBoneEx(best).Trans;
}

//=====================================================================================//
//                                  SkAnim::SkAnim()                                   //
//=====================================================================================//
SkAnim::SkAnim(Skeleton *OldSkel, const std::string &name)
:	m_name(name)
{
	int i;
	Alloc(OldSkel->BonesNum);
	m_InstantState.Alloc(GetBoneCount());

	point3 BaseOffs;
	std::set<float> times;
	std::set<float>::iterator times_it;

	for(i=0; i<GetBoneCount(); i++)
		OldSkel->Body[i].Keys.GetKeyTimes(&times);
	float error=1000,ot=0;
	for(times_it=times.begin();times_it!=times.end();		ot = *times_it,times_it++)
	{   
		error +=*times_it-ot;
		if(error<0.1) continue;
		error=0;
		OldSkel->UpdateOnTime(*times_it);
		OldSkel->CalcTransform();
		point3 tr=OldSkel->GetTranslation();
		tr.z=0;
		if(times_it==times.begin()) BaseOffs=tr;
		m_Offset.SetKey(*times_it,Quaternion::IDENTITY,tr-BaseOffs);
		for(i=0; i<GetBoneCount(); i++)
		{
			GetBone(i).Anima.SetKey(*times_it,OldSkel->Body[i].WorldAngles,OldSkel->Body[i].Transform.trans-tr);
		}
	}
	for(i=0; i<GetBoneCount(); i++)
	{
		GetBone(i).Anima.Decimate(0.005);
		GetBone(i).Name=OldSkel->Body[i].GetName();
		m_Bones2ID[GetBone(i).Name]=i;
	}
	m_Duration=*times.rbegin();
}

//=====================================================================================//
//                              AnimaData SkAnim::Start()                              //
//=====================================================================================//
// возвращает время завершения анимации
AnimaData SkAnim::Start(float CurTime, float IntTime, const SkState* LastState)
{
	AnimaData dta;
	dta.StartTime = CurTime;
	dta.EndTime = CurTime + m_Duration/SkSpeed;
	dta.Int_Time = std::min(0.9f*(dta.EndTime-dta.StartTime),IntTime);
	if(LastState) dta.LastState=*LastState;
	else dta.LastState.Alloc(GetBoneCount());
	return dta;
}

//=====================================================================================//
//                            int SkAnim::GetBoneID() const                            //
//=====================================================================================//
int SkAnim::GetBoneID(const std::string &BoneName) const
{
	std::map<std::string, int>::const_iterator it;
	it=m_Bones2ID.find(BoneName);
	if(it==m_Bones2ID.end()) return -1;
	return it->second;
}

//=====================================================================================//
//                                 bool SkAnim::Save()                                 //
//=====================================================================================//
bool SkAnim::Save(OptSlot &slot)
{
	slot<<(int)SKVERSION;
	slot<<GetBoneCount();
	m_Offset.Save(slot);

	for(int i=0; i<GetBoneCount(); i++)
	{
		slot<<GetBone(i).Name;
		GetBone(i).Anima.Save(slot);
	}
	return true;
}

//=====================================================================================//
//                                 bool SkAnim::Load()                                 //
//=====================================================================================//
bool SkAnim::Load(OptSlot &slot)
{
	int ver;
	slot>>ver;
	if(ver!=SKVERSION) return false;
	unsigned int bone_count;
	slot>>bone_count;
	Alloc(bone_count);
	m_InstantState.Alloc(GetBoneCount());

	m_Offset.Load(slot);
	m_Bones2ID.clear();
	for(int i=0; i<GetBoneCount(); i++)
	{
		slot>>GetBone(i).Name;
		GetBone(i).Anima.Load(slot);
		m_Bones2ID[GetBone(i).Name]=i;
	}

	m_Duration=0;
	for(int i=0; i<GetBoneCount(); i++)
	{
		m_Duration=std::max(GetBone(i).Anima.GetLastTime(),m_Duration);
	}

	return true;
}

/***********************************************************\
|*          реализация класса SkSkin	      			   *|
\***********************************************************/
//=====================================================================================//
//                                  SkSkin::SkSkin()                                   //
//=====================================================================================//
SkSkin::SkSkin(Skin *oldskin)
:	m_Mesh(*oldskin),
	m_BonesNum(0)
{
	m_BonesNum = oldskin->LnkNum;
	SkBoneToPnts links[LinkCount];
	assert( m_BonesNum <= LinkCount );

	for(int i = 0; i < m_BonesNum; i++)
	{
		links[i].m_Points.resize(oldskin->Lnk[i].NumPoints);
		links[i].m_BoneName = oldskin->Lnk[i].BoneName;
		int rn=0;

		for(int j = 0; j < links[i].m_Points.size(); j++)
		{
			if(oldskin->Lnk[i].Points[j].Norm) continue;
			links[i].m_Points[rn].Offset = oldskin->Lnk[i].Points[j].Offset;
			links[i].m_Points[rn].Weight = oldskin->Lnk[i].Points[j].Weight;
			links[i].m_Points[rn].PartNum = oldskin->Lnk[i].Points[j].PartNum;
			links[i].m_Points[rn].PntNum = oldskin->Lnk[i].Points[j].PntNum;
			links[i].m_Points[rn].Point = &m_Mesh.Parts[links[i].m_Points[rn].PartNum]->prim.Verts[links[i].m_Points[rn].PntNum].pos;
			rn++;
		}

		links[i].m_Points.resize(rn);
	}

	m_links.Convert(links,m_BonesNum);
	MakePointPtrs();
};

//=====================================================================================//
//                                  SkSkin::~SkSkin()                                  //
//=====================================================================================//
SkSkin::~SkSkin()
{
}

//=====================================================================================//
//                            SkSkin& SkSkin::operator= ()                             //
//=====================================================================================//
SkSkin& SkSkin::operator= (const SkSkin& a)
{
	if(this != &a)
	{
		m_Mesh = a.m_Mesh;
		m_BonesNum = a.m_BonesNum;
		m_links = a.m_links;
		//std::copy_n(a.m_Links,m_BonesNum,m_Links);
		MakePointPtrs();
	}

	return *this;
}

#include  "../globals.h"

//=====================================================================================//
//                              static point3 SkMultMat()                              //
//=====================================================================================//
static point3 SkMultMat(const D3DMATRIX &m, const point3 &o)
{
	return point3(m._41 + o.x*m._11 + o.y*m._21 + o.z*m._31,
				  m._42 + o.x*m._12 + o.y*m._22 + o.z*m._32,
				  m._43 + o.x*m._13 + o.y*m._23 + o.z*m._33);
}

//=====================================================================================//
//                                void SkSkin::Update()                                //
//=====================================================================================//
void SkSkin::Update(const SkState *Skel) //обновить состояние кожи
{
	const D3DMATRIX *m[SkPntToBones::MaxBones];

	char *curptr = m_links.m_alignedData;

	float minx = 1e+10f, miny = 1e+10f, minz = 1e+10f;
	float maxx = -1e+10f, maxy = -1e+10f, maxz = -1e+10f;

	{
		for(int i = 0; i < m_links.m_pointsj[0]; ++i)
		{
			SkPntToBones::SkPoint *curpnt = (SkPntToBones::SkPoint *)(curptr);
			SkPntToBones::SkInfluence *curinflu = (SkPntToBones::SkInfluence *)(curptr + sizeof(SkPntToBones::SkPoint));

			m[0] = &Skel->GetBoneEx(curinflu[0].m_boneId).World;
			*curpnt->m_ptr = SkMultMat(*m[0],curinflu[0].m_offset);

			minx = std::min(curpnt->m_ptr->x,minx);
			miny = std::min(curpnt->m_ptr->y,miny);
			minz = std::min(curpnt->m_ptr->z,minz);
			maxx = std::max(curpnt->m_ptr->x,maxx);
			maxy = std::max(curpnt->m_ptr->y,maxy);
			maxz = std::max(curpnt->m_ptr->z,maxz);

			curptr += sizeof(SkPntToBones::SkPoint) + sizeof(SkPntToBones::SkInfluence);
		}
	}

	for(int k = 1; k < SkPntToBones::MaxBones; ++k)
	{
		for(int i = 0; i < m_links.m_pointsj[k]; ++i)
		{
			SkPntToBones::SkPoint *curpnt = (SkPntToBones::SkPoint *)(curptr);
			SkPntToBones::SkInfluence *curinflu = (SkPntToBones::SkInfluence *)(curptr + sizeof(SkPntToBones::SkPoint));

			for(int j = 0; j <= k; ++j)
			{
				m[j] = &Skel->GetBoneEx(curinflu[j].m_boneId).World;
			}

			*curpnt->m_ptr = SkMultMat(*m[0],curinflu[0].m_offset) * curinflu[0].m_weight;

			for(int j = 1; j <= k; ++j)
			{
				*curpnt->m_ptr += SkMultMat(*m[j],curinflu[j].m_offset) * curinflu[j].m_weight;
			}

			minx = std::min(curpnt->m_ptr->x,minx);
			miny = std::min(curpnt->m_ptr->y,miny);
			minz = std::min(curpnt->m_ptr->z,minz);
			maxx = std::max(curpnt->m_ptr->x,maxx);
			maxy = std::max(curpnt->m_ptr->y,maxy);
			maxz = std::max(curpnt->m_ptr->z,maxz);

			curptr += sizeof(SkPntToBones::SkPoint) + sizeof(SkPntToBones::SkInfluence)*(k+1);
		}
	}

	m_bbox.minx = minx;
	m_bbox.miny = miny;
	m_bbox.minz = minz;
	m_bbox.maxx = maxx;
	m_bbox.maxy = maxy;
	m_bbox.maxz = maxz;
}

//=====================================================================================//
//                            void SkSkin::ValidateLinks()                             //
//=====================================================================================//
void SkSkin::ValidateLinks(const SkAnim *Anim) //ставит в соответствие кости скелета и кожи
{
	char *curptr = m_links.m_alignedData;
	int masterIndex = 0;

	for(int k = 0; k < SkPntToBones::MaxBones; ++k)
	{
		for(int i = 0; i < m_links.m_pointsj[k]; ++i)
		{
			SkPntToBones::SkPoint *curpnt = (SkPntToBones::SkPoint *)(curptr);
			SkPntToBones::SkInfluence *curinflu = (SkPntToBones::SkInfluence *)(curptr + sizeof(SkPntToBones::SkPoint));

			for(int j = 0; j < k+1; ++j)
			{
				curinflu[j].m_boneId = Anim->GetBoneID(m_links.m_staticPoints[masterIndex].m_boneName[j]);
				if(curinflu[j].m_boneId < 0)
				{
					std::ostringstream sstr;
					sstr << "Не найдена кость <"
						<< m_links.m_staticPoints[masterIndex].m_boneName[j] << ">"
						<< " в анимации [" << Anim->GetAnimName() << "]";
					throw CASUS(sstr.str());
				}
			}

			curptr += sizeof(SkPntToBones::SkPoint) + sizeof(SkPntToBones::SkInfluence)*(k+1);
			++masterIndex;
		}
	}
}

//=====================================================================================//
//                               BBox SkSkin::GetBBox()                                //
//=====================================================================================//
//BBox SkSkin::GetBBox()
//{
//	return m_Mesh.GetBBox();
//}

//=====================================================================================//
//                                 bool SkSkin::Save()                                 //
//=====================================================================================//
bool SkSkin::Save(OptSlot &slot)
{
	slot << (int)SKVERSION;
	m_Mesh.Save(slot);
	m_links.Save(slot);
	return true;

	//slot << (int)SKVERSION;
	//m_Mesh.Save(slot);
	//slot << m_BonesNum;
	//for(int i = 0; i < m_BonesNum; i++)
	//{
	//	m_Links[i].Save(slot);
	//}

	//return true;
}

//=====================================================================================//
//                                 bool SkSkin::Load()                                 //
//=====================================================================================//
bool SkSkin::Load(OptSlot &slot)
{
	int i;
	slot>>i;
	if(i == SKVERSION)
	{
		m_Mesh.Load(slot);
		m_links.Load(slot);
	}

	MakePointPtrs();
	return true;
}

//=====================================================================================//
//                                bool SkSkin::Load2()                                 //
//=====================================================================================//
bool SkSkin::Load2(OptSlot &slot)
{
	int i;
	slot >> i;

	if(i == SKVERSION)
	{
		m_Mesh.Load(slot);
		m_links.Load2(slot);
	}

	MakePointPtrs();
	return true;
}

//=====================================================================================//
//                            void SkSkin::MakePointPtrs()                             //
//=====================================================================================//
void SkSkin::MakePointPtrs()
{
	char *curptr = m_links.m_alignedData;
	int masterIndex = 0;

	for(int k = 0; k < SkPntToBones::MaxBones; ++k)
	{
		for(int i = 0; i < m_links.m_pointsj[k]; ++i)
		{
			SkPntToBones::SkPoint *curpnt = (SkPntToBones::SkPoint *)(curptr);

			int partNum = m_links.m_staticPoints[masterIndex].m_partNum;
			int pntNum = m_links.m_staticPoints[masterIndex].m_pntNum;
			curpnt->m_ptr = &m_Mesh.Parts[partNum]->prim.Verts[pntNum].pos;

			m_Mesh.Parts[partNum]->prim.Verts[pntNum].pos = point3(0.0f,0.0f,0.0f);
			m_Mesh.Parts[partNum]->prim.Verts[pntNum].norm = point3(0.0f,0.0f,0.0f);
			m_Mesh.Parts[partNum]->prim.Verts[pntNum].diffuse = 0xFFFFFFFF;

			curptr += sizeof(SkPntToBones::SkPoint) + sizeof(SkPntToBones::SkInfluence)*(k+1);
			++masterIndex;
		}
	}
}

//=====================================================================================//
//                              bool SkBoneToPnts::Save()                              //
//=====================================================================================//
bool SkBoneToPnts::Save(OptSlot &slot)
{
	slot << m_BoneName << m_Points.size();

	// gvozdoder: здесь происходит уникальное явление, сохраняется указатель в файл
	slot.Write(&m_Points[0],sizeof(Pnt)*m_Points.size());

	//for(int i=0; i < m_Points.size(); i++)
	//{
	//	slot << m_Points[i].Point
	//		 << m_Points[i].Offset
	//		 << m_Points[i].Weight
	//		 << m_Points[i].PartNum
	//		 << m_Points[i].PntNum;
	//}
	return true;
}

//=====================================================================================//
//                              bool SkBoneToPnts::Load()                              //
//=====================================================================================//
bool SkBoneToPnts::Load(OptSlot &slot)
{
	int s;
	slot >> m_BoneName >> s;
	m_Points.resize(s);

	memset(&m_Points[0],0,sizeof(Pnt)*m_Points.size());
	// gvozdoder: здесь происходит уникальное явление, загружается указатель из файла
	slot.Read(&m_Points[0],sizeof(Pnt)*m_Points.size());

	//for(int i=0; i<m_Points.size(); i++)
	//{
	//	slot >> m_Points[i].Point
	//		 >> m_Points[i].Offset
	//		 >> m_Points[i].Weight
	//		 >> m_Points[i].PartNum
	//		 >> m_Points[i].PntNum;
	//}
	return true;
}

//=====================================================================================//
//                          void SkPntToBones::AcceptPoints()                          //
//=====================================================================================//
template<int N, typename R, typename U>
void SkPntToBones::AcceptPoints(const SkBoneToPnts *links, int l, const R &counts, U &influs)
{
	//if(pts.empty()) return;
	int startIndex = m_pointsoff[N-1];
	int index = startIndex;

	for(int i = 0; i < l; ++i)
	{
		for(int j = 0; j < links[i].m_Points.size(); ++j)
		{
			if(counts.at(links[i].m_Points[j].PartNum).at(links[i].m_Points[j].PntNum) == N)
			{
				int idx = -1;
				for(int m = startIndex; m < index; ++m)
				{
					if(m_staticPoints[m].m_pntNum == links[i].m_Points[j].PntNum &&
						m_staticPoints[m].m_partNum == links[i].m_Points[j].PartNum)
					{
						idx = m;
						break;
					}
				}

				if(idx == -1)
				{
					idx = index++;
				}

				int s = -1;
				for(int k = 0; k < N; ++k)
				{
					if(influs[(idx-startIndex)*N + k].m_boneId == -1)
					{
						s = k;
						break;
					}
				}

				assert( s != -1 );
				//assert( idx < pts.size() );

				influs[(idx-startIndex)*N + s].m_offset = links[i].m_Points[j].Offset;
				influs[(idx-startIndex)*N + s].m_weight = links[i].m_Points[j].Weight;
				influs[(idx-startIndex)*N + s].m_boneId = 0;
				m_staticPoints[idx].m_boneName[s] = links[i].m_BoneName;
				m_staticPoints[idx].m_pntNum = links[i].m_Points[j].PntNum;
				m_staticPoints[idx].m_partNum = links[i].m_Points[j].PartNum;
			}
		}
	}
}

//=====================================================================================//
//                            SkPntToBones::SkPntToBones()                             //
//=====================================================================================//
SkPntToBones::SkPntToBones(const SkPntToBones &pntb)
://	m_offsets(pntb.m_offsets),
	m_staticPoints(pntb.m_staticPoints)
{
	std::copy_n(pntb.m_pointsj,MaxBones,m_pointsj);
	std::copy_n(pntb.m_pointsoff,MaxBones,m_pointsoff);

	m_data.resize(pntb.m_data.size());
	AlignData();

	std::copy_n(pntb.m_alignedData,m_data.size()-Alignment,m_alignedData);
}

//=====================================================================================//
//                           void SkPntToBones::AlignData()                            //
//=====================================================================================//
void SkPntToBones::AlignData()
{
	m_alignedData = &m_data[0];
	if(((int)m_alignedData)&(Alignment-1))
	{
		int addr = (int)m_alignedData;
		addr &= ~(Alignment-1);
		addr += Alignment;
		m_alignedData = (char *)addr;
	}
}

//=====================================================================================//
//                              bool SkPntToBones::Save()                              //
//=====================================================================================//
bool SkPntToBones::Save(OptSlot &slot)
{
	slot.Write(m_pointsj, sizeof(m_pointsj[0])*MaxBones);
	slot.Write(m_pointsoff, sizeof(m_pointsoff[0])*MaxBones);

	int aldatasize = m_data.size();
	slot << aldatasize;

	char *curptr = m_alignedData;
	int masterIndex = 0;

	for(int k = 0; k < MaxBones; ++k)
	{
		for(int i = 0; i < m_pointsj[k]; ++i)
		{
			SkPoint *curpnt = (SkPoint *)(curptr);
			SkInfluence *curinflu = (SkInfluence *)(curptr + sizeof(SkPoint));
			slot.Write(curinflu,sizeof(SkInfluence)*(k+1));
			curptr += sizeof(SkPoint) + sizeof(SkInfluence)*(k+1);
		}
	}

	int sps = m_staticPoints.size();
	slot << sps;

	for(int i = 0; i < sps; ++i)
	{
		for(int j = 0; j < MaxBones; ++j) slot << m_staticPoints[i].m_boneName[j];
		slot << m_staticPoints[i].m_partNum;
		slot << m_staticPoints[i].m_pntNum;
	}

	return true;
}

//=====================================================================================//
//                             bool SkPntToBones::Load2()                              //
//=====================================================================================//
bool SkPntToBones::Load2(OptSlot &slot)
{
	slot.Read(m_pointsj, sizeof(m_pointsj[0])*MaxBones);
	slot.Read(m_pointsoff, sizeof(m_pointsoff[0])*MaxBones);

	int aldatasize;
	slot >> aldatasize;

	m_data.swap(Data_t());
	m_data.resize(aldatasize);
	AlignData();

	char *curptr = m_alignedData;
	int masterIndex = 0;

	for(int k = 0; k < MaxBones; ++k)
	{
		for(int i = 0; i < m_pointsj[k]; ++i)
		{
			SkPoint *curpnt = (SkPoint *)(curptr);
			SkInfluence *curinflu = (SkInfluence *)(curptr + sizeof(SkPoint));
			slot.Read(curinflu,sizeof(SkInfluence)*(k+1));
			curptr += sizeof(SkPoint) + sizeof(SkInfluence)*(k+1);
		}
	}

	int sps;
	slot >> sps;
	m_staticPoints.swap(StaticPoints_t());
	m_staticPoints.resize(sps);

	for(int i = 0; i < sps; ++i)
	{
		for(int j = 0; j < MaxBones; ++j) slot >> m_staticPoints[i].m_boneName[j];
		slot >> m_staticPoints[i].m_partNum;
		slot >> m_staticPoints[i].m_pntNum;
	}

	return true;
}

//=====================================================================================//
//                            void SkPntToBones::Convert()                             //
//=====================================================================================//
void SkPntToBones::Convert(SkBoneToPnts *links, int l)
{
	std::vector< std::vector<int> > counts;
	counts.resize(32);
	for(int i = 0; i < 32; ++i)
	{
		counts[i].reserve(1024);
	}

	int cnt[8] = {0,0,0,0,0,0,0,0};
	for(int i = 0; i < l; ++i)
	{
		for(int j = 0; j < links[i].m_Points.size(); ++j)
		{
			int idx = links[i].m_Points[j].PntNum;
			int pidx = links[i].m_Points[j].PartNum;

			if(pidx >= counts.size())
			{
				size_t olds = counts.size();
				counts.resize(pidx+1);
				for(size_t k = olds; k != counts.size(); ++k)
				{
					counts[k].reserve(1024);
				}
			}

			if(idx >= counts[pidx].size())
			{
				while(idx >= counts[pidx].capacity()) counts[pidx].reserve(counts[pidx].capacity()*2);
				counts[pidx].resize(idx+1);
			}

			if(counts[pidx][idx]) cnt[counts[pidx][idx]-1] -= 1;
			cnt[counts[pidx][idx]] += 1;

			++counts[pidx][idx];
		}
	}

	std::copy_n(cnt,MaxBones,m_pointsj);

	for(int i = 0; i < MaxBones; ++i)
	{
		m_pointsoff[i] = std::accumulate(m_pointsj,m_pointsj+i,0);
	}

	Influences_t influs[MaxBones];

	m_staticPoints.resize(std::accumulate(m_pointsj,m_pointsj+MaxBones,0));

	for(int i = 0; i < MaxBones; ++i)
	{
		influs[i].resize((i+1)*cnt[i]);
	}

	AcceptPoints<1>(links,l,counts,influs[0]);
	AcceptPoints<2>(links,l,counts,influs[1]);
	AcceptPoints<3>(links,l,counts,influs[2]);
	AcceptPoints<4>(links,l,counts,influs[3]);
	AcceptPoints<5>(links,l,counts,influs[4]);

	int dataSize = 0;

	for(int i = 0; i < MaxBones; ++i)
	{
		dataSize += m_pointsj[i]*sizeof(SkPoint);
		dataSize += m_pointsj[i]*(i+1)*sizeof(SkInfluence);
	}

	m_data.resize(dataSize + Alignment);
	AlignData();

	int masterIndex = 0;
	int offset = 0;
	for(int i = 0; i < MaxBones; ++i)
	{
		for(int j = 0; j < m_pointsj[i]; ++j)
		{
			//m_offsets[masterIndex++] = offset;

			SkPoint *pt = (SkPoint *)(m_alignedData + offset);
			pt->m_ptr = 0;

			offset += sizeof(SkPoint);

			SkInfluence *infl = (SkInfluence *)(m_alignedData + offset);
			for(int k = 0; k < i+1; ++k)
			{
				infl[k] = influs[i][j*(i+1) + k];
			}

			offset += (i+1)*sizeof(SkInfluence);
		}
	}
}

//=====================================================================================//
//                              bool SkPntToBones::Load()                              //
//=====================================================================================//
bool SkPntToBones::Load(OptSlot &slot)
{
	SkBoneToPnts links[256];
	int l;
	slot >> l;
	for(int i = 0; i < l; ++i) links[i].Load(slot);

	Convert(links,l);

	return true;
}
