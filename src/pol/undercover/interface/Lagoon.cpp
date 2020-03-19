/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: ������� ����� ��� ������������ ���� ��������� ����������.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   22.05.2000

************************************************************************/
#include "Precomp.h"
#include <undercover/globals.h>
#include <common/GraphPipe/GraphPipe.h>
#include "Lagoon.h"

//---------- ��� ���� ------------
#ifdef _DEBUG_INTERFACE
CLog lagoon_log;
#define lag	lagoon_log["lagoon.log"]
#else
#define lag	/##/
#endif
//--------------------------------

Lagoon::Lagoon(const char *pName)
:	m_Object(4)
{
	m_sName = pName;
	m_sClass = "lagoon";
	m_bScaleX = m_bScaleY = true;
	m_hVirtualRegion = 0;
	m_hScreenRegion = 0;
	//	----------------------------
	m_zBegin = 0.7;		//	������ ����������� ����������
	m_zEnd = 0.9;		//	����� ����������� ����������
	//  ������������� ������� �������
	SetRect(0,0,0,0);
	//	----------------------------
	m_pParent = 0;
	m_bFocus = false;
	m_bFocusValid = false;
	m_bVisible = true;
	m_bEnable = false;
	m_bMouseCapture = false;
	//	������������� ��� ������������ �����������
	m_Object.Verts[0].uv = texcoord(0,0);
	m_Object.Verts[1].uv = texcoord(1,0);
	m_Object.Verts[2].uv = texcoord(1,1);
	m_Object.Verts[3].uv = texcoord(0,1);
	//m_TexCoord[0] = texcoord(0,0);
	//m_TexCoord[1] = texcoord(1,0);
	//m_TexCoord[2] = texcoord(1,1);
	//m_TexCoord[3] = texcoord(0,1);

	//m_Object.Pos = m_Vertex;
	//m_Object.UVs[0] = m_TexCoord;
	//m_Object.IdxNum = 0;
	m_Object.Prim = Primi::TRIANGLEFAN;
	m_Object.Contents = 0;
	//m_Object.VertNum = 4;

	m_sShader = "noshader";
	SetVertex();
	//	------------------------------------------
	lag("    ...leave constructor Lagoon(\"%s\");\n",pName);
}

Lagoon::~Lagoon()
{
	ClearChildMap();
	lag("    ...leave destructor ~Lagoon(\"%s\");\n",m_sName.c_str());
}

bool Lagoon::IsClass(const char *pClass)
{
	return !strcmp(pClass,m_sClass.c_str());
}

//	------  ������� ��� ������ � ��������� ��������� ------------
void Lagoon::AddChild(Lagoon* pLagoon)
{
	std::map<std::string,Lagoon*>::iterator il;
	std::set<ZOrder>::iterator iz;
	int i;
	float step;

	if(pLagoon)
	{
		lag("%s: AddChild(%s);\n",Name(),pLagoon->Name());
		il = m_ChildMap.find(pLagoon->Name());
		if(il == m_ChildMap.end())
		{
			m_ChildMap.insert(std::map<std::string,Lagoon*>::value_type(pLagoon->Name(),pLagoon));
			//	������������ zBuffer'�
			step = (m_zEnd-m_zBegin)/(m_ChildMap.size()+1);
			lag("step = %f;\n",step);
			for(i=0,iz=m_zBuffer.begin();iz!=m_zBuffer.end();iz++,i++)
			{
				iz->pLagoon->m_zEnd = (m_zEnd-(step*(i+1)));
				iz->pLagoon->m_zBegin = iz->pLagoon->m_zEnd-step;
//				lag("z: [%f,%f];\n",iz->pLagoon->m_zBegin,iz->pLagoon->m_zEnd);
				iz->pLagoon->RefreshZ();
			}
			pLagoon->m_zEnd = (m_zEnd-(step*(i+1)));
			pLagoon->m_zBegin = pLagoon->m_zEnd-step;
			pLagoon->RefreshZ();
			m_zBuffer.insert(ZOrder(pLagoon));
//			lag("new z: [%f,%f];\n",pLagoon->m_zBegin,pLagoon->m_zEnd);
			pLagoon->m_pParent = this;
		}
		else
		{
			throw CASUS(std::string("������� � ������ <")+std::string(pLagoon->Name())+std::string("> ��� ����������.\n"));
		}
	}
}

void Lagoon::DeleteChild(const char *pName)
{
	std::set<ZOrder>::iterator iz;
	std::map<std::string,Lagoon*>::iterator il;

	il = m_ChildMap.find(pName);
	if(il == m_ChildMap.end())
		throw CASUS(std::string("������� � ������ <")+std::string(pName)+std::string("> �� ����������.\n"));
	//	������� �� ZBuffer'�
	for(iz=m_zBuffer.begin();iz!=m_zBuffer.end();iz++)
	{
		if(iz->pLagoon == il->second)
		{
			m_zBuffer.erase(iz);
			break;
		}
	}
	//	������� ������
	delete il->second;
	m_ChildMap.erase(il);
}

void Lagoon::ClearChildMap(void)
{
	std::map<std::string,Lagoon*>::iterator i;

	for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
	{
		delete i->second;
	}
	m_ChildMap.clear();
	m_zBuffer.clear();
}

void Lagoon::RefreshZ(void)
{
	std::set<ZOrder>::iterator iz;
	int i;
	float step;

	//	������������ zBuffer'�
//	lag("%s: RefreshZ -------------->;\n",Name());
	step = (m_zEnd-m_zBegin)/(m_ChildMap.size()+1);
//	lag("step = %f;\n",step);
	for(i=0,iz=m_zBuffer.begin();iz!=m_zBuffer.end();iz++,i++)
	{
		iz->pLagoon->m_zEnd = (m_zEnd-(step*(i+1)));;
		iz->pLagoon->m_zBegin = iz->pLagoon->m_zEnd-step;
//		lag("z: [%f,%f];\n",iz->pLagoon->m_zBegin,iz->pLagoon->m_zEnd);
		iz->pLagoon->RefreshZ();
	}
	SetVertex();
}
//	------  ������� ��������� ������������ ������� ------------
//	left upper == (left,top)  right,lower == (right,bottom)
//	����������� ����������
void Lagoon::SetRect(RECT *pRect)
{
	if(m_hVirtualRegion)
		DeleteObject(m_hVirtualRegion);
	//	-------------------------------
	m_hVirtualRegion = CreateRectRgn(pRect->left,pRect->top,pRect->right,pRect->bottom);
	//	����������
	m_x = pRect->left;
	m_y = pRect->top;
	m_width = pRect->right - pRect->left;
	m_height = pRect->bottom - pRect->top;
	TransformToScreen();
//	lag("    ...leave SetRect;\n");
}
//	����������� ����������
void Lagoon::SetRect(int left,int top,int right,int bottom)
{
	if(m_hVirtualRegion)
		DeleteObject(m_hVirtualRegion);
	//	-------------------------------
	m_hVirtualRegion = CreateRectRgn(left,top,right,bottom);
	//	����������
	m_x = left;
	m_y = top;
	m_width = right - left;
	m_height = bottom - top;
	TransformToScreen();
}
//	����������� ����������
void Lagoon::SetRegion(HRGN hRegion)
{
	RECT rect;

	if(m_hVirtualRegion)
		DeleteObject(m_hVirtualRegion);
	//	-------------------------------
	m_hVirtualRegion = hRegion;
	//	����������
	GetRgnBox(m_hVirtualRegion,&rect);
	m_x = rect.left;
	m_y = rect.top;
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;
	TransformToScreen();
//	lag("    ...leave SetRegion; RECT(%d,%d,%d,%d)\n",m_x,m_y,m_width,m_height);
}

void Lagoon::MoveTo(int x,int y)
{
	//	�������� ������ �������
	OffsetRgn(m_hVirtualRegion,x-m_x,y-m_y);
	m_x = x;
	m_y = y;
	//	�������� �������� ���������
	TransformToScreen();
	//	���������� ������� ��� �������� ��������
	std::map<std::string,Lagoon*>::iterator i;
	for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
	{
		i->second->MoveTo(x,y);
	}
}

//	------  ������� �������� ����������� ��������� � �������� ------------
void Lagoon::TransformToScreen(void)
{
	BYTE *lpRgnData = NULL;
	DWORD dwBytes;
	XFORM xForm = {m_bScaleX?((float)D3DKernel::ResX()/VirtualResX):1,0,0,m_bScaleX?((float)D3DKernel::ResY()/VirtualResY):1,0,0};

	//	�������� ���������� ��������� ���� ��� RGNDATA
	dwBytes = GetRegionData(m_hVirtualRegion,0,NULL);
	//	�������� ������
	lpRgnData = new BYTE[dwBytes];
	if(lpRgnData)
	{
		//	�������� ������
		GetRegionData(m_hVirtualRegion,dwBytes,(RGNDATA *)lpRgnData);
		//	������� ���������� ������
		if(m_hScreenRegion)
			DeleteObject(m_hScreenRegion);
		//	�������������� ������
		m_hScreenRegion = ExtCreateRegion(&xForm,dwBytes,(RGNDATA *)lpRgnData);

		delete[] lpRgnData;
	}
	SetVertex();
	//	�� �� ����� ��� �������� ��������
	std::map<std::string,Lagoon*>::iterator i;
	for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
	{
		i->second->TransformToScreen();
	}
}

//	------  ������� ����������� ------------
void Lagoon::PaintRegion(HDC hdc)
{
	RECT rect;
	GetRgnBox(m_hScreenRegion,&rect);
	FrameRgn(hdc,m_hScreenRegion,(HBRUSH)GetStockObject(WHITE_BRUSH),1,1);
	SetTextColor(hdc,0xffffff);
	DrawText(hdc,m_sName.c_str(),-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	//	��������� �������� ��������
	std::set<ZOrder>::iterator izEnd = m_zBuffer.end();
	std::set<ZOrder>::iterator iz;
	for(iz=m_zBuffer.begin();iz!=izEnd;iz++)
	{
		iz->pLagoon->PaintRegion(hdc);
	}
}

void Lagoon::Render(GraphPipe *lpGraphPipe)
{
	if(m_bVisible)
	{
		//	��������� ������ ��������
#ifdef _HOME_VERSION
		lpGraphPipe->Chop(m_sShader,&m_Object,"Lagoon");
#else
		lpGraphPipe->Chop(m_sShader,&m_Object);
#endif
		//	lag("m_Object(%s).z: %f - %f;\n",Name(),m_Vertex[0].z,m_zBegin);
		//	��������� �������� ��������
		std::set<ZOrder>::iterator izEnd = m_zBuffer.end();
		std::set<ZOrder>::iterator iz;
		for(iz=m_zBuffer.begin();iz!=izEnd;iz++)
		{
			iz->pLagoon->Render(lpGraphPipe);
		}
	}
}

void Lagoon::SetVisible(const char *pChild,bool bVisible)
{
	std::map<std::string,Lagoon*>::iterator im;

	im = m_ChildMap.find(pChild);
	if(im!=m_ChildMap.end())
		im->second->m_bVisible = bVisible;
}

void Lagoon::SetVisible(bool bVisible)
{
	std::map<std::string,Lagoon*>::iterator i;

	m_bVisible = bVisible;
	for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
	{
		i->second->SetVisible(bVisible);
	}
}

//	--------- �������������� ��������������� ������� ----------------
void Lagoon::SetVertex(void)
{
	m_Object.Verts[0].pos = point3(ToScreenX(m_x),ToScreenY(m_y),m_zEnd);
	m_Object.Verts[1].pos = point3(m_Object.Verts[0].pos.x+ToScreenX(m_width),m_Object.Verts[0].pos.y,m_zEnd);
	m_Object.Verts[2].pos = point3(m_Object.Verts[0].pos.x+ToScreenX(m_width),m_Object.Verts[0].pos.y+ToScreenY(m_height),m_zEnd);
	m_Object.Verts[3].pos = point3(m_Object.Verts[0].pos.x,m_Object.Verts[0].pos.y+ToScreenY(m_height),m_zEnd);
}

Lagoon* Lagoon::GetLagoon(int x,int y,bool bAlways)
{
	std::set<ZOrder>::reverse_iterator riz;
	Lagoon *pLagoon;

	if(bAlways || (!bAlways && m_bEnable && m_bVisible))
	{
		for(riz=m_zBuffer.rbegin();riz!=m_zBuffer.rend();riz++)
		{
			if((pLagoon = riz->pLagoon->GetLagoon(x,y,bAlways))!=0)
			{
				return pLagoon;
			}
		}
		if(PtInRegion(m_hScreenRegion,x,y))
			return this;
	}

	return 0;
}

Lagoon* Lagoon::GetFocus(void)
{
	std::map<std::string,Lagoon*>::iterator i;
	Lagoon *pLagoon;

	if(m_bEnable)
	{
		//	��������� ����
		if(m_bFocus)
			return this;
		//	��������� ������
		for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
		{
			if((pLagoon = i->second->GetFocus())!=0)
				return pLagoon;
		}
	}
	//	������ �� �����
	return 0;
}

void Lagoon::Tune(void)
{
	std::map<std::string,Lagoon*>::iterator i;

	TransformToScreen();
	for(i=m_ChildMap.begin();i!=m_ChildMap.end();i++)
	{
		i->second->Tune();
	}
}

//****************** ������������ ������� ************************//
//	�������� ���������� ��������
void Lagoon::Tick(void)
{
	std::set<ZOrder>::reverse_iterator riz;

	for(riz=m_zBuffer.rbegin();riz!=m_zBuffer.rend();riz++)
	{
		riz->pLagoon->Tick();
	}
}
//	���� ������ ��� ������ �� �������
void Lagoon::MouseIn(void)
{
}
//	���� ����������� �� ��������
void Lagoon::MousePresent(void)
{
}
//	���� ������ ��� �������� �������
void Lagoon::MouseOut(void)
{
}
//	���� �������� ������� �� ��� ���������
void Lagoon::MouseCaptureOut(void)
{
}
//	�������� ��������� ������������� ����
void Lagoon::SetMessage(unsigned int /*iMsg*/,Lagoon * /*pLagoon*/)
{
}
//	�������� ������ �����
void Lagoon::SetFocus(void)
{
		m_bFocus = true;
}
//	������ ������ �����
void Lagoon::KillFocus(void)
{
	m_bFocus = false;
}
//	����� ������������
void Lagoon::FocusPresent(void)
{
}
