#include "precomp.h"

#include "../Game.h"
#include <d3dxcore.h>
#include "../iworld.h"
#include "ComplexShadow.h"
#include "../gamelevel/grid.h"
#include "../Gamelevel/Gamelevel.h"
#include <common/GraphPipe/culling.h>
#include <common/graphpipe/vshader.h>
#include <common/TextureMgr/DIBData.h>
#include <common/GraphPipe/statesmanager.h>
#include <undercover/options/options.h>

#define gvzShadowZero	0x00000000
#define gvzShadowOne	0xAF000000

namespace Shadows
{

bool g_useHT = true;

//=====================================================================================//
//                                    struct tline                                     //
//=====================================================================================//
struct tline
{
	int		bi, li;					// индексы точек
	int		ti[sc_line::MAXNBRS];	// индексы соседних точек
	int		ic;						// кол-во ^ индексов
};

//=====================================================================================//
//                              static void fillTexture()                              //
//=====================================================================================//
static void fillTexture(DIBData *tex)
{
	long *pixptr = (long *)tex->Image();

	for(int i = 0; i < tex->Width()*tex->Height(); ++i)
	{
		pixptr[i] = 0xFFFF0000;
	}

	for(int i = 0; i < tex->Height(); ++i)
	{
		pixptr[tex->Width() * i + i] = 0xFF00FF00;
		pixptr[tex->Width() * i + (tex->Height()-i-1)] = 0xFF00FF00;
	}
}

//=====================================================================================//
//                           ComplexShadow::ComplexShadow()                            //
//=====================================================================================//
ComplexShadow::ComplexShadow(RealEntity *owner)
:	BaseShadow(owner),
	m_lockData(0)
{
	m_shader = "person_complex_shadow";
	if (Options::GetInt("system.video.shadowquality") > 8) throw CASUS("Слишком высокое качество теней\nСтавьте 8 или меньше");

	TEXTX = 1 << Options::GetInt("system.video.shadowquality");
	TEXTY = 1 << Options::GetInt("system.video.shadowquality");
	TXS = Options::GetInt("system.video.shadowquality");

	bLinez = false;

	buf = new short int[TEXTX*TEXTY];
	memset(buf,0,sizeof(short)*TEXTX*TEXTY);

	// Создадим DIBData
	dbdTexture = new DIBData;
	dbdTexture->Create(TEXTX, TEXTY, 32);
	fillTexture(dbdTexture);

	shdwSurface = TextureMgr::Instance()->CreateTexture(txtName.c_str(), dbdTexture, -2, 0);

	NeedUpdate = true;

	GVZ_LOG("Created %s\n", txtName.c_str ());
}

//=====================================================================================//
//                           ComplexShadow::ComplexShadow()                            //
//=====================================================================================//
ComplexShadow::ComplexShadow(BaseShadow *elder)
:	BaseShadow(elder),
	m_lockData(0)
{
	m_shader = "person_complex_shadow";

	if(Options::GetInt("system.video.shadowquality") > 8) throw CASUS("Слишком высокое качество теней\nСтавьте 8 или меньше");

	TEXTX = 1 << Options::GetInt("system.video.shadowquality");
	TEXTY = 1 << Options::GetInt("system.video.shadowquality");
	TXS = Options::GetInt("system.video.shadowquality");

	bLinez = false;

	buf = new short int[TEXTX*TEXTY];
	memset(buf,0,sizeof(short)*TEXTX*TEXTY);

	// Создадим DIBData
	dbdTexture = new DIBData;
	dbdTexture->Create(TEXTX, TEXTY, 32);
	fillTexture(dbdTexture);

	TextureMgr::Instance()->Release(txtName.c_str());
	shdwSurface = TextureMgr::Instance()->CreateTexture(txtName.c_str(), dbdTexture, -2, 0);

	for(std::list<SimpleTexturedObject *>::iterator i = elder->objects.begin(); i != elder->objects.end(); ++i)
	{
		AddObject(*i);
	}

	NeedUpdate = true;

	GVZ_LOG("Created %s\n", txtName.c_str ());
}

//=====================================================================================//
//                           ComplexShadow::~ComplexShadow()                           //
//=====================================================================================//
ComplexShadow::~ComplexShadow()
{
	if(buf) delete [] buf;
	if(dbdTexture) delete dbdTexture;

	if(!ShadowUtility::NoKill()) TextureMgr::Instance()->Release(txtName.c_str());

	GVZ_LOG("Destroyed %s\n", txtName.c_str());
}

//=====================================================================================//
//                      bool ComplexShadow::PoLNeedUpdate() const                      //
//=====================================================================================//
bool ComplexShadow::PoLNeedUpdate(float t)
{
	if(!entity->Visible || !entity->Enabled || !Game::RenderEnabled()) return false;
	return ((t - tLastCalc > tRedrawPeriod*GetKoef(t)) || !Updated || (t<0.0f) || NeedUpdate);
}

//=====================================================================================//
//                          void ComplexShadow::LockSurface()                          //
//=====================================================================================//
void ComplexShadow::LockSurface()
{
	if(shdwSurface)
	{
		DDSURFACEDESC2 ddsd;
		ZeroMemory(&ddsd,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		HRESULT hres = shdwSurface->Lock(NULL,&ddsd,DDLOCK_WRITEONLY,NULL);

#ifdef _HOME_VERSION
		if(FAILED(hres))
		{
			char errstr[4096];
			D3DXGetErrorString(hres,4096,errstr);
			MLL_MAKE_LOG("shddbg.log",entity->GetSysName() << ": " << errstr << "\n");
		}
#endif

		m_lockData = (unsigned char *)ddsd.lpSurface;
		m_lockPitch = ddsd.lPitch;
	}
}

//=====================================================================================//
//                         void ComplexShadow::UnlockSurface()                         //
//=====================================================================================//
void ComplexShadow::UnlockSurface()
{
	if(shdwSurface && m_lockData)
	{
		shdwSurface->Unlock(NULL);
		m_lockData = 0;
	}
}

//=====================================================================================//
//                            void ComplexShadow::Update()                             //
//=====================================================================================//
void ComplexShadow::Update(float t)
{
	if(PoLNeedUpdate(t))
	{
		tLastCalc = t;
		Grid *grid = IWorld::Get()->GetLevel()->GetGrid();

		if(grid)
		{
			SetLight(
				ShadowUtility::getLightDirection(
					point3(
						entity->GetLocation().x,
						entity->GetLocation().y,
						grid->Height(entity->GetLocation())
					)
				)
			);
		}

		// Обновить тень
		sc_line *cursor = &m_lines[0];

		for(int j = 0; j < m_lines.size(); ++j, ++cursor) cursor->ct = 0;

		Calc();
		BlitLinez();
		if(m_lockData)
		{
			BlitBuffer(m_lockData,m_lockPitch);
		}

		Updated = true;
		NeedUpdate = false;

		cache.CollectPlanes(this,10.0f);
		Updated = false;

		m_dirty = false;
	}
}

//=====================================================================================//
//                             void ComplexShadow::Draw()                              //
//=====================================================================================//
void ComplexShadow::Draw(GraphPipe *Pipe)
{
	if(!entity->Visible || !entity->Enabled) return;
	if(m_nodes.empty()) return;

	//if(g_useHT) m_sync.wait();

	if(NeedUpdate || m_dirty)
	{
		LockSurface();
		Update(-1.0f);
		UnlockSurface();
	}

	if(!bindPoint) return;

	StatesManager::SetTexture(0,shdwSurface);

	//if(Updated)
	//{
	//	cache.CollectPlanes(this,10.0f); Updated = false;
	//}

	cache.Draw(Pipe, m_shader);
}	

//=====================================================================================//
//                                   void PushLine()                                   //
//=====================================================================================//
void PushLine(tline *lns, int &count, int idx1, int idx2, int idx3)
{
	int i;
	bool flag = false;
	int c = 0;

	for (i = 0; (i < count) && (!flag); i++)
	{
		if (((lns[i].bi == idx1) && (lns[i].li == idx2)) ||
			((lns[i].li == idx1) && (lns[i].bi == idx2))) 
		{
			flag = true;
			c = i;
		}
	}

	if (flag) 
	{
		if (lns[c].ic < sc_line::MAXNBRS)
		{
			lns[c].ti[lns[c].ic++] = idx3;
		}/* else {
			throw CASUS("Слишком много граничащих треугольников");
		}*/

		return;
	}

	lns[count].bi = idx1;
	lns[count].li = idx2;
	lns[count].ic = 1;
	lns[count].ti[0] = idx3;

	count += 1;

	return;
}

//=====================================================================================//
//                           void ComplexShadow::AddObject()                           //
//=====================================================================================//
void ComplexShadow::AddObject(const TexObject *o)
{
	for (int i = 0; i < o->PartNum; ++i) AddObject(o->Parts[i]);
}

//=====================================================================================//
//                           void ComplexShadow::AddObject()                           //
//=====================================================================================//
void ComplexShadow::AddObject(SimpleTexturedObject *so)
{
	BaseShadow::AddObject(so);

  if (so->prim.Verts.empty()) return;

	int i, j;
	sc_node *tndb=0, *tnodez=0;
	sc_line *tlnb=0; 
	tline *tlinez=0;
	int nodezCount = 0;
	int linezCount = 0;
	int nodezIdxs = 0;
	int cnIndex = 0;

	nodezIdxs = nodezCount;
	nodezCount += so->prim.Verts.size();
	linezCount += so->prim.Idxs.size();

	tnodez = new sc_node [nodezCount];
	tlinez = new tline [linezCount];

	linezCount = 0;

	for (j = 0; j < so->prim.Verts.size(); j++, cnIndex++)
	{
		tnodez[cnIndex].ptr = &so->prim.Verts[j].pos;
	}

	for (j = 0; j < so->prim.Idxs.size(); j += 3)
	{
		PushLine (tlinez, linezCount, so->GetIndexesRO()[ j ] + m_nodes.size(),
										so->GetIndexesRO()[j+1] + m_nodes.size(),
										so->GetIndexesRO()[j+2] + m_nodes.size());
		PushLine (tlinez, linezCount, so->GetIndexesRO()[j+1] + m_nodes.size(),
										so->GetIndexesRO()[j+2] + m_nodes.size(),
										so->GetIndexesRO()[ j ] + m_nodes.size());
		PushLine (tlinez, linezCount, so->GetIndexesRO()[j+2] + m_nodes.size(),
										so->GetIndexesRO()[ j ] + m_nodes.size(),
										so->GetIndexesRO()[j+1] + m_nodes.size());
	}

// optimization phase
	m_nodes.insert(m_nodes.end(),tnodez,tnodez+nodezCount);

	for (i = 0; i < m_lines.size(); i++)
	{
		m_lines[i].bi = &m_nodes[m_lines[i].bIdx];
		m_lines[i].li = &m_nodes[m_lines[i].lIdx];

		for (int j = 0; j < m_lines[i].nbrCount; ++j)
		{
			m_lines[i].ti[j] = &m_nodes[m_lines[i].tIdx[j]];
		}
	}

	unsigned int prevSize = m_lines.size();
	m_lines.resize(linezCount + m_lines.size());
	for (i = 0; i < linezCount; i++)
	{
		m_lines[prevSize+i].bIdx = tlinez[i].bi;
		m_lines[prevSize+i].lIdx = tlinez[i].li;

		m_lines[prevSize+i].bi = &m_nodes[tlinez[i].bi];
		m_lines[prevSize+i].li = &m_nodes[tlinez[i].li];

		for (int j = 0; j < tlinez[i].ic; ++j)
		{
			m_lines[prevSize+i].tIdx[j] = tlinez[i].ti[j];
			m_lines[prevSize+i].ti[j] = &m_nodes[tlinez[i].ti[j]];
		}

		m_lines[prevSize+i].nbrCount = tlinez[i].ic;
	}

	if (tndb) delete [] tndb;
	if (tnodez) delete [] tnodez;
	if (tlnb) delete [] tlnb;
	if (tlinez) delete [] tlinez;
}

//=====================================================================================//
//                             void ComplexShadow::Clear()                             //
//=====================================================================================//
void ComplexShadow::Clear()
{
	BaseShadow::Clear();

	GVZ_LOG("Deleted all objects in %s...\n", txtName.c_str());

	m_lines.swap(Lines_t());
	m_nodes.swap(Nodes_t());
}

//=====================================================================================//
//                      void ComplexShadow::UpdateOnModeChange()                       //
//=====================================================================================//
void ComplexShadow::UpdateOnModeChange()
{
	shdwSurface = TextureMgr::Instance()->CreateTexture (txtName.c_str(), dbdTexture, 0, 0);
//	shdwSurface = TextureMgr::Instance()->Texture (txtName.c_str());
	Updated = false;

	GVZ_LOG("    %s\n", txtName.c_str());
}

//=====================================================================================//
//                             void ComplexShadow::Calc()                              //
//=====================================================================================//
void ComplexShadow::Calc()
{
	int /*pcount=0,pc=0,*/i;
	float scalex, scaley;
	float x, y, z;
	sc_node *cnode;
	float ltx, lty, ltz;

	if(m_nodes.empty()) return;

	ltx = lt.x;
	lty = lt.y;
	ltz = lt.z;

	x = m_nodes[0].ptr->x;
	y = m_nodes[0].ptr->y;
	z = m_nodes[0].ptr->z;

	D3DMATRIX m;

	if (parentWorld) m = (*parentWorld)*axis; else m = axis;

	m_nodes[0].x = m._11*x + m._21*y + m._31*z + m._41;
	m_nodes[0].y = m._12*x + m._22*y + m._32*z + m._42;
	z            = m._13*x + m._23*y + m._33*z + m._43;

	minx = maxx = m_nodes[0].x;
	miny = maxy = m_nodes[0].y;
	bindPoint = m_nodes[0].ptr;
	d = z;

	cnode = &m_nodes[1];

	for (i = 1; i < m_nodes.size(); ++i, ++cnode)
	{
		x = cnode->ptr->x;
		y = cnode->ptr->y;
		z = cnode->ptr->z;

		cnode->x = m._11*x + m._21*y + m._31*z + m._41;
		cnode->y = m._12*x + m._22*y + m._32*z + m._42;
		z        = m._13*x + m._23*y + m._33*z + m._43;

		minx = std::min(minx,cnode->x);
		maxx = std::max(maxx,cnode->x);
		miny = std::min(miny,cnode->y);
		maxy = std::max(maxy,cnode->y);

		if (d > z)
		{
			bindPoint = cnode->ptr;
			d = z;
		}
	}

	scalex = (TEXTX-1)/(maxx-minx);
	scaley = (TEXTY-1)/(maxy-miny);

// Переведем координаты на плоскости в текстурные координаты

	float _miny = miny, _minx = minx;
	const sc_node * const node_ptr = &m_nodes[0];
	const unsigned int node_count = m_nodes.size();

	//for(int i = 0; i < node_count; ++i)
	//{
	//	m_nodes[i].xi = scalex*(m_nodes[i].x-_minx);
	//	m_nodes[i].yi = scaley*(m_nodes[i].y-_miny);
	//}

	long bnd = TEXTY-1;
	float temp = 0.0f;

	_asm {
			mov		edi, dword ptr node_ptr
			mov		ecx, dword ptr node_count
			mov		eax, bnd

			fild	dword ptr bnd
			fld		dword ptr scaley
			fld		dword ptr _miny
			fld		dword ptr scalex
			fld		dword ptr _minx

cvtloop:	fld		dword ptr [edi]sc_node.y
			fsub	st(0),st(3)
			fmul	st(0),st(4)
			fchs
			fadd	st(0),st(5)
			fld		dword ptr [edi]sc_node.x
			fsub	st(0),st(2)
			fmul	st(0),st(3)
			fistp	dword ptr [edi]sc_node.xi
			fistp	dword ptr [edi]sc_node.yi

			add		edi, SIZE sc_node
			loop	cvtloop

			fcompp
			fcomp
			fcompp
	}

	for(Lines_t::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		int dx1 = i->bi->yi - i->li->yi;
		if(!dx1) continue;
		int dy1 = i->li->xi - i->bi->xi;

		if(dx1 < 0)
		{
			dx1 = -dx1;
			dy1 = -dy1;
		}

		for (int j = 0; j < i->nbrCount; ++j)
		{
			const int dx2 = i->ti[j]->xi - i->bi->xi;
			const int dy2 = i->ti[j]->yi - i->bi->yi;
            const int sc1 = dx1*dx2 + dy1*dy2;

			if (sc1 == 0) continue;
			if (sc1 <= 0) i->ct += 1; else i->ct -= 1;
		}
	}

	bLinez = true;

}

//=====================================================================================//
//                          void ComplexShadow::BlitBuffer()                           //
//=====================================================================================//
void ComplexShadow::BlitBuffer(unsigned char *bmp, long pitch)
{
	unsigned long *out = (unsigned long *)bmp;
	unsigned long *marker = out;
	int ulpitch = pitch >> 2;
	short *scur = buf;
	unsigned long iterats = TEXTX*TEXTY;
	unsigned long iterats2 = (TEXTX*TEXTY) >> 1;
	long iterats3 = -(((long)TEXTX*(long)TEXTY) << 1);
	short *scur_limit = scur + TEXTX*TEXTY;

	_asm
	{
		mov		edi, marker
		mov		eax, gvzShadowOne
		mov		esi, scur_limit
		mov		edx, iterats3
		mov		bx, 0
		mov		ecx, 0

blit_loop_zero:
		add		bx, word ptr[esi + edx]
		jz		no_blit_this_time_zero
		xor		eax, gvzShadowOne
		rep		stosd

no_blit_this_time_nonzero:
		add		ecx, 1
		add		edx, 2
		jz		end_of_blit

		add		bx, word ptr[esi + edx]
		jnz		no_blit_this_time_nonzero
		xor		eax, gvzShadowOne
		rep		stosd

no_blit_this_time_zero:
		add		ecx, 1
		add		edx, 2
		jnz		blit_loop_zero

end_of_blit:
		xor		eax, gvzShadowOne
		rep		stosd
		mov		ecx, iterats2
		mov		edi, scur
		xor		eax, eax
		rep		stosd
	}
}

//=====================================================================================//
//                           void ComplexShadow::BlitLinez()                           //
//=====================================================================================//
void ComplexShadow::BlitLinez()
{
//	if(bLinez)
	{
		for(Lines_t::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if(i->ct)
			{
				ShadowUtility::Line(this, i->bi->xi, i->bi->yi, i->li->xi, i->li->yi, i->ct);
			}
		}
	}

	bLinez = false;
}

} // namespace