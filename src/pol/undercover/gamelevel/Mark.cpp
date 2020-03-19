// Mark.cpp: implementation of the Mark class.
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "Mark.h"
#include <common/GraphPipe/graphpipe.h>
#include "../iworld.h"
#include "GameLevel.h"

////////////////////////////////////////////////////////////////////////////
//
// ������� ����� ��������
//
////////////////////////////////////////////////////////////////////////////

// �����������
BaseMark::BaseMark(const point3& Position, const float Radius, const std::string& Shader)
:	prim(MAXTRIS*3)
{
	Dead = false;
	Col = 0xffffffff;
	Visible = true;
	Pos = Position;
	Rad = Radius;
	//TriNum = 0;
	ShaderName = Shader;
	Savable=false;
	prim.Verts.clear();
	prim.Contents=Primi::NEEDTRANSFORM;
	prim.Prim=Primi::TRIANGLE;
}

////////////////////////////////////////////////////////////////////////////
//
// ����� ��������� ��������
//
////////////////////////////////////////////////////////////////////////////
// ���������� ���������
void Flicker::Update(const float /*CurTime*/)
{
}

////////////////////////////////////////////////////////////////////////////
//
// ����� �������
//
////////////////////////////////////////////////////////////////////////////
// �����������
Splash::Splash(const point3& Position, const float Radius, const std::string& Shader,
			   const point3& Color, const float BirthTime, const float LifeTime)
			   : BaseMark(Position, Radius, Shader)
{
	birthtime = BirthTime;
	lifetime = LifeTime;
	start_color = Color;
	Col = 0;
}
// ���������� ���������
void Splash::Update(const float CurTime)
{
	if(CurTime > (birthtime + lifetime))
	{
		Dead = true;
		return;
	}

	float f = 255.0 * (1.0 - (CurTime - birthtime)/lifetime);
	point3 p = start_color*f;
	Col = RGBA_MAKE((int)p.x, (int)p.y, (int)p.z, (int)f);
}

////////////////////////////////////////////////////////////////////////////
//
// ����� ��������� ��������
//
////////////////////////////////////////////////////////////////////////////
// ���������� ���������
void DMark::Update(const float CurTime)
{
	if(CurTime > (birthtime + lifetime))
	{
		Dead = true;
		return;
	}

	// �����
	float f = 255.0;
	// ����������
	if(CurTime < (birthtime + switchtime))
	{
		f = 255.0 * (CurTime - birthtime)/switchtime;
	}
	// ������
	if(CurTime > (birthtime + lifetime - switchtime))
	{
		f = 255.0 * (birthtime + lifetime - CurTime)/switchtime;
	}
	point3 p = start_color*f;
	Col = RGBA_MAKE((int)p.x, (int)p.y, (int)p.z, (int)f);
	FindTris=true;
}

/////////////////////////////////////////////////////////////////////////
// ���������� MarksPool
/////////////////////////////////////////////////////////////////////////
MarksPool::~MarksPool()
{
	clear();
}
void MarksPool::clear()
{
	m_Marks.clear();
}
void MarksPool::Draw()
{
	GraphPipe *Pipe=IWorld::Get()->GetPipe();
	iterator it=begin(),ite=end();
	for(;it!=ite;++it)
	{
		if(!it->Visible) continue;
		//if(!it->TriNum) continue;
		//Primi t;
		//t.Pos=it->Tris;
		//t.UVs[0]=it->UVs;
		//t.VertNum=it->TriNum*3;
		//t.Diffuse=it->Color;
		//t.IdxNum=0;
		//t.Contents=Primi::NEEDTRANSFORM;
		//t.Prim=Primi::TRIANGLE;
#ifdef _HOME_VERSION
		Pipe->Chop(it->ShaderName,&it->prim,"Mark");
#else
		Pipe->Chop(it->ShaderName,&it->prim);
#endif
	}
}
void MarksPool::Update(float Time)
{
	GraphPipe *Pipe=IWorld::Get()->GetPipe();
	BSphere bs;
	MarksSet::iterator it=m_Marks.begin(),ite=m_Marks.end();
	while(it!=ite)
	{
		BaseMark &d=**it;
		d.Update(Time);
		if(d.Dead)
		{
			m_Marks.erase(it++);
			continue;
		}
		bs.Center=d.Pos;bs.Radius=d.Rad;
		if(Pipe->GetCam()->Cone.TestBSphere(bs)==Frustum::NOTVISIBLE)
		{
			d.Visible=false;
			++it;
			continue;
		}
		else       d.Visible=true;

		if(d.FindTris)
		{
			d.prim.Verts.clear();
			//Primi::FVFVertex *vts = 
			//d.TriNum=0;
			if(IWorld::Get()->GetLevel()->MarksEnabled(GameLevel::MT_DYNAMIC))
			{
				IWorld::Get()->GetLevel()->CollectPlanes(d.Pos,d.Rad, d);

				for(int _i=0;_i<d.GetTriangleCount();_i++)//FIXME!
				{
					d.prim.Verts[_i*3].diffuse = d.Col;
					d.prim.Verts[_i*3+1].diffuse = d.Col;
					d.prim.Verts[_i*3+2].diffuse = d.Col;
				}
			}
		}
		else
		{
			for(int _i=0;_i<d.GetTriangleCount();_i++)//FIXME!
			{
				d.prim.Verts[_i*3].diffuse = d.Col;
				d.prim.Verts[_i*3+1].diffuse = d.Col;
				d.prim.Verts[_i*3+2].diffuse = d.Col;
			}
		}
		++it;
	}
}
void MarksPool::MakeSaveLoad(SavSlot &slot)
{
	return;
	iterator it=begin(),ite=end();
	if(slot.IsSaving())
	{
		int num=0;
		for(;it!=ite;it++)  if(it->Savable) num++;

		slot<<num;
		for(it=begin();it!=ite;it++)
		{
			if(it->Savable)
			{
				slot<<it->Pos<<it->Rad<<it->ShaderName;
			}
		}
	}
	else
	{
		int n=0;
		slot>>n;
		for(;n;--n)
		{
			point3 pos;
			float rad;
			std::string ShaderName;
			slot>>pos>>rad>>ShaderName;
			//	void Add(BaseMark *Mark){m_Marks.push_back(Mark);}
			boost::shared_ptr<Mark> mrk(new Mark(pos,rad,ShaderName));
			mrk->prim.Verts.clear();
			IWorld::Get()->GetLevel()->CollectPlanes(mrk->Pos,mrk->Rad, *mrk);
			Add(mrk);
		}
	}
}

