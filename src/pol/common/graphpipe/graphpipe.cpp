/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: Этот объект реализует графический pipeline
т.е. сюда отгружаются графические объекты, которые 
отсекаются камерой, у которых удаляются невидимые линии
и тому подобное. Здесь же происходит сортировка
по шейдерам.
Author: Grom 
Creation: 12 апреля 2000
***************************************************************/                
#include "precomp.h"
#include "simpletexturedobject.h"
#include "../utils/profiler.h"
#include "../GraphPipe/statesmanager.h"

#include "graphpipe.h"

//#define GP_LOG(L)	MLL_MAKE_LOG("graphpipe.log",L)
#define GP_LOG(L)

D3DMATRIX supermat;
enum {DYNBUFSIZE=4096*8/*65000*/,PARTBUFSIZE=10000}; //размер буфера вершин под шейдер

//=====================================================================================//
//                               GraphPipe::GraphPipe()                                //
//=====================================================================================//
GraphPipe::GraphPipe()
{
	CacheTextures=true;
	//Cam.SetProjection(TORAD(60),1,420,768.f/1024.f);
	//Cam.SetLocation(point3(10,10,10),point3(1,1,-1));
	DynBuf=NULL;
}

//=====================================================================================//
//                               GraphPipe::~GraphPipe()                               //
//=====================================================================================//
GraphPipe::~GraphPipe()
{
	Release();
}

//=====================================================================================//
//                           void GraphPipe::CreateVBuffer()                           //
//=====================================================================================//
//создание буфера вершин  
void GraphPipe::CreateVBuffer()
{
	unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1);
	//unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_DIFFUSE ;
	if(DynBuf) delete DynBuf;
	DynBuf=new DynamicVB( D3D,FVFflags,DYNBUFSIZE,sizeof(float)*3*2+4*sizeof(float)+sizeof(int));

}

//инициализация данных
//=====================================================================================//
//                               void GraphPipe::Init()                                //
//=====================================================================================//
void GraphPipe::Init(IDirect3D7 *_D3D, IDirect3DDevice7 *_D3DDev, float X, float Y)
{  
	StatesManager::Clear();
	D3DUtil_SetTranslateMatrix(supermat,-1,1,0);
	supermat._11=2.f/D3DKernel::ResX();
	supermat._22=-2.f/D3DKernel::ResY();

	D3D=_D3D;D3DDev=_D3DDev;ResX=X;ResY=Y;
	//не забыть обновить шейдеры      ]
	CreateVBuffer();
	GP_LOG("GraphPipe Init\n");
	StringSet::iterator ss=ShaderFiles.begin(),sse=ShaderFiles.end();
	while(ss!=sse)
	{
		LoadShaders(*ss);
		ss++;
	}
	LoadShaders("shaders.sys/error.shader");

	//Cam.SetProjection(TORAD(60),1,420,768.f/1024.f);

	Cam.Move(point3(0,0,0));
}

//=====================================================================================//
//                         void GraphPipe::SwitchRenderMode()                          //
//=====================================================================================//
void GraphPipe::SwitchRenderMode(RENDER_MODE Mode)
{
	switch(Mode)
	{
	case RM_2D:
		if(LastSpace==RM_3D||LastSpace==RM_NONE)
		{
			//logFile["testing.log"]("switched to 2D mode\n");
			StatesManager::SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&supermat);
			StatesManager::SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)IDENTITYMAT);
			StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
			LastSpace=RM_2D;
		}
		break;
	case RM_3D:
		if(LastSpace==RM_2D||LastSpace==RM_NONE)//FIXME
		{
			//logFile["testing.log"]("switched to 3D mode\n");
			StatesManager::SetTransform( D3DTRANSFORMSTATE_PROJECTION, &mll::algebra::D3DMATRIX(Cam.GetProj()));
			StatesManager::SetTransform( D3DTRANSFORMSTATE_VIEW, &mll::algebra::D3DMATRIX(Cam.GetView()));
			LastSpace=RM_3D;
		}
		break;
	case RM_NONE:
		//logFile["testing.log"]("switched to initial mode\n");
		LastSpace=RM_NONE;
		break;
	}
}

//=====================================================================================//
//                               void GraphPipe::Chop()                                //
//=====================================================================================//
//загрузка примитивов в pipeline
#if defined(_HOME_VERSION)
void GraphPipe::Chop(const std::string &Shader, const Primi *Prim, unsigned int pcount, const char *dataType)
#else
void GraphPipe::Chop(const std::string &Shader, const Primi *Prim, unsigned int pcount)
#endif
{
//#if defined(_HOME_VERSION)
//	GP_LOG(pcount << "\t" << dataType << "\n");
//#endif
	if(!pcount) return;
	if(!Prim->Idxs.size() && !Prim->Verts.size()) return;
	VShader *vs=FindShader(Shader);
	if(!vs) //не смогли найти даже  ошибочный шейдер
	{
		std::string _e=Shader;
		_e=std::string(">>>'")+_e+"'<<<\n мало того, что не найден шейдер\n"
			"нет даже стандартного шейдера 'noshader'\n\n";
		throw CASUS(_e.c_str());
	}

	if(Prim->Contents&Primi::NEEDTRANSFORM)
	{//просто скопировать вершины в буффер
		SwitchRenderMode(RM_3D);
	}
	else
	{//записать в буффер вершины после трансформации
		SwitchRenderMode(RM_2D);
	}

	//logFile["testing.log"]("chopping shader '%s' in %s mode\n",Shader.c_str(),LastSpace==RM_2D?"2d":(LastSpace==RM_3D?"3d":"error"));
	vs->Chop(Prim,D3DDev,DynBuf,pcount);
}

//=====================================================================================//
//                               void GraphPipe::Chop()                                //
//=====================================================================================//
#if defined(_HOME_VERSION)
void GraphPipe::Chop(const std::string &Shader, const Primi *Prim, const char *dataType)
{
	Chop(Shader,Prim,Prim->GetVertexCount(),dataType);
}
#else
void GraphPipe::Chop(const std::string &Shader, const Primi *Prim)
{
	Chop(Shader,Prim,Prim->GetVertexCount());
}
#endif
//=====================================================================================//
//                            void GraphPipe::LoadShaders()                            //
//=====================================================================================//
void GraphPipe::LoadShaders(std::string FileName)
{
	ShaderFile=FileName;
	GP_LOG("Загружаем шейдер: " << FileName << "\n");
	ShdCompiler Loader(this);
	Loader.Compile(FileName.c_str());

}

//=====================================================================================//
//                              void GraphPipe::Release()                              //
//=====================================================================================//
void GraphPipe::Release()
{
	DESTROY(DynBuf);
	ShaderMap::iterator i=Shaders.begin();
	while(i!=Shaders.end())
	{
		i->second.Release(D3DDev);
		i++;
	}
	Shaders.clear();
	GP_LOG("GraphPipe Release\n");
}

//=====================================================================================//
//                          int GraphPipe::TransformPoints()                           //
//=====================================================================================//
int GraphPipe::TransformPoints(int Size, const point3 *Input, point3 *Out)
{
	memcpy(Out,Input,Size*sizeof(*Input));
	return Size;
}

//=====================================================================================//
//                          int GraphPipe::TransformPoints()                           //
//=====================================================================================//
int GraphPipe::TransformPoints2(int Size, const point3 *Input, point3 *Out)
{
	//FIXME: переделать цикл умножения (матрицу необходимо сделать сразу со всеми преобразованиями)
	if(!Size) return 0;
	D3DMATRIX m = mll::algebra::D3DMATRIX(Cam.GetView()*Cam.GetProj());
	//D3DMath_MatrixMultiply(m,mv,mp);

	point3 *out=Out;
	const point3 *in=Input;
	/* by Flif float f,n,fov;*/
	int size=Size;
	while(size--)
	{
		PointMatrixMultiply(TODXVECTOR(*out),
			TODXVECTOR(*in),m);
		out->x+=1;
		out->y=1-out->y;
		out->x *= D3DKernel::ResX()*0.5f;//*Cam.GetParams().m_near;
		out->y *= D3DKernel::ResY()*0.5f;//*Cam.GetParams().m_near;
		if(out->x<0||out->y<0||out->x>=ResX||out->y>=ResY||
			out->z<0||out->z>1)
		{
			out->z=-1;
			//continue;
		}
		out++;
		in++;
	}
	return Size;
}

//=====================================================================================//
//                               void GraphPipe::Chop()                                //
//=====================================================================================//
#ifdef _HOME_VERSION
void GraphPipe::Chop(const TexObject *Obj, const char *dataType)
#else
void GraphPipe::Chop(const TexObject *Obj)
#endif
{
	//Primi  Prim;
	int ptnum;
	for(ptnum=0;ptnum<Obj->PartNum;ptnum++)
	{
		SimpleTexturedObject *s=Obj->Parts[ptnum];
		Primi &Prim = s->prim;
		//Prim.Pos=s->Points;
		//Prim.Norm=s->Normals;
		//Prim.UVs[0]=s->uv;
		//Prim.IdxNum=s->IdxNum;
		//Prim.Idxs=(unsigned short*)s->GetIndexesFull();
		//Prim.Prim=Primi::TRIANGLE;
		//Prim.Contents=Primi::NEEDTRANSFORM;
		//Prim.VertNum=s->PntNum;
		if(!Prim.Verts.size()&&!Prim.Idxs.size())continue;
		if(Prim.Verts.size())
		{
#ifdef _HOME_VERSION
			Chop(s->MaterialName,&Prim,dataType);
#else
			Chop(s->MaterialName,&Prim);
#endif
		}
	}

}

//=====================================================================================//
//                               void GraphPipe::Chop()                                //
//=====================================================================================//
void GraphPipe::Chop(const TexObject *Obj,const std::vector<const D3DMATRIX*> &Worlds,
					 DynamicVB *Buf,int StartVert, unsigned skip_flags)
{
	Primi  Prim;
	int ptnum;
	for(ptnum=0;ptnum<Obj->PartNum;ptnum++)
	{
		SimpleTexturedObject *s=Obj->Parts[ptnum];
		if(!s->prim.Verts.size()&&!s->prim.Verts.size())continue;
		VShader *vs=FindShader(s->MaterialName);
		if(!vs) //не смогли найти даже  ошибочный шейдер
		{
			std::string _e=s->MaterialName;
			_e=std::string(">>>'")+_e+"'<<<\n мало того, что не найден шейдер\n"
				"нет даже стандартного шейдера 'noshader'\n\n";
			throw CASUS(_e.c_str());
		}

		if(vs->Transparent)
		{      if(skip_flags&SF_TRANSPARENT) {StartVert+=s->prim.Verts.size();continue; } }
		else
		{      if(skip_flags&SF_SOLID) {StartVert+=s->prim.Verts.size();continue; }   }

		SwitchRenderMode(RM_3D);

		vs->Chop(D3DPT_TRIANGLELIST,Buf,StartVert,s->prim.Verts.size(),(unsigned short*)s->GetIndexesFull(),
			s->prim.Idxs.size(),Worlds);
		StartVert+=s->prim.Verts.size();
	}

}

void GraphPipe::DrawBBox(const BBox &Box,unsigned long Col)
{
	SwitchRenderMode(RM_3D);
	D3DLVERTEX corners[8]={
		D3DLVERTEX(D3DVECTOR(Box.minx,Box.miny,Box.minz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.maxx,Box.miny,Box.minz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.maxx,Box.maxy,Box.minz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.minx,Box.maxy,Box.minz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.minx,Box.miny,Box.maxz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.maxx,Box.miny,Box.maxz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.maxx,Box.maxy,Box.maxz),Col,0,0,0),
			D3DLVERTEX(D3DVECTOR(Box.minx,Box.maxy,Box.maxz),Col,0,0,0),
	};
	unsigned short idxs[12*2]={
		0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4,
			0,4, 1,5, 2,6, 3,7,
	};
	//настройка прозрачности
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
	//работа с Z-буфером
	StatesManager::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
	StatesManager::SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	StatesManager::SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StatesManager::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	StatesManager::SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StatesManager::SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE );

	D3DDev->SetTexture(0,0);
	D3DDev->DrawIndexedPrimitive(D3DPT_LINELIST,D3DFVF_LVERTEX,corners,8,idxs,12*2,0);
}

void GraphPipe::UnloadShader(const std::string &ShaderName)
{
	ShaderMap::iterator s=Shaders.find(ShaderName),e=Shaders.end();
	if(s!=e)  
	{
		s->second.Release(D3DDev);
		Shaders.erase(s);
	}
}

void GraphPipe::UnloadShaderFile(const std::string &FileName)
{
	GP_LOG("выгружаем файл с шейдерами: '" << FileName << "'\n");
	ShaderMap::iterator s=Shaders.begin(),e=Shaders.end(),s1;
	while(s!=e)
	{
		if(s->second.FileName==FileName)
		{
			GP_LOG("выгружаем шейдер: '" << s->second.ShaderName << "'\n");
			s1=s++;
			s1->second.Release(D3DDev);
			Shaders.erase(s1);
		}
		else
			s++;
	}
	ShaderFiles.erase(FileName);
}

void GraphPipe::RegisterShaderFile(const std::string &FileName, bool cachetextures)
{
	CacheTextures=cachetextures;
	ShaderFiles.insert(FileName);
	LoadShaders(FileName);
}

void GraphPipe::AdjustHW()
{
	//GP_LOG("ahw in\n");
	StatesManager::Clear();
	//Release();
	//Init(D3DKernel::GetD3D(),D3DKernel::GetD3DDevice(),D3DKernel::ResX(),D3DKernel::ResY());
	D3DKernel::RestoreSurfaces();

	ShaderMap::iterator s=Shaders.begin(),e=Shaders.end();
	while(s!=e)  
	{
		s->second.AdjustHW(D3DDev);
		s++;
	}
	StatesManager::Clear();
	//GP_LOG("ahw out\n");

}

bool GraphPipe::IsExist(const char* shd_name)
{
	bool isexist=Shaders.find(shd_name)!=Shaders.end();
	if(isexist)
	{
		GP_LOG("Уже загружен: '" << shd_name << "'\n");
	}
	return isexist;
}

void GraphPipe::OnShader(const char* shd_name, const shader_struct& shd)
{
	VShader *theShad = 0;
	Shaders[shd_name] = VShader(shd);
	theShad = &Shaders[shd_name];
	Shaders[shd_name].ShaderName=shd_name;
	if(CacheTextures)
		Shaders[shd_name].AdjustHW(D3DDev);
	Shaders[shd_name].FileName=ShaderFile;
	GP_LOG("Успешно загружен шейдер '" << shd_name << "'\n");
}

bool GraphPipe::OnError(const char* file_name, int line, SC_ERRORS type)
{
	GP_LOG("Ошибка в шейдере '" << file_name << "' строка:" << line << ", тип:" << ShaderOutput::error_desc[(int)type] << "\n");
	return true;
}

//=====================================================================================//
//                            void GraphPipe::StartFrame()                             //
//=====================================================================================//
void GraphPipe::StartFrame()
{
//#if defined(_HOME_VERSION)
//	GP_LOG("--------------- new frame started\n");
//#endif
	DynBuf->FlushAtFrameStart();
	SwitchRenderMode(RM_NONE);
	CodeProfiler::FrameStart();
}

//=====================================================================================//
//                             void GraphPipe::EndFrame()                              //
//=====================================================================================//
void GraphPipe::EndFrame()
{
	ApplyCamera();
}

//=====================================================================================//
//                        void GraphPipe::PushBspTrianglesDyn()                        //
//=====================================================================================//
//void GraphPipe::PushBspTrianglesDyn(VShader *sh,int ShdNum, DynamicVB *DynBuf)
//{
//	SwitchRenderMode(RM_3D);
//	sh->PushBspTrianglesDyn(ShdNum,DynBuf);
//}
