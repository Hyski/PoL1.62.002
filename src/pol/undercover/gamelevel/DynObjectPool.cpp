#include "precomp.h"

#include "Grid.h"
#include "ALDMgr.h"
#include "SndAuto.h"
#include "ProgrBar.h"
#include "../IWorld.h"
#include "GameLevel.h"
#include <boost/bind.hpp>
#include "DynObjectSet.h"
#include "DynObjectPool.h"
#include "DynObjectOldData.h"
#include <Logic2/HexUtils.h>
#include <boost/tuple/tuple.hpp>
#include <Common/GraphPipe/GraphPipe.h>
#include <Common/3DEffects/EffectManager.h>
#include <Common/GraphPipe/StatesManager.h>

#include <mll/utils/named_vars.h>

#ifdef new
#undef new
#endif

#include <Vis/VisMesh.h>
#include <Vis/VisMeshMgr.h>
#include <Vis/VisMeshState.h>

static MlVersion g_dynObjectPoolVersion(0,0);

namespace DynObjectPoolDetail
{

//=====================================================================================//
//                                struct DynObjCompare                                 //
//=====================================================================================//
struct DynObjCompare
{
	bool operator()(const HDynObject &l, const HDynObject &r) const
	{
		return l->GetName() < r->GetName();
	}

	bool operator()(const std::string &l, const HDynObject &r) const
	{
		return l < r->GetName();
	}

	bool operator()(const HDynObject &l, const std::string &r) const
	{
		return l->GetName() < r;
	}
};

//=====================================================================================//
//                               struct DynObjSetCompare                               //
//=====================================================================================//
struct DynObjSetCompare
{
	bool operator()(const HDynObjectSet &l, const HDynObjectSet &r) const
	{
		return l->GetName() < r->GetName();
	}

	bool operator()(const HDynObjectSet &l, const std::string &r) const
	{
		return l->GetName() < r;
	}

	bool operator()(const std::string &l, const HDynObjectSet &r) const
	{
		return l < r->GetName();
	}
};

//=====================================================================================//
//                              class DynObjectPool::Impl                              //
//=====================================================================================//
class DynObjectPoolImpl
{
public:
	typedef std::vector<HDynObject> Objects_t;
	typedef std::vector<HDynObjectSet> ObjectSets_t;

	typedef Objects_t::iterator ObjectsIt;

	Objects_t m_objects;
	ObjectSets_t m_objSets;

	ObjectSets_t m_updates;
	ObjectSets_t m_updateUsables;

	ShapePool m_Shapes;

	void MakeSaveLoad(SavSlot &s, const std::string &levname)
	{
		if(s.IsSaving()) MakeSave(s); else MakeLoad(s,levname);
	}

	ShapePool *GetShapePool() { return &m_Shapes; }
	void Update(float Time);
	void UpdateHG(DynObject *obj);

	float SwitchState(const std::string &, float);
	void DoExplosion(const std::string &, const hit_s &, LogicDamageStrategy *);

	bool TraceRay(const point3 &, const point3 &, point3 *, point3 *, std::string *, unsigned int);// DynObjectPool::TRACE_TYPE);
	bool TraceSegment(const point3 &, const point3 &, point3 *, point3 *, std::string *);
	bool TraceSegment2(const point3 &, const point3 &, point3 *, point3 *, std::string *, unsigned int);// DynObjectPool::TRACE_TYPE);

	void EndTurn();

	void Clear();
	void ClearObjectInfluence();
	void EraseByName(const std::string &);
	void CollectHexesForObject(const std::string &, std::vector<ipnt2_t> *);
	void CollectObjectsForPoints(const std::vector<point3> &, std::set<std::string> *);

	void Draw(GraphPipe *pipe, bool trans);
	void StartSounds();

	void Bake();

	void LoadAlfa(ALD::Manager &, const std::string &);

protected:
	~DynObjectPoolImpl() {}

private:
	void MakeSave(SavSlot &s);
	void MakeLoad(SavSlot &s, const std::string &);
	void MakeLoad4(SavSlot &s, const std::string &);
	void MakeLoad5(SavSlot &s);
};

//=====================================================================================//
//                          void DynObjectPoolImpl::EndTurn()                          //
//=====================================================================================//
void DynObjectPoolImpl::EndTurn()
{
	for(ObjectSets_t::iterator it = m_objSets.begin(); it != m_objSets.end(); ++it)
	{
		(*it)->EndTurn();
	}
}

//=====================================================================================//
//                        void DynObjectPoolImpl::StartSounds()                        //
//=====================================================================================//
void DynObjectPoolImpl::StartSounds()
{
	Objects_t::iterator it = m_objects.begin(), ite = m_objects.end();

	for(; it != ite; it++)
	{
		(*it)->StartSound();
	}
}

//=====================================================================================//
//                      void DynObjectPool::Impl::MakeSaveLoad()                       //
//=====================================================================================//
void DynObjectPoolImpl::MakeSave(SavSlot &slot)
{
	g_dynObjectPoolVersion.Store(slot);
	slot << std::string("gamedata");

	slot << m_objSets.size();
	for(ObjectSets_t::iterator i = m_objSets.begin(); i != m_objSets.end(); ++i)
	{
		slot << (*i)->GetName();
	}

	for(ObjectSets_t::iterator i = m_objSets.begin(); i != m_objSets.end(); ++i)
	{
		(*i)->MakeSave(slot);
	}

	slot << m_objects.size();
	for(Objects_t::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		(*i)->MakeSave(slot);
	}
}

//=====================================================================================//
//                        void DynObjectPool::Impl::MakeLoad4()                        //
//=====================================================================================//
void DynObjectPoolImpl::MakeLoad4(SavSlot &s, const std::string &levname)
{
	int count;	// Количество DynObject'ов

	GetShapePool()->Clear();
	m_objSets.clear();
	m_objects.clear();

	typedef std::vector<DynObjectOldData> Data_t;
	Data_t data;

	s >> count;

	data.resize(count);
	m_objects.resize(count);

	if(!count) return;

	for(int j=0; j<count; j++)
	{
		ProgrBar::SetMofN(j,count);

		DynObjectOldData &objdata = data[j];
		objdata.Read(s);

		HDynObject obj = m_objects[j] = new DynObject(objdata);
		obj->MakeLoad(s);
	}

	std::sort(data.begin(),data.end());
	std::sort(m_objects.begin(),m_objects.end(),DynObjCompare());

	for(Data_t::iterator i = data.begin();
		i != data.end();
		i = std::find_if(i,data.end(),
			boost::bind(
				std::not_equal_to<std::string>(),
				boost::bind(DynObjectOldData::GetName,_1),
				i->GetName()
				)
			)
		)
	{
		DynObjectSetInfo dosinfo(*i,levname);
		Objects_t::iterator beg,end;
		boost::tie(beg,end) = std::equal_range(m_objects.begin(),m_objects.end(),i->GetName(),DynObjCompare());
		HDynObjectSet dset = new DynObjectSet(dosinfo,beg,end);
		m_objSets.push_back(dset);
	}

	std::sort(m_objSets.begin(),m_objSets.end(),DynObjSetCompare());

	for(Objects_t::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		if((*i)->HasGeometry())
		{
			(*i)->SetShapeHandle(GetShapePool()->AddShape((*i)->GetTexObject()));
		}
	}

	GetShapePool()->CreateVB();
	Bake();
	Update(0.0f);
}

//=====================================================================================//
//                             static int getFloorNumber()                             //
//=====================================================================================//
static int getFloorNumber(const std::string &id)
{
	int result = 0;
	std::string::size_type pos = id.find_last_not_of("0123456789");
	std::istringstream sstr(id.substr(pos+1));
	sstr >> result;
	assert( !sstr.fail() );
	return result;
}

//=====================================================================================//
//                       static void convertVisMeshToTexObject()                       //
//=====================================================================================//
static void convertVisMeshToTexObject(Vis::HMesh mesh, TexObject *texobj)
{
	assert( std::distance(mesh->begin(),mesh->end()) == 1 );
	const Vis::Mesh::NodeData *node = &*mesh->begin();

	texobj->PartNum = node->m_pieces.size();

	for(Vis::Mesh::NodeData::MeshPieces_t::const_iterator i = node->m_pieces.begin(); i != node->m_pieces.end(); ++i)
	{
		int idx = std::distance(node->m_pieces.begin(),i);
		Vis::MeshPiece *piece = *i;

		Vis::ChannelPosition3 *pos = piece->getChannel(Vis::channelIdPos);
		Vis::ChannelNormal3 *normal = piece->getChannel(Vis::channelIdNormal);
		Vis::ChannelTexCoord2 *uv = piece->getChannel(Vis::channelIdTexCoord);
		assert( pos->getSize() == normal->getSize() );
		assert( pos->getSize() == uv->getSize() );

		Vis::ChannelIndex16 *indices = piece->getChannel(Vis::channelIdIndex);

		const int vcount = pos->getSize();
		const int icount = indices->getSize();
		assert( vcount != 0 );
		assert( icount != 0 );

		SimpleTexturedObject *sto = texobj->Parts[idx] = new SimpleTexturedObject;
		sto->prim.Verts.resize(vcount);
		sto->prim.Idxs.resize(icount);

		memset(&sto->prim.Verts[0],0,sizeof(sto->prim.Verts[0])*vcount);
		memset(&sto->prim.Idxs[0],0,sizeof(sto->prim.Idxs[0])*icount);

		for(int j = 0; j < vcount; ++j)
		{
			sto->prim.Verts[j].pos = pos->getData()[j];
			sto->prim.Verts[j].norm = normal->getData()[j];
			sto->prim.Verts[j].uv = uv->getData()[j];
		}

		std::copy_n(&indices->getData()[0],icount,&sto->prim.Idxs[0]);
		sto->MaterialName = piece->getPiece()->getMaterial()->getName();
	}
}

typedef mll::utils::animation<mll::utils::l_interpolator<mll::algebra::point3> > AnimPos_t;
typedef mll::utils::animation<mll::utils::l_interpolator<mll::algebra::quaternion> > AnimOrient_t;

//=====================================================================================//
//                          static void convertFromMllAnim()                           //
//=====================================================================================//
static void convertFromMllAnim(const AnimPos_t &posa,
							   const AnimOrient_t &orienta,
							   KeyAnimation *anim)
{
	for(AnimPos_t::const_iterator i = posa.begin(); i != posa.end(); ++i)
	{
		::point3 v(i->second);
		anim->SetKey(i->first,v);
	}

	for(AnimOrient_t::const_iterator i = orienta.begin(); i != orienta.end(); ++i)
	{
		::Quaternion q(i->second);
		anim->SetKey(i->first,q);
	}
}

//=====================================================================================//
//                         void DynObjectPoolImpl::LoadAlfa()                          //
//=====================================================================================//
void DynObjectPoolImpl::LoadAlfa(ALD::Manager &manager, const std::string &levname)
{
	GetShapePool()->Clear();
	m_objSets.clear();
	m_objects.clear();

	typedef std::map<unsigned int, ShapePool::Handle> SharedMeshes_t;

	MlResourceMgr rmgr;
	Vis::MeshMgr meshmgr(0);

	SharedMeshes_t smeshes;

	using ALD::Manager;
	Manager::folder_iterator mesh_folder = manager.ifolder("general").ifolder("shared").ifolder("mesh");
	
	for(Manager::item_iterator i = mesh_folder.ibegin(); i != mesh_folder.iend(); ++i)
	{
		TexObject tmp_object;
		Vis::HMesh mesh = meshmgr.embedMesh(i->id(),i->ibinstream().stream(),0);
		const unsigned int id = boost::lexical_cast<unsigned int>(i->id());

		assert( smeshes.find(id) == smeshes.end() );
		convertVisMeshToTexObject(mesh,&tmp_object);
		smeshes.insert(std::make_pair(id,GetShapePool()->AddShape(&tmp_object)));
	}

	int count = 0;

	Manager::folder_iterator floors_folder = manager.ifolder("general").ifolder("floors");//.ifolder("floor1").ifolder("object");
	for(Manager::folder_iterator i = floors_folder.fbegin(); i != mesh_folder.fend(); ++i)
	{
		count += std::distance(i.ifolder("object").ibegin(),i.ifolder("object").iend());
	}

	if(!count) return;

	typedef std::vector<DynObjectOldData> Data_t;
	Data_t data;

	data.resize(count);
	m_objects.resize(count);

	int idx = 0;

	for(Manager::folder_iterator i = floors_folder.fbegin(); i != mesh_folder.fend(); ++i)
	{
		Manager::folder_iterator object_folder = i.ifolder("object");
		int floor = getFloorNumber(i->id());

		for(Manager::item_iterator j = object_folder.ibegin(); j != object_folder.iend(); ++j, ++idx)
		{
			mll::utils::named_vars vars;
			AnimPos_t animation_p;
			AnimOrient_t animation_q;
			unsigned int geometry_id;
			mll::algebra::matrix3 world;

			//
			j->ibinstream() >> vars >> geometry_id >> world >> animation_p >> animation_q;
			//assert(world._33==1.0f);

			KeyAnimation tmp_anim;
			convertFromMllAnim(animation_p, animation_q, &tmp_anim);

			DynObjectOldData &objdata = data[idx];
			objdata.ReadVars(vars,floor);

			HDynObject obj = m_objects[idx] = new DynObject(objdata);
			obj->SetTexObject(*GetShapePool()->GetShape(smeshes[geometry_id])->GetGeometry());
			obj->SetShapeHandle(smeshes[geometry_id]);
			obj->SetAnimation(tmp_anim);
			obj->SetWorldMatrix(mll::algebra::D3DMATRIX(world));
			obj->SetInvWorldMatrix(mll::algebra::D3DMATRIX(mll::algebra::matrix3(world).invert()));

			obj->UpdateAfterLoad();
		}
	}

	std::sort(data.begin(),data.end());
	std::sort(m_objects.begin(),m_objects.end(),DynObjCompare());

	for(Data_t::iterator i = data.begin();
		i != data.end();
		i = std::find_if(i,data.end(),
			boost::bind(
				std::not_equal_to<std::string>(),
				boost::bind(DynObjectOldData::GetName,_1),
				i->GetName()
				)
			)
		)
	{
		DynObjectSetInfo dosinfo(*i,levname);
		Objects_t::iterator beg, end;
		boost::tie(beg,end) = std::equal_range(m_objects.begin(),m_objects.end(),i->GetName(),DynObjCompare());
		HDynObjectSet dset = new DynObjectSet(dosinfo,beg,end);
		m_objSets.push_back(dset);
	}

	std::sort(m_objSets.begin(),m_objSets.end(),DynObjSetCompare());

	GetShapePool()->CreateVB();
	Bake();
	Update(0.0f);
}

//=====================================================================================//
//                        void DynObjectPool::Impl::MakeLoad5()                        //
//=====================================================================================//
void DynObjectPoolImpl::MakeLoad5(SavSlot &slot)
{
	MlVersion ver;
	ver.Restore(slot);

	if(!ver.IsCompatible(g_dynObjectPoolVersion))
	{
		throw CASUS("Неизвестная версия сейва");
	}

	std::string tag;
	slot >> tag;

	assert( tag == "gamedata" );

	unsigned int count;
	slot >> count;	// Количество DynObjectSet'ов

	// Прочитаем названия объектов, которые должны существовать
	typedef std::vector<std::string> Names_t;
	Names_t names(count);
	for(unsigned int i = 0; i < count; ++i)
	{
		slot >> names[i];
	}

	for(ObjectSets_t::iterator i = m_objSets.begin(); i != m_objSets.end();)
	{
		if(!std::binary_search(names.begin(),names.end(),(*i)->GetName()))
		{
			EraseByName((*i)->GetName());
		}
		else
		{
			if(names[i-m_objSets.begin()] != (*i)->GetName())
			{
				assert( !"DynObjectPoolImpl::MakeLoad5: Inconsistent save; name of object set does not project to saved game" );
				POL_LOG("DynObjectPoolImpl::MakeLoad5: Inconsistent save; name of object set does not project to saved game\n");
				return;
			}
			(*i++)->MakeLoad(slot);
		}
	}

	slot >> count;	// Количество DynObject'ов
	if(count != m_objects.size())
	{
		assert( !"DynObjectPoolImpl::MakeLoad5: Inconsistent save; name of object does not project to saved game" );
		POL_LOG("DynObjectPoolImpl::MakeLoad5: Inconsistent save; name of object does not project to saved game\n");
		return;
	}

	for(unsigned int i = 0; i < count; ++i)
	{
		m_objects[i]->MakeLoad(slot);
	}

	Bake();
	Update(0.0f);
}

//=====================================================================================//
//                        void DynObjectPool::Impl::MakeLoad()                         //
//=====================================================================================//
void DynObjectPoolImpl::MakeLoad(SavSlot &s, const std::string &levname)
{
	if(s.GetStore()->GetVersion() > 4)
	{
		MakeLoad5(s);
	}
	else
	{
		MakeLoad4(s,levname);
	}
}

//=====================================================================================//
//                           void DynObjectPoolImpl::Bake()                            //
//=====================================================================================//
void DynObjectPoolImpl::Bake()
{
	for(ObjectSets_t::iterator i = m_objSets.begin(); i != m_objSets.end(); ++i)
	{
		(*i)->Bake();
		if((*i)->EverNeedUpdate()) m_updates.push_back(*i);
		if((*i)->EverNeedUpdateUsable()) m_updateUsables.push_back(*i);
	}
}

//=====================================================================================//
//                         void DynObjectPool::Impl::Update()                          //
//=====================================================================================//
void DynObjectPoolImpl::Update(float tau)
{
	for(ObjectSets_t::iterator i = m_updates.begin(); i != m_updates.end(); ++i)
	{
		(*i)->Update(tau);
	}
	for(ObjectSets_t::iterator i = m_updateUsables.begin(); i != m_updateUsables.end(); ++i)
	{
		(*i)->UpdateUsable(tau);
	}
}

//=====================================================================================//
//                                static bool CanPass()                                //
//=====================================================================================//
static bool CanPass(const ipnt2_t &np)
{
	const HexGrid::cell &cell = HexGrid::GetInst()->Get(np);

	for(int p=0; p<6; p++)
	{
		if(cell.GetWeight(p)<50) return true;
	}

	return false;
}

//=====================================================================================//
//                      float DynObjectPool::Impl::SwitchState()                       //
//=====================================================================================//
float DynObjectPoolImpl::SwitchState(const std::string &name, float time)
{
	ObjectSets_t::iterator i, ie;
	boost::tie(i,ie) = std::equal_range(m_objSets.begin(),m_objSets.end(),name,DynObjSetCompare());
	if(i == ie) return 0.0f;

	HDynObjectSet dset = *i;
	return dset->SwitchState(time);
}

//=====================================================================================//
//                                  class rec_handler                                  //
//=====================================================================================//
class rec_handler
{
private: int &c;
public:
	rec_handler(int &counter):c(counter){c++;};
	~rec_handler(){c--;}

};

//=====================================================================================//
//                       void DynObjectPool::Impl::DoExplosion()                       //
//=====================================================================================//
void DynObjectPoolImpl::DoExplosion(const std::string &objname, const hit_s &hit,
									  LogicDamageStrategy *lds)
{
	 // refactoring
	static int recursion_level=0;
	rec_handler rh(recursion_level);
	if(recursion_level>3) return;

	typedef std::set<std::string> ObjectNames;
	ObjectNames ObjNames;

	hit_s NewHit = hit;
	for(int n = 0; n < hit_s::MAX_DMSGS; n++)
	{
		// Если повреждение не из этих типов, то оно не влияет на неживые объекты
		if(NewHit.m_dmg[n].Type != hit_s::DT_STRIKE &&
			NewHit.m_dmg[n].Type != hit_s::DT_FLAME &&
			NewHit.m_dmg[n].Type != hit_s::DT_EXPLOSIVE)
		{
			NewHit.m_dmg[n].Type = hit_s::DT_NONE;
		}

		if(lds && NewHit.m_dmg[n].Type == hit_s::DT_FLAME)
		{
			lds->LightFire(NewHit.m_to, NewHit.m_radius, NewHit.m_dmg[n].Value);
		}
	}

	if(hit.m_radius > 0.0f)
	{
		for(ObjectsIt it = m_objects.begin(); it != m_objects.end(); it++)
		{
			HDynObject obj = *it;
			if(!obj->IsDestructible()) continue;

			int HP = obj->GetDestructibleInfo().m_hitPoints;

			/// Если нет повреждения или максимальное возможное повреждение превышает хит-пойнты,
			/// то переходим к следующему объекту
			if( (NewHit.m_dmg[0].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[0].Value) &&
				(NewHit.m_dmg[1].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[1].Value) &&
				(NewHit.m_dmg[2].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[2].Value))
			{
				continue;
			}

			float falloff = 1.0f;

			//if(obj.Name!=objname)
			//{
			float dist = obj->GetBound().DistToPoint(hit.m_to);
			if(dist > hit.m_radius) continue;
			falloff = dist > 1.0f ? 1.0f/(dist*dist*dist) : 1.0f;
			//}

			//!FIXME:
			// FIXME что?
			// Здесь, если нет повреждения или повреждение меньше хит-пойнтов, то переходим к следующему объекту
			if( (NewHit.m_dmg[0].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[0].Value*falloff) &&
				(NewHit.m_dmg[1].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[1].Value*falloff) &&
				(NewHit.m_dmg[2].Type == hit_s::DT_NONE || HP>=NewHit.m_dmg[2].Value*falloff))
			{
				continue;
			}

			ObjNames.insert(obj->GetName());
			//obj->StopSound();
			//Это чтобы правильно изменить зоны проходимости
			//ChangeHG(obj->GetName(),1,obj->GetWorldMatrix()._43);
		}
	}
	else
	{
		ObjNames.insert(objname);
	}

	//инициируем взрыв всех объектов
	std::vector<hit_s> Hits;
	for(ObjectNames::iterator i = ObjNames.begin(); i != ObjNames.end(); i++)
	{
		ObjectSets_t::iterator it, ite;
		boost::tie(it,ite) = std::equal_range(m_objSets.begin(),m_objSets.end(),*i,DynObjSetCompare());

		if(it != ite && (*it)->GetObjectCount() && !(*it)->Destroyed() && (*it)->Visible())
		{
			HDynObject obj = *(*it)->Begin();
			hit_s Hit = obj->GetDestructibleInfo().m_damage;
			Hit.m_from = Hit.m_to = obj->GetBound().GetCenter();
			Hits.push_back(Hit);
			(*it)->Destruct();

			if(lds)
			{
				lds->DestroyObject(*i);
			}

			IWorld::Get()->GetEffects()->DestroyObjectByExplosion(hit.m_to, **it);
		}

		//if(LogicStrategy) LogicStrategy->LightFire(hit.m_to, hit.m_radius, 15/*?*/);
	}

	//а теперь обработка взрыва всех собранных объектов
	return;//fixme:
	for(int j=0;j<Hits.size();j++)
	{
		ExplosionManager::Get()->OnAirHit(-1,Hits[j]);
	}
}

//=====================================================================================//
//                        bool DynObjectPool::Impl::TraceRay()                         //
//=====================================================================================//
bool DynObjectPoolImpl::TraceRay(const point3 &origin, const point3 &dir,
								   point3 *hit, point3 *normal,
								   std::string *name,
								   unsigned int passThru)
								   //DynObjectPool::TRACE_TYPE tt)
{
	const float BOGUS = 1e+6f;
	float dist = BOGUS;
	point3 norm, pnt;
	float d;

	ObjectsIt it = m_objects.begin(), ite = m_objects.end();
	ObjectsIt rit = ite;

	for(;it != ite; it++)
	{
		HDynObject obj = *it;
		unsigned int objdesc = obj->GetTraceParams();

		if(objdesc && (passThru & objdesc)) continue;
		if(!obj->GetSet() || !obj->GetSet()->Visible()) continue;

		if(obj->TraceRay(ray(origin,dir),&d,&norm))
		{
			if(d < dist)
			{
				rit = it;
				dist = d;
				*normal = norm;
			}
		}
	}

	if(rit != ite)
	{
		*hit = origin + dist*dir;
		*name = (*rit)->GetSet()->GetName();
		return true;
	}
	else
	{
		return false;
	}
}

//=====================================================================================//
//                       bool DynObjectPoolImpl::TraceSegment()                        //
//=====================================================================================//
bool DynObjectPoolImpl::TraceSegment2(const point3 &origin, const point3 &dir,
								   point3 *hit, point3 *normal,
								   std::string *name,
								   unsigned int passThru)
//								   DynObjectPool::TRACE_TYPE tt)
{
	const float BOGUS = 1e+6f;
	float dist = BOGUS;
	point3 norm, pnt;
	float d;

	ObjectsIt it = m_objects.begin(), ite = m_objects.end();
	ObjectsIt rit = ite;

	for(;it != ite; it++)
	{
		HDynObject obj = *it;
		unsigned int objdesc = obj->GetTraceParams();

		if(objdesc && (passThru & objdesc)) continue;

		//if(tt == DynObjectPool::TT_SIGHT)
		//{
		//	if(obj->IsTransparent()) continue;
		//}
		//else if(tt == DynObjectPool::TT_PHYSIC)
		//{
		//	if(obj->IsGhost()) continue;
		//}

		if(obj->TraceSegment(ray(origin,dir),&d,&norm))
		{
			if(d < dist)
			{
				rit = it;
				dist = d;
				*normal = norm;
			}
		}
	}

	if(rit != ite)
	{
		*hit = origin + dist*dir;
		*name = (*rit)->GetSet()->GetName();
		return true;
	}
	else
	{
		return false;
	}
}

//=====================================================================================//
//                      bool DynObjectPool::Impl::TraceSegment()                       //
//=====================================================================================//
bool DynObjectPoolImpl::TraceSegment(const point3 &origin, const point3 &dir, point3 *hit,
									   point3 *normal, std::string *name)
{
	const float BOGUS = 1.0f;
	float dist=BOGUS;
	point3 norm,pnt;
	float d;

	point3 start_pnt = origin, end_pnt = origin+dir, mid_pnt = origin+0.5f*dir;
	float Len=dir.Length()*0.5f; //половина длины сегмента
	ObjectsIt it = m_objects.begin(), ite = m_objects.end();
	ObjectsIt rit = ite;

	for(;it!=ite;it++)
	{
		HDynObject obj = *it;
		if(obj->IsGhost()) continue;//FIXME: нужен флаг
		if(obj->GetBound().DistToPoint(mid_pnt)>Len) continue;

		if(obj->TraceRay(ray(origin,dir),&d,&norm))
		{
			if(d <= dist)
			{
				rit = it;
				dist = d;
				*normal = norm;
			}
		}
	}

	if(rit != ite)
	{
		*hit = origin + dist*dir;
		*name = (*rit)->GetSet()->GetName();
		return true;
	}
	else
	{
		return false;
	}
}

//=====================================================================================//
//                          void DynObjectPool::Impl::Clear()                          //
//=====================================================================================//
void DynObjectPoolImpl::Clear()
{
	m_objSets.clear();
	m_objects.clear();
	m_Shapes.Clear();
}

//=====================================================================================//
//                  void DynObjectPool::Impl::ClearObjectInfluence()                   //
//=====================================================================================//
void DynObjectPoolImpl::ClearObjectInfluence()
{
	//// refactoring
	//ObjectsIt it = m_Objects.begin(), ite = m_Objects.end();
	//Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	//if(!LevelGrid) return;

	//for(; it != ite; ++it)
	//{
	//	DynObject &obj=it->second;
	//	ChangeHG(obj.GetName(),1,obj.GetWorldMatrix()._43);
	//}
}

//=====================================================================================//
//                       void DynObjectPool::Impl::EraseByName()                       //
//=====================================================================================//
void DynObjectPoolImpl::EraseByName(const std::string &name)
{
	ObjectSets_t::iterator i, ie;
	boost::tie(i,ie) = std::equal_range(m_objSets.begin(),m_objSets.end(),name,DynObjSetCompare());
	if(i == ie) return;

	Objects_t::iterator io, ioe;
	boost::tie(io,ioe) = std::equal_range(m_objects.begin(),m_objects.end(),name,DynObjCompare());

	m_objSets.erase(i);
	m_objects.erase(io,ioe);
}

//=====================================================================================//
//                  void DynObjectPool::Impl::CollectHexesForObject()                  //
//=====================================================================================//
void DynObjectPoolImpl::CollectHexesForObject(const std::string &name, std::vector<ipnt2_t> *vec)
{
	// refactoring
	//три варианта:
	//1.объект с динамической проходимостью
	//2.объект с динамической высотой
	//3.произвольный объект
	ipnt2_t np;
	Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	HexGrid::CellVec *vect;

	vect=&LevelGrid->ManDirect[name];
	if(vect->size())
	{
		for(int i=0;i<vect->size();i++)
		{
			vec->push_back((*vect)[i]);
			for(int p=0;p<6;p++)
			{
				HexUtils::GetFrontPnts0((*vect)[i], p, &np);
				if(LevelGrid->IsOutOfRange(np)) 	continue;
				if(LevelGrid->Get((*vect)[i]).GetProperty()==LevelGrid->Get(np).GetProperty()) continue;
				vec->push_back(np);
			}
		}
		return;
	}


	vect=&LevelGrid->HeightDirect[name];
	if(vect->size())
	{
		for(int i=0;i<vect->size();i++)
		{
			vec->push_back((*vect)[i]);
		}
		return;
	}


	int cx,cy;
	ObjectSets_t::iterator it, ite;
	boost::tie(it,ite) = std::equal_range(m_objSets.begin(),m_objSets.end(),name,DynObjSetCompare());
	if(it == ite) return;

	HDynObjectSet dset = *it;

	for(DynObjectSet::Iterator_t st = dset->Begin(); st != dset->End(); st++)
	{
		//1. найдем центральную точку
		DynObject &obj = **st;
		point3 pnt=obj.GetLocalBound().GetCenter(),tpnt;
		PointMatrixMultiply(*(D3DVECTOR*)&tpnt,*(D3DVECTOR*)&pnt,obj.GetWorldMatrix());
		ipnt2_t hpnt=HexUtils::scr2hex(tpnt);
		//2. отступим немного
		float rad=obj.GetLocalBound().GetBigRadius()+2;
		int stepx=ceil(rad);
		int stepy=1.f/sin(TORAD(60))*ceil(rad);

		//3. пройдемся по полученному множеству и пометим близкие к объекту хексы
		for(cx=hpnt.x-stepx;cx<hpnt.x+stepx;cx++)
		{
			for(cy=hpnt.y-stepy;cy<hpnt.y+stepy;cy++)
			{
				if(cx<0||cy<0||cx>=HexGrid::GetInst()->GetSizeX()||cy>=HexGrid::GetInst()->GetSizeY())
					continue;
				HexGrid::cell *c=&HexGrid::GetInst()->Get(ipnt2_t(cx,cy));

				point3 hexpos=*c;
				point3 pnt;
				PointMatrixMultiply(*(D3DVECTOR*)&pnt,*(D3DVECTOR*)&hexpos,obj.GetInvWorldMatrix());
				float dist=obj.GetLocalBound().DistToPoint(pnt);
				if(dist>2) continue;
				vec->push_back(ipnt2_t(cx,cy));
			}
		}
	}
}

//=====================================================================================//
//                 void DynObjectPool::Impl::CollectObjectsForPoints()                 //
//=====================================================================================//
void DynObjectPoolImpl::CollectObjectsForPoints(const std::vector<point3> &pnts, std::set<std::string> *objs)
{
	POL_LOG("DynObjectPoolImpl::CollectObjectsForPoints: called function which must not be called\n");
	//// refactoring
	//if(!objs) return;
	//int num_pnts=pnts.size();

	//ObjectsIt it=m_Objects.begin(),ite = m_Objects.end();
	//for(;it!=ite;it++)
	//{
	//	DynObject &obj=it->second;
	//	if(objs->find(obj.GetName()) != objs->end()) continue;
	//	for(int i = 0; i < num_pnts; ++i)
	//	{
	//		if(obj.GetBound().DistToPoint(pnts[i])<1.f)
	//		{
	//			objs->insert(obj.GetName());
	//			break;
	//		}
	//	}
	//}
}

//=====================================================================================//
//                                     struct Data                                     //
//=====================================================================================//
struct DrawData
{
	HDynObject m_obj;
	float m_dist;

	bool operator<(const DrawData &d) const { return m_dist > d.m_dist; }
};

//=====================================================================================//
//                          void DynObjectPool::Impl::Draw()                           //
//=====================================================================================//
void DynObjectPoolImpl::Draw(GraphPipe *pipe, bool trans)
{
	//int oc=0;
	ObjectsIt it = m_objects.begin(), ite = m_objects.end();
	bool noroofs = !IWorld::Get()->GetLevel()->IsRoofVisible();

	std::vector<DrawData> objs;

	const Frustum *f = &pipe->GetCam()->Cone;
	point3 campos = pipe->GetCam()->GetPos();

	for(; it!=ite; ++it)
	{
		HDynObject obj = *it;

		if(noroofs && obj->IsRoof()) continue;
		if(!obj->GetSet() || (obj->GetSet()->Destroyed() || !obj->GetSet()->Visible())) continue;

		//FIXME: необходимо делать это гораздо реже
		if(f->TestBBox(obj->GetBound())==Frustum::NOTVISIBLE) continue;

		if(obj->IsTransparentShader() == trans)
		{
			DrawData dta = {obj,(obj->GetBound().GetCenter() - campos).Length()};
			objs.push_back(dta);
		}
	}

	if(trans)
	{
		std::sort(objs.begin(),objs.end());
	}

	for(int i = 0; i < objs.size(); ++i)
	{
		GetShapePool()->PushToDraw(objs[i].m_obj->GetShapeHandle(),objs[i].m_obj->GetWorldMatrix());
	}

	if(trans)
	{
		GetShapePool()->DrawAllTransparent();
	}
	else
	{
		GetShapePool()->DrawAll();
	}
	StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
}

}

using namespace DynObjectPoolDetail;

//=====================================================================================//
//                class DynObjectPool::Impl : public DynObjectPoolImpl                 //
//=====================================================================================//
class DynObjectPool::Impl : public DynObjectPoolImpl
{
public:
};

//=====================================================================================//
//                           DynObjectPool::DynObjectPool()                            //
//=====================================================================================//
DynObjectPool::DynObjectPool()
:	m_pimpl(new Impl)
{
}

//=====================================================================================//
//               DynObjectPool::SetIterator_t DynObjectPool::BeginSets()               //
//=====================================================================================//
DynObjectPool::SetIterator_t DynObjectPool::BeginSets()
{
	return m_pimpl->m_objSets.begin();
}

//=====================================================================================//
//         DynObjectPool::ConstSetIterator_t DynObjectPool::BeginSets() const          //
//=====================================================================================//
DynObjectPool::ConstSetIterator_t DynObjectPool::BeginSets() const
{
	return m_pimpl->m_objSets.begin();
}

//=====================================================================================//
//                DynObjectPool::SetIterator_t DynObjectPool::EndSets()                //
//=====================================================================================//
DynObjectPool::SetIterator_t DynObjectPool::EndSets()
{
	return m_pimpl->m_objSets.end();
}

//=====================================================================================//
//          DynObjectPool::ConstSetIterator_t DynObjectPool::EndSets() const           //
//=====================================================================================//
DynObjectPool::ConstSetIterator_t DynObjectPool::EndSets() const
{
	return m_pimpl->m_objSets.end();
}

//=====================================================================================//
//                     DynObjectSet *DynObjectPool::GetObjectSet()                     //
//=====================================================================================//
DynObjectSet *DynObjectPool::GetObjectSet(const std::string &name)
{
	Impl::ObjectSets_t::iterator it, ite;
	boost::tie(it,ite) = std::equal_range(m_pimpl->m_objSets.begin(),m_pimpl->m_objSets.end(),name,DynObjSetCompare());
	if(it == ite) return 0;
	return it->get();
}

//=====================================================================================//
//               const DynObjectSet *DynObjectPool::GetObjectSet() const               //
//=====================================================================================//
const DynObjectSet *DynObjectPool::GetObjectSet(const std::string &name) const
{
	Impl::ObjectSets_t::iterator it, ite;
	boost::tie(it,ite) = std::equal_range(m_pimpl->m_objSets.begin(),m_pimpl->m_objSets.end(),name,DynObjSetCompare());
	if(it == ite) return 0;
	return it->get();
}

//=====================================================================================//
//                         void DynObjectPool::MakeSaveLoad()                          //
//=====================================================================================//
void DynObjectPool::MakeSaveLoad(SavSlot &s, const std::string &levname)
{
	m_pimpl->MakeSaveLoad(s,levname);
}

//=====================================================================================//
//                           DynObjectPool::~DynObjectPool()                           //
//=====================================================================================//
DynObjectPool::~DynObjectPool()
{
}

//=====================================================================================//
//                      ShapePool* DynObjectPool::GetShapePool()                       //
//=====================================================================================//
ShapePool* DynObjectPool::GetShapePool()
{
	return m_pimpl->GetShapePool();
}

//=====================================================================================//
//                            void DynObjectPool::Update()                             //
//=====================================================================================//
void DynObjectPool::Update(float Time)
{
	m_pimpl->Update(Time);
}

//=====================================================================================//
//                         float DynObjectPool::SwitchState()                          //
//=====================================================================================//
float DynObjectPool::SwitchState(const std::string& Name,float Time)
{
	return m_pimpl->SwitchState(Name,Time);
}

//=====================================================================================//
//                          void DynObjectPool::DoExplosion()                          //
//=====================================================================================//
void DynObjectPool::DoExplosion(std::string ObjName,const hit_s& hit, LogicDamageStrategy *LogicStrategy) //При взрыве уничтожаются объекты...
{
	m_pimpl->DoExplosion(ObjName,hit,LogicStrategy);
}

//=====================================================================================//
//                           bool DynObjectPool::TraceRay()                            //
//=====================================================================================//
bool DynObjectPool::TraceRay(const point3 &From, const point3 &Dir, point3 *Res,
							 point3 *Norm,std::string *Name,
							 unsigned int passThru)
							 //DynObjectPool::TRACE_TYPE trace_type)
{
	return m_pimpl->TraceRay(From,Dir,Res,Norm,Name,passThru);
}

//=====================================================================================//
//                         bool DynObjectPool::TraceSegment2()                         //
//=====================================================================================//
bool DynObjectPool::TraceSegment2(const point3 &From, const point3 &Dir, point3 *Res,
								  point3 *Norm,std::string *Name,
								  unsigned int passThru)
								  //DynObjectPool::TRACE_TYPE trace_type)
{
	return m_pimpl->TraceSegment2(From,Dir,Res,Norm,Name,passThru);
}

//=====================================================================================//
//                         bool DynObjectPool::TraceSegment()                          //
//=====================================================================================//
bool DynObjectPool::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res,
								 point3 *Norm,std::string *Name)
{
	return m_pimpl->TraceSegment(From,Dir,Res,Norm,Name);
}

//=====================================================================================//
//                             void DynObjectPool::Clear()                             //
//=====================================================================================//
void DynObjectPool::Clear()
{
	m_pimpl->Clear();
}

//=====================================================================================//
//                     void DynObjectPool::ClearObjectInfluence()                      //
//=====================================================================================//
void DynObjectPool::ClearObjectInfluence()
{
	m_pimpl->ClearObjectInfluence();
}

//=====================================================================================//
//                     void DynObjectPool::CollectHexesForObject()                     //
//=====================================================================================//
void DynObjectPool::CollectHexesForObject(const std::string &ObjName,std::vector<ipnt2_t> *vec)
{
	m_pimpl->CollectHexesForObject(ObjName,vec);
}

//=====================================================================================//
//                             void DynObjectPool::Start()                             //
//=====================================================================================//
void DynObjectPool::Start()
{
	m_pimpl->StartSounds();
}

//=====================================================================================//
//                             void DynObjectPool::Stop()                              //
//=====================================================================================//
void DynObjectPool::Stop()
{
	//// refactoring
	//Impl::ObjectsIt it = m_pimpl->m_Objects.begin(), ite = m_pimpl->m_Objects.end();
	//for(; it != ite; it++)
	//{
	//	if(it->second.Sounder) it->second.Sounder->Stop();
	//}
}

//=====================================================================================//
//                            void DynObjectPool::EndTurn()                            //
//=====================================================================================//
void DynObjectPool::EndTurn()
{
	m_pimpl->EndTurn();
}

//=====================================================================================//
//                        void DynObjectPool::SetStationInfo()                         //
//=====================================================================================//
void DynObjectPool::SetStationInfo(const std::string &Name,const Station& st)
{
	//// refactoring
	//std::pair<Impl::ObjectsIt,Impl::ObjectsIt> range = m_pimpl->m_Objects.equal_range(Name);
	//Impl::ObjectsIt it=range.first;

	//for(; it != range.second; it++)
	//{
	//	it->second.SetStationInfo(st);
	//}
}

//=====================================================================================//
//                             void DynObjectPool::Draw()                              //
//=====================================================================================//
void DynObjectPool::Draw(GraphPipe *Pipe, bool trans)
{
	m_pimpl->Draw(Pipe,trans);
}

//=====================================================================================//
//                          void DynObjectPool::EraseByName()                          //
//=====================================================================================//
void DynObjectPool::EraseByName(const std::string& Name)
{
	m_pimpl->EraseByName(Name);
}

//=====================================================================================//
//                    void DynObjectPool::CollectObjectsForPoints()                    //
//=====================================================================================//
void DynObjectPool::CollectObjectsForPoints(const std::vector<point3> &pnts, std::set<std::string> *objects)
{
	m_pimpl->CollectObjectsForPoints(pnts,objects);
}

//=====================================================================================//
//                           void DynObjectPool::LoadAlfa()                            //
//=====================================================================================//
void DynObjectPool::LoadAlfa(ALD::Manager &mgr, const std::string &levname)
{
	m_pimpl->LoadAlfa(mgr,levname);
}

//=====================================================================================//
//                      const LevelCase* DynObjectPool::GetCase()                      //
//=====================================================================================//
const LevelCase* DynObjectPool::GetCase(const std::string& Name)
{
	return 0;

	//// refactoring
	//std::pair<Impl::ObjectsIt,Impl::ObjectsIt> range = m_pimpl->m_Objects.equal_range(Name);
	//Impl::ObjectsIt it = range.first, it1 = range.second;

	//while(it != it1)
	//{
	//	return &(it->second.GetCaseInfo());
	//}

	//return NULL;
}

//=====================================================================================//
//                             void DynObjectPool::Bake()                              //
//=====================================================================================//
void DynObjectPool::Bake()
{
	m_pimpl->Bake();
}

//=====================================================================================//
//                         void DynObjectPool::OnLostDevice()                          //
//=====================================================================================//
void DynObjectPool::OnLostDevice()
{
	m_pimpl->m_Shapes.OnLostDevice();
}

//=====================================================================================//
//                         void DynObjectPool::OnResetDevice()                         //
//=====================================================================================//
void DynObjectPool::OnResetDevice()
{
	m_pimpl->m_Shapes.OnResetDevice();
}
