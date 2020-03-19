#include "precomp.h"
#include "PoLGameLevel.h"

#include "chop.h"
#include "ALDMgr.h"
#include "ProgrBar.h"
#include "LevelToLogic.h"
#include "PoLLevelUtils.h"
#include "ScatteredItems.h"
#include "../Skin/Shadow.h"
#include "DynObjectOldData.h"
#include "IntersectionCache.h"
#include "../Options/Options.h"
#include <Common/Utils/VFileWpr.h>
#include <Common/GraphPipe/Camera.h>
#include <Common/GraphPipe/GraphPipe.h>
#include <Common/TextureMgr/TextureMgr.h>
#include <Common/3DEffects/EffectManager.h>

#include <boost/format.hpp>

namespace PoLGameLevelDetail
{

//=====================================================================================//
//                             class LevelsLongShotReader                              //
//=====================================================================================//
class LevelsLongShotReader
{
	mll::utils::table m_tbl;
	mll::utils::table::col_type m_sysname;
	mll::utils::table::col_type m_envname;

public:
	LevelsLongShotReader()
	{
		PoL::VFileWpr wpr("scripts/levels.txt");
		if(wpr.get() && wpr.get()->Size())
		{
			PoL::VFileBuf buf(wpr.get());
			std::istream in(&buf);
			in >> m_tbl;

			m_sysname = m_tbl.col(0);
			m_envname = m_tbl.col(18);
		}
	}

	std::string getLongShotName(const std::string &n) const
	{
		for(int i = 0; i < m_tbl.height(); ++i)
		{
			if(m_sysname[i] == n)
			{
				return m_envname[i] + ".shader";
			}
		}

		return "";
	}
};

//=====================================================================================//
//                   const LevelsLongShotReader &getLongShotReader()                   //
//=====================================================================================//
const LevelsLongShotReader &getLongShotReader()
{
	static LevelsLongShotReader inst;
	return inst;
}

//=====================================================================================//
//                         class GrantedAPI : public LevelAPI                          //
//=====================================================================================//
class GrantedAPI : public LevelAPI
{
protected:
	static PoLGameLevel *Lev;

public://API methods:
	virtual void EndTurn()
	{
		if(Lev) Lev->EndTurn();
	}

	virtual void EnableJoint(int Num,bool Flag)
	{
		unsigned int p = HexGrid::GetInst()->GetActiveJoints();
		unsigned int mask = 1<<Num;
		if(Flag)
		{
			HexGrid::GetInst()->SetActiveJoints(p|mask);
		}
		else
		{
			HexGrid::GetInst()->SetActiveJoints(p&~mask);
		}
	};

public:
	void SetLevel(PoLGameLevel *lev) { Lev = lev; }
	virtual ~GrantedAPI() {}
	GrantedAPI(){ SetAPI(this); }
};

PoLGameLevel *GrantedAPI::Lev = NULL;
GrantedAPI LevAPI;

//=====================================================================================//
//                              struct ProgressBarHelper                               //
//=====================================================================================//
struct ProgressBarHelper
{
public:
	float ldStart, ldEnd;
	float ldGeometryStart, ldGeometryEnd;
	float ldPassStart, ldPassEnd;
	float ldObjStart, ldObjEnd;
	float ldLightMapStart, ldLightMapEnd;
	float ldEffStart, ldEffEnd;
	float ldSndStart, ldSndEnd;
	float ldEndStart, ldEndEnd;

	float getCell(const mll::utils::table &tbl, const std::string &n)
	{
		mll::utils::table::col_type hdr = tbl.col(0);
		return boost::lexical_cast<float>(tbl.row(std::find(hdr.begin(),hdr.end(),n))[1]);
	}

	ProgressBarHelper()
	{
		PoL::VFileWpr file("scripts/loadprogress.txt");
		PoL::VFileBuf buf(file.get());
		std::istream stream(&buf);

		mll::utils::table tbl;
		stream >> tbl;

		ldStart = getCell(tbl,"start");
		ldEnd = ldGeometryStart = getCell(tbl,"geometry");
		ldGeometryEnd = ldPassStart = getCell(tbl,"pass");
		ldPassEnd = ldObjStart = getCell(tbl,"obj");
		ldObjEnd = ldLightMapStart = getCell(tbl,"lightmap");
		ldLightMapEnd = ldEffStart = getCell(tbl,"effects");
		ldEffEnd = ldSndStart = getCell(tbl,"sounds");
		ldSndEnd = ldEndStart = getCell(tbl,"cache");
		ldEndEnd = getCell(tbl,"end");
	}
};

}

using namespace PoLGameLevelDetail;

namespace DirtyLinks
{
	std::string GetStrRes(const std::string& rid); 
}

bool PoLGameLevel::m_roofsVisible = false;

//=====================================================================================//
//                            PoLGameLevel::PoLGameLevel()                             //
//=====================================================================================//
PoLGameLevel::PoLGameLevel(const std::string &name)
:	m_levelName(name)
{
	LoadLevel();
}

//=====================================================================================//
//                            PoLGameLevel::PoLGameLevel()                             //
//=====================================================================================//
PoLGameLevel::PoLGameLevel(GenericStorage &st)
{
	assert( !st.IsSaving() );
	SavSlot cell(&st, "Lvdta");
	SavSlot lvlspots(&st, "LVLSPOTS");

	cell >> m_levelName;
	if(st.GetVersion() >= 8)
	{
		cell >> m_roofsVisible;
	}

	LoadLevel();

	m_dynpool.MakeSaveLoad(cell,m_levelName);
	m_marks.MakeSaveLoad(cell);

	IWorld::Get()->GetEffects()->MakeLoad(lvlspots,this);
}

//=====================================================================================//
//                           void PoLGameLevel::LoadLevel()                            //
//=====================================================================================//
void PoLGameLevel::LoadLevel()
{
	static ProgressBarHelper pbh;
	std::fill_n(m_marksEnabled,(int)MT_COUNT,true);

	const std::string levname = "maps/" + m_levelName + ".mllev";
	const std::string realname = IWorld::Get()->GetLevelDesc(m_levelName);

	PoL::VFileWpr file(levname);
	if(!file.get() || !file->Size())
	{
		std::ostringstream sstr;
		sstr << "Cannot load level [" << m_levelName << "]";
		throw CASUS(sstr.str());
	}

	PoL::VFileBuf buf(file.get());
	std::istream stream(&buf);

	using ALD::Manager;

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevel"),realname);
	ProgrBar::SetRange(pbh.ldStart,pbh.ldEnd);
	ProgrBar::SetPercent(0.0f);
	IWorld::Get()->GetPipe()->RegisterShaderFile("shaders.sys/effects.shader",false);
	ProgrBar::SetPercent(0.33f);
	IWorld::Get()->GetPipe()->RegisterShaderFile("shaders.sys/units.shader",false);
	ProgrBar::SetPercent(0.66f);
	IWorld::Get()->GetPipe()->RegisterShaderFile(getLongShotReader().getLongShotName(m_levelName),false);
	ProgrBar::SetPercent(1.0f);

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelgeom"),realname);
	ProgrBar::SetRange(pbh.ldGeometryStart,pbh.ldGeometryEnd);
	ProgrBar::SetPercent(0.0f);
	Manager manager(stream);
	Manager::folder_iterator basis_folder = manager.ifolder("general").ifolder("basis");
	assert( std::distance(basis_folder.ibegin(),basis_folder.iend()) == 1 );
	ProgrBar::SetPercent(0.1f);
	basis_folder.ibegin()->ibinstream() >> m_octTree;
	ProgrBar::SetPercent(0.3f);

	std::ostringstream sstr;
	sstr << "shaders/" << m_levelName << ".shader";
	IWorld::Get()->GetPipe()->RegisterShaderFile(sstr.str());
	ProgrBar::SetPercent(0.5f);
	LinkShaders(IWorld::Get()->GetPipe());
	ProgrBar::SetPercent(1.0f);

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelpass"),realname);
	ProgrBar::SetRange(pbh.ldPassStart,pbh.ldPassEnd);
	ProgrBar::SetPercent(0.0f);
	m_levelGrid.reset(new Grid("models/" + m_levelName + ".grid"));
	ProgrBar::SetPercent(0.9f);
	LevelAPI::GetAPI()->EnableJoint(0,true); LevelAPI::GetAPI()->EnableJoint(1,true);
	LevelAPI::GetAPI()->EnableJoint(2,true); LevelAPI::GetAPI()->EnableJoint(3,true);
	LevelAPI::GetAPI()->EnableJoint(4,true); LevelAPI::GetAPI()->EnableJoint(5,true);
	LevelAPI::GetAPI()->EnableJoint(6,true); LevelAPI::GetAPI()->EnableJoint(7,true);
	ProgrBar::SetPercent(1.0f);

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelobj"),realname);
	ProgrBar::SetRange(pbh.ldObjStart,pbh.ldObjEnd);
	ProgrBar::SetPercent(0.0f);
	LoadObjectsGeometry(manager);
	LoadCameras();
	ProgrBar::SetPercent(1.0f);

	if(Options::GetInt("system.video.lightmaps"))
	{
		ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevellght"),realname);
		ProgrBar::SetRange(pbh.ldLightMapStart,pbh.ldLightMapEnd);
		ProgrBar::SetPercent(0.0f);
		m_lmName = ":lm";
		Manager::folder_iterator lm_folder = manager.ifolder("general").ifolder("lightmap");
		assert( std::distance(lm_folder.ibegin(),lm_folder.iend()) == 1 );
		CreateLightMap(lm_folder.ibegin()->ibinstream().stream());
		ProgrBar::SetPercent(1.0f);
	}

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadleveleff"),realname);
	ProgrBar::SetRange(pbh.ldEndStart,pbh.ldEndEnd);
	ProgrBar::SetPercent(0.0f);
	LoadEffects(manager);
	ProgrBar::SetPercent(1.0f);

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelsound"),realname);
	ProgrBar::SetRange(pbh.ldSndStart,pbh.ldSndEnd);
	ProgrBar::SetPercent(0.0f);
	LoadSounds(manager);
	ProgrBar::SetPercent(1.0f);

	ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelend"),realname);
	ProgrBar::SetRange(pbh.ldEndStart,pbh.ldEndEnd);
	ProgrBar::SetPercent(0.0f);
	m_octTreeDrawer.init(m_octTree.get_storage(), m_octTree.get_triangles(), m_octTree.get_materials());
	ProgrBar::SetPercent(0.4f);
	m_octTreeDrawer.updateMaterials();
	CalcNormals();
	ProgrBar::SetPercent(1.0f);

	m_octTree.set_visible_floor(m_roofsVisible?2:1);
}

//=====================================================================================//
//                            PoLGameLevel::~PoLGameLevel()                            //
//=====================================================================================//
PoLGameLevel::~PoLGameLevel()
{
	LevAPI.SetLevel(0);
	Stop();

	m_levelGrid.reset();
	m_dynpool.Clear();
	m_marks.clear();

	if(!m_lmName.empty())
	{
		TextureMgr::Instance()->Release(m_lmName.c_str());
	}

	for(Effects_t::iterator ei = m_levelEffects.begin(); ei != m_levelEffects.end(); ++ei)
	{
		IWorld::Get()->GetEffects()->DestroyEffect(ei->id);
	}

	m_levelEffects.clear();
	m_levelSounds.clear();
	ScatteredItem::Clear();

	std::ostringstream sstr;
	sstr << "shaders/" << m_levelName << ".shader";
	IWorld::Get()->GetPipe()->UnloadShaderFile(sstr.str());

	IWorld::Get()->GetPipe()->UnloadShaderFile("shaders.sys/effects.shader");
	IWorld::Get()->GetPipe()->UnloadShaderFile("shaders.sys/units.shader");
	IWorld::Get()->GetPipe()->UnloadShaderFile(getLongShotReader().getLongShotName(m_levelName));
}

//=====================================================================================//
//                              void PoLGameLevel::Stop()                              //
//=====================================================================================//
void PoLGameLevel::Stop()
{
	for(Effects_t::iterator ei = m_levelEffects.begin(); ei != m_levelEffects.end(); ++ei)
	{
		IWorld::Get()->GetEffects()->DestroyEffect(ei->id);
	}

	for(Sounds_t::iterator si = m_levelSounds.begin(); si != m_levelSounds.end(); ++si)
	{
		if(si->Type & NamedSound::NS_CYCLED)
		{
			if(si->m_Emitter2->get())
			{
				(*si->m_Emitter2)->stop();
				(*si->m_Emitter2).reset();
			}
		}
	}

	m_dynpool.Stop();
}

//=====================================================================================//
//                          void PoLGameLevel::CalcNormals()                           //
//=====================================================================================//
void PoLGameLevel::CalcNormals()
{
	using mll::algebra::vector3;
	using mll::algebra::point3;

	m_normals.resize(m_octTree.get_triangles().size());
	m_triDesc.resize(m_octTree.get_triangles().size());

	const oct_tree::storage &verts = m_octTree.get_storage();
	const oct_tree::PolyList &trias = m_octTree.get_triangles();
	const oct_tree::MatList &mats = m_octTree.get_materials();

	for(size_t i = 0; i < trias.size(); ++i)
	{
		const point3 &pt1 = verts[trias[i].v[0]].pos;
		const point3 &pt2 = verts[trias[i].v[1]].pos;
		const point3 &pt3 = verts[trias[i].v[2]].pos;
		vector3 normal = (pt1 - pt2).cross(pt3 - pt2).normalize();
		if(normal.dot(verts[trias[i].v[0]].norm) < 0.0f) normal *= -1.0f;
		m_normals[i] = normal;

		VShader *shader = IWorld::Get()->GetPipe()->FindShader(mats[trias[i].mat_id]);
		m_triDesc[i] = 0;

		if(shader->Transparent)
		{
			m_triDesc[i] |= mdTransparent;
		}
		if(shader->GetTraits().surfaceParam & SHD_NONSOLID)
		{
			m_triDesc[i] |= mdGhost;
		}
		if(shader->GetTraits().surfaceParam & SHD_TRANS)
		{
			m_triDesc[i] |= mdCursor;
		}
		if(trias[i].floor > 1)
		{
			m_triDesc[i] |= mdRoof;
		}
	}

	m_traceContext.m_tree = &m_octTree;
	m_traceContext.m_normals = &m_normals[0];
	m_traceContext.m_triDesc = &m_triDesc[0];

	m_isectCache.reset(new PoL::IntersectionCache(&m_octTree,m_traceContext.m_triDesc));
}

//=====================================================================================//
//                              void PoLGameLevel::Save()                              //
//=====================================================================================//
void PoLGameLevel::Save(GenericStorage &st)
{
	assert( st.IsSaving() );

	SavSlot cell(&st, "Lvdta");
	SavSlot lvlspots(&st, "LVLSPOTS");

	cell << m_levelName;
	cell << m_roofsVisible;
	m_dynpool.MakeSaveLoad(cell, m_levelName);
	m_marks.MakeSaveLoad(cell);

	IWorld::Get()->GetEffects()->MakeSave(lvlspots);
}

//=====================================================================================//
//                          void PoLGameLevel::LoadCameras()                           //
//=====================================================================================//
void PoLGameLevel::LoadCameras()
{
	std::string name = "models/" + m_levelName + ".cameras";
	PoL::VFileWpr file(name);

	if(file.get() && file.get()->Size())
	{
		PoL::VFileBuf buf(file.get());
		std::istream stream(&buf);
		m_camPaths.clear();

		int camnum;
		(stream.ignore(strlen("Cameras Number:"))>>camnum).ignore(strlen("\n"));

		while(camnum--)
		{
			std::string CamName;
			KeyAnimation ka;
			(stream>>CamName).ignore(strlen("\n"));
			std::transform(CamName.begin(),CamName.end(),CamName.begin(),std::tolower);
			ka.Load(stream);
			m_camPaths[CamName] = ka;
		}
	}
}

//=====================================================================================//
//                          void PoLGameLevel::LoadEffects()                           //
//=====================================================================================//
void PoLGameLevel::LoadEffects(ALD::Manager &manager)
{
	Shadows::ClearLights();

	using ALD::Manager;
	Manager::folder_iterator floors_folder = manager.ifolder("general").ifolder("floors");
	
	for(Manager::folder_iterator i = floors_folder.fbegin(); i != floors_folder.fend(); ++i)
	{
		Manager::folder_iterator effect_folder = i.ifolder("effect");
		for(Manager::item_iterator j = effect_folder.ibegin(); j != effect_folder.iend(); ++j)
		{
			mll::algebra::matrix3 mtr;
			mll::utils::named_vars vars;
			j->ibinstream() >> vars >> mtr;

			if(vars.exists("light"))
			{
				float power = vars.get_default("light_power",255.0f);
				float dist = vars.get_default("light_dist",100.0f);
				Shadows::AddLight(::point3(mll::algebra::point3(0.5f,0.5f,0.5f)*mtr),power,dist);
			}
			else
			{
				NamedEffect eff;
				eff.LoadAlfa(vars,mtr);

				m_levelEffects.push_back(eff);
			}
		}
	}
}

//=====================================================================================//
//                           void PoLGameLevel::LoadSounds()                           //
//=====================================================================================//
void PoLGameLevel::LoadSounds(ALD::Manager &manager)
{
	using ALD::Manager;
	Manager::folder_iterator floors_folder = manager.ifolder("general").ifolder("floors");
	
	for(Manager::folder_iterator i = floors_folder.fbegin(); i != floors_folder.fend(); ++i)
	{
		Manager::folder_iterator effect_folder = i.ifolder("sound");
		for(Manager::item_iterator j = effect_folder.ibegin(); j != effect_folder.iend(); ++j)
		{
			mll::algebra::matrix3 mtr;
			mll::utils::named_vars vars;
			j->ibinstream() >> vars >> mtr;

			SoundUtter snd;
			snd.LoadAlfa(vars,mtr);
			m_levelSounds.push_back(snd);
		}
	}
}

//=====================================================================================//
//                      void PoLGameLevel::LoadObjectsGeometry()                       //
//=====================================================================================//
void PoLGameLevel::LoadObjectsGeometry(ALD::Manager &manager)
{
	GetLevelObjects()->LoadAlfa(manager, m_levelName);
}

//=====================================================================================//
//                        void PoLGameLevel::CreateLightMaps()                         //
//=====================================================================================//
void PoLGameLevel::CreateLightMap(std::istream &in)
{
	static int counter = 0;
	int x,y,bpp,size;

	mll::io::ibinstream bin(in);
	bin >>  x >> y >> bpp >> size;
	assert( x && y && bpp && size );

	DIBData dta;
	dta.Create(x,y,bpp);
	bin.stream().read(reinterpret_cast<char *>(dta.Image()),size);
	TextureMgr::Instance()->CreateTexture(m_lmName.c_str(),&dta,0,0);
	dta.Release();
}

//=====================================================================================//
//                          void PoLGameLevel::EnableMarks()                           //
//=====================================================================================//
void PoLGameLevel::EnableMarks(MARK_TYPE type, bool enable)
{
	m_marksEnabled[type] = enable;
}

//=====================================================================================//
//                          bool PoLGameLevel::MarksEnabled()                          //
//=====================================================================================//
bool PoLGameLevel::MarksEnabled(MARK_TYPE type)
{
	return m_marksEnabled[type];
}

//=====================================================================================//
//                         void PoLGameLevel::UpdateObjects()                          //
//=====================================================================================//
void PoLGameLevel::UpdateObjects(float tau)
{
	m_dynpool.Update(tau);

	Sounds_t::iterator si = m_levelSounds.begin();
	while(si != m_levelSounds.end())
	{
		if(si->Type&NamedSound::NS_RANDOM)
		{
			float ltime = si->LastTimePlayed;
			float interval = tau-ltime;
			float maxinterval = si->Freq;
			if(sqrt(frand())*maxinterval<interval)
			{//пора проиграть звук
				std::string a((si->Type&NamedSound::NS_STATIC)?"lodinamic2d":"lodinamic");
				std::string b(std::string("sounds/")+si->GetNextName()+".wav");
				si->LastTimePlayed=si->LastTimePlayed+si->Freq;
				Muffle::HScript snd_script = Muffle::ISound::instance()->getScript(a.c_str());
				Muffle::HEmitter emitter = Muffle::ISound::instance()->createEmitter(snd_script,b.c_str());
				emitter->setPosition(si->Pos.as_mll_vector());
				emitter->play();
			}
		}
		si++;
	}
}

//=====================================================================================//
//                             void PoLGameLevel::Start()                              //
//=====================================================================================//
void PoLGameLevel::Start()
{
	for(Sounds_t::iterator si = m_levelSounds.begin(); si != m_levelSounds.end(); ++si)
	{
		if(si->Type & NamedSound::NS_CYCLED)
		{
			bool Static = (si->Type&NamedSound::NS_STATIC)?true:false;
			std::ostringstream sstr;
			sstr << "sounds/" << si->GetNextName() << ".wav";

			const char *a = Static ? "lodinamiccycled2d" : "lodinamiccycled";
			Muffle::HScript snd_script = Muffle::ISound::instance()->getScript(a);
			*si->m_Emitter2 = Muffle::ISound::instance()->createEmitter(snd_script,sstr.str().c_str());
			if(!Static) (*si->m_Emitter2)->setPosition(si->GetPos().as_mll_vector());
			(*si->m_Emitter2)->play();
		}
		si->LastTimePlayed = Timer::GetSeconds()+2*frand()*si->Freq;
	}

	for(Effects_t::iterator ei = m_levelEffects.begin(); ei != m_levelEffects.end(); ++ei)
	{
		ei->id = IWorld::Get()->GetEffects()->CreateAmbientEffect(ei->Name,
			ei->Position,
 			ei->Color/255.0f);
	}

	m_dynpool.Start();
}

//=====================================================================================//
//                            void PoLGameLevel::AddMark()                             //
//=====================================================================================//
void PoLGameLevel::AddMark(boost::shared_ptr<BaseMark> mark)
{
	mark->prim.Verts.clear();
	if(m_marksEnabled[MT_STATIC]) CollectPlanes(mark->Pos,mark->Rad,*mark);
	m_marks.Add(mark);
}

//=====================================================================================//
//                             void PoLGameLevel::DoMark()                             //
//=====================================================================================//
void PoLGameLevel::DoMark(const point3 &where, const float rad, const std::string& shader)
{
	boost::shared_ptr<Mark> d(new Mark(where, rad, shader));
	CollectPlanes(d->Pos,rad,*d);
	m_marks.Add(d);
}

//=====================================================================================//
//                          void PoLGameLevel::UpdateMarks()                           //
//=====================================================================================//
void PoLGameLevel::UpdateMarks(float Time)
{
	m_marks.Update(Time);
}

//=====================================================================================//
//                          void PoLGameLevel::LinkShaders()                           //
//=====================================================================================//
void PoLGameLevel::LinkShaders(GraphPipe *Pipe)
{
}

//=====================================================================================//
//                           void PoLGameLevel::DrawMarks()                            //
//=====================================================================================//
void PoLGameLevel::DrawMarks()
{
	m_marks.Draw();
}

//=====================================================================================//
//                           bool PoLGameLevel::TraceRay2()                            //
//=====================================================================================//
bool PoLGameLevel::TraceRay3(const ray &r, unsigned int passThru, TraceResult *tr)
{
	using mll::geometry::ray3;
	ray3 pcr(r.Origin.as_mll_point(),r.Direction.as_mll_vector());

	return PoLLevelUtils::TraceRay2(m_isectCache.get()/*m_traceContext*/,pcr,passThru,tr);
}

//=====================================================================================//
//                         bool PoLGameLevel::TraceSegment2()                          //
//=====================================================================================//
bool PoLGameLevel::TraceSegment3(const ray &r, unsigned int passThru, TraceResult *tr)
{
	using mll::geometry::ray3;
	ray3 pcr(r.Origin.as_mll_point(),r.Direction.as_mll_vector());

	const bool result = PoLLevelUtils::TraceSegment2(m_isectCache.get()/*m_traceContext*/,pcr,passThru,tr);
	return result;
}

//=====================================================================================//
//                             static bool IsFirstPerson()                             //
//=====================================================================================//
static bool IsFirstPerson()
{
	return IWorld::Get()->GetPipe()->GetCam()->GetMoveType() == cmtPersonBone;
}

//=====================================================================================//
//                             void PoLGameLevel::Update()                             //
//=====================================================================================//
void PoLGameLevel::Update(GraphPipe *pipe)
{
	using mll::algebra::vector3;
	const CameraParams &params = pipe->GetCam()->GetParams();
	mll::geometry::frustum frustum(params.m_near,params.m_far,params.m_fov,params.m_aspect);
	const vector3 dir(params.m_dir);
	const vector3 right(params.m_up.cross(dir));
	const vector3 up(dir.cross(right));
	mll::algebra::matrix3 mtr = mll::algebra::view(dir,up,params.m_pos);
	mtr.invert();
	frustum.set_pos(mtr);

	m_octTree.set_visible_floor(m_roofsVisible || IsFirstPerson() ? 2 : 1);

	m_octTreeDrawer.resetVisible();
	m_octTree.update(frustum,&m_octTreeDrawer);
}

//=====================================================================================//
//                              void PoLGameLevel::Draw()                              //
//=====================================================================================//
void PoLGameLevel::Draw(GraphPipe *pipe, bool transp)
{
	const unsigned int andfl = transp ? 0 : oct_tree_drawer::mtOpaque;
	const unsigned int notfl = transp ? oct_tree_drawer::mtOpaque : 0;

	Frustum *fru = &pipe->GetCam()->Cone;
	const Camera* cam = pipe->GetCam();
	m_env.Update(cam);

	m_octTreeDrawer.draw(andfl,notfl);
}

//=====================================================================================//
//                         void PoLGameLevel::CollectPlanes()                          //
//=====================================================================================//
void PoLGameLevel::CollectPlanes(point3 &pos, float rad, BaseMark &m, int index)
{
	PoLLevelUtils::CollectTrianglesInSphere(&m_octTree,m.prim,m.Col,mll::algebra::point3(pos.x,pos.y,pos.z),rad);
}

//=====================================================================================//
//                            void CollectPlanesForShadow()                            //
//=====================================================================================//
void PoLGameLevel::CollectPlanesForShadow(Shadows::BaseShadow *shd, Primi *prim, float rad)
{
	PoLLevelUtils::CollectTrianglesForShadow(&m_octTree,*shd,*prim,rad);
}

//=====================================================================================//
//                      bool PoLGameLevel::IsRoofVisible() const                       //
//=====================================================================================//
bool PoLGameLevel::IsRoofVisible() const
{
	return m_roofsVisible;
}

//=====================================================================================//
//                         void PoLGameLevel::SetRoofVisible()                         //
//=====================================================================================//
void PoLGameLevel::SetRoofVisible(bool v)
{
	m_roofsVisible = v;
}

//=====================================================================================//
//               const KeyAnimation *PoLGameLevel::GetCameraPath() const               //
//=====================================================================================//
const KeyAnimation *PoLGameLevel::GetCameraPath(const std::string &path) const
{
	CameraPaths_t::const_iterator i = m_camPaths.find(path);
	if(i == m_camPaths.end()) return 0;
	return &i->second;
}

//=====================================================================================//
//                const std::string &PoLGameLevel::GetLevelName() const                //
//=====================================================================================//
const std::string &PoLGameLevel::GetLevelName() const
{
	return m_levelName;
}

//=====================================================================================//
//                            Grid *PoLGameLevel::GetGrid()                            //
//=====================================================================================//
Grid *PoLGameLevel::GetGrid()
{
	return m_levelGrid.get();
}

//=====================================================================================//
//                         BusPath *PoLGameLevel::GetBusPath()                         //
//=====================================================================================//
BusPath *PoLGameLevel::GetBusPath(const std::string &)
{
	return 0;
}

//=====================================================================================//
//                      DestPoints *PoLGameLevel::GetDestPoints()                      //
//=====================================================================================//
DestPoints *PoLGameLevel::GetDestPoints(const std::string &)
{
	return 0;
}

//=====================================================================================//
//                        LongShot *PoLGameLevel::GetLongShot()                        //
//=====================================================================================//
LongShot *PoLGameLevel::GetLongShot()
{
	return &m_env;
}

//=====================================================================================//
//                     void PoLGameLevel::OnStartChangeVideoMode()                     //
//=====================================================================================//
void PoLGameLevel::OnStartChangeVideoMode()
{
	m_octTreeDrawer.reset();
	m_dynpool.OnLostDevice();
}

//=====================================================================================//
//                    void PoLGameLevel::OnFinishChangeVideoMode()                     //
//=====================================================================================//
void PoLGameLevel::OnFinishChangeVideoMode()
{
	if(Options::GetInt("system.video.lightmaps"))
	{
		using ALD::Manager;
		const std::string levname = "maps/" + m_levelName + ".mllev";
		PoL::VFileWpr file(levname);
		PoL::VFileBuf buf(file.get());
		std::istream stream(&buf);
		Manager manager(stream);

		m_lmName = ":lm";
		Manager::folder_iterator lm_folder = manager.ifolder("general").ifolder("lightmap");
		assert( std::distance(lm_folder.ibegin(),lm_folder.iend()) == 1 );
		CreateLightMap(lm_folder.ibegin()->ibinstream().stream());
	}
	m_octTreeDrawer.init(m_octTree.get_storage(), m_octTree.get_triangles(), m_octTree.get_materials());
	m_octTreeDrawer.updateMaterials();
	m_dynpool.OnResetDevice();
}
