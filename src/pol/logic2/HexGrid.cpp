//
// ������ ��� ������ � HexGrid
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "hexgrid.h"
#include "aiutils.h"
#include "damager.h"
#include "hexutils.h"

HexGrid*  HexGrid::m_instance;

//=====================================================================================//
//                           bool HexUtils::IsValidHexPos()                            //
//=====================================================================================//
bool HexUtils::IsValidHexPos(const ipnt2_t &pos)
{
	return pos.x >= 0.0f && pos.y >= 0.0f 
		&& pos.x < HexGrid::GetInst()->GetSizeX() && pos.y < HexGrid::GetInst()->GetSizeY();
}

//========================================================

//=====================================================================================//
//                                HexGrid::cell::cell()                                //
//=====================================================================================//
HexGrid::cell::cell()
:	m_buses(0),
	m_entity(0),
	m_cost(NONE),
	m_risk(0)
{
}

//=====================================================================================//
//                               HexGrid::cell::~cell()                                //
//=====================================================================================//
HexGrid::cell::~cell()
{
}

//========================================================

//=====================================================================================//
//                               void HexGrid::ReadStr()                               //
//=====================================================================================//
void HexGrid::ReadStr(std::istringstream &stream, std::string& std_str)
{
	stream>>std::noskipws;
	std_str="";
	char ch;

	stream>>ch;
	while(ch!='\n'&& ch!='\r')
	{
		std_str+=ch;
		stream>>ch;
	}

	stream>>std::skipws;

}

//=====================================================================================//
//                                     void Skip()                                     //
//=====================================================================================//
void Skip(std::istringstream &stream, const std::string &Key)
{
	unsigned int i=0;
	unsigned char ch;
	for(i=0;i<Key.size();i++)
	{
		stream>>ch;
		if(ch!=Key[i]) 
			throw CASUS("�������� ����������� �����");
	}

}

//=====================================================================================//
//                                 HexGrid::HexGrid()                                  //
//=====================================================================================//
HexGrid::HexGrid(const char* name)
:	m_active_joints(0xffffffff)
{
	m_instance = this;

	PackageFile  file(name);
	if(!file.IsOk())
	{
		char str[20 + MAX_PATH];
		sprintf(str,"���� %s �� ������", name); 
		throw CASUS(str);
	}

	file.Seek(0,SEEK_END);int size=file.GetPos(); file.Seek(0,SEEK_SET);
	std::string t;
	t.resize(size);
	file.Read(&t[0],size);
	std::istringstream stream(t);
	// stream>>std::noskipws;

	//������� �����
	Skip(stream,"Size:");
	stream>>m_SizeX>>m_SizeY;
	// fscanf(file,"Size:%d %d\n",&m_SizeX,&m_SizeY);

	m_grid.swap(Grid_t(m_SizeX*m_SizeY));

	//������ hex'�
	Skip(stream,"HexSize:");
	stream>>m_HexSize;
	//fscanf(file,"HexSize:%f\n",&m_HexSize);

	//������� ������ ������
	for(int x = 0; x < m_SizeX; x++){
		for(int y = 0; y < m_SizeY; y++){
			cell *cur= &m_grid[x + y*m_SizeX];

			Skip(stream,"{(");
			stream>>cur->x >> cur->y >> cur->z;
			Skip(stream,")(");
			stream>>cur->m_weight[0] >> cur->m_weight[1] >> cur->m_weight[2] >> 
				cur->m_weight[3] >> cur->m_weight[4] >> cur->m_weight[5];
			Skip(stream,")");
			stream>>cur->m_prop;
			Skip(stream,"}");

			/*fscanf( file,"{(%f %f %f) (%d %d %d %d %d %d) %i}\n",
			&cur->x, &cur->y, &cur->z,
			&cur->m_weight[0], &cur->m_weight[1],
			&cur->m_weight[2], &cur->m_weight[3],
			&cur->m_weight[4], &cur->m_weight[5],
			&cur->m_prop);*/

			cur->m_prop++;
		}
	}

	//������ ���������� �������
	Skip(stream,"PropsNum=");

	{
		unsigned int propnum = 0;
		stream >> propnum;
		//�������� ������ �������
		m_props.swap(Props_t(propnum+1));
	}

	int         tmp_int;
	std::string tmp_str;

	for(int i = 1; i < (int)m_props.size(); i++)
	{
		//������� ��������
		//ReadStr(stream,tmp_str);
		stream.ignore(strlen("---\n"));
		//fscanf(file, "---\n");

		//��� ����
		Skip(stream,"RegionName:");
		//fscanf(file, "RegionName:");
		ReadStr(stream, m_props[i].m_zone);

		Skip(stream,"Joint:");
		stream>>std::hex>>m_props[i].m_joints>>std::dec;
		//fscanf(file, "Joint:%x\n", &m_props[i].m_joints);

		Skip(stream,"DynHeight:");
		stream>>tmp_int;
		//fscanf(file, "DynHeight:%i\n", &tmp_int);
		m_props[i].DynHeight=tmp_int?true:false;

		Skip(stream,"HeightObjName:");
		//fscanf(file, "HeightObjName:");
		ReadStr(stream, m_props[i].HeightObj);
		{for(size_t _t=0;_t<m_props[i].HeightObj.size();_t++)
			m_props[i].HeightObj[_t]=tolower(m_props[i].HeightObj[_t]);}

		Skip(stream,"DynamicManeuverability:");
		stream>>tmp_int;
		//fscanf(file, "DynamicManeuverability:%i\n", &tmp_int);
		m_props[i].DynMan=tmp_int?true:false;

		Skip(stream,"ManObjName:");
		//fscanf(file, "ManObjName:");
		ReadStr(stream, m_props[i].ManObj);
		{for(size_t _t=0;_t<m_props[i].ManObj.size();_t++)
			m_props[i].ManObj[_t]=tolower(m_props[i].ManObj[_t]);}

		Skip(stream,"DynamicManeuverability:");
		stream>>tmp_int;
		//fscanf(file, "DynamicManeuverability:%i\n", &tmp_int);
		m_props[i].ManInv=tmp_int?true:false;

		Skip(stream,"PlaceInfo:");
		stream>>std::hex>>tmp_int>>std::dec;
		//fscanf(file, "PlaceInfo:%x\n", &tmp_int);

		Skip(stream,"SideName");//���������� ��� :
		//fscanf(file, "SideName:");
		ReadStr(stream, tmp_str);

		Skip(stream,"BirthFlags:");
		stream>>std::hex>>tmp_int>>std::dec;
		//fscanf(file, "BirthFlags:%x\n", &tmp_int);

		stream>>tmp_str;

		stream>>tmp_str;

		stream>>tmp_str;

		stream>>tmp_str;

		stream>>tmp_str;
		/*stream.ignore(strlen("SubjName:"));
		//fscanf(file, "SubjName:");
		ReadStr(stream, tmp_str);*/
	}

	//��������� ������ hex'� ��� ������
	//fclose(file);

	//�������� �����
	{
		for(int x = 0; x < m_SizeX; x++)
		{
			for(int y = 0; y < m_SizeY; y++)
			{
				cell *cur= &m_grid[x + y*m_SizeX];
				if(cur->m_weight[0]>=50&&cur->m_weight[1]>=50&&cur->m_weight[2]>=50&&
					cur->m_weight[3]>=50&&cur->m_weight[4]>=50&&cur->m_weight[5]>=50)
				{
					for(int i=0;i<6;i++)
					{
						ipnt2_t np;
						HexUtils::GetFrontPnts0(ipnt2_t(x,y), i, &np);
						if(IsOutOfRange(np)) continue;
						m_grid[np.x + np.y*m_SizeX].m_weight[HexUtils::GetReverseDir(i)]=100;
					}
				}
				prop *cprop=&m_props[cur->m_prop];

				if(cprop->DynMan&&!cprop->ManInv)  //���� ������������ �������� ������� �� ��������� �������
					ManDirect[cprop->ManObj].push_back(ipnt2_t(x,y));

				if(cprop->DynMan&&cprop->ManInv)  //���� ������������ �������� ������� �� ��������� �������
					ManInvert[cprop->ManObj].push_back(ipnt2_t(x,y));

				if(cprop->DynHeight)  //���� ������ ������� �� �������
				{
					HeightDirect[cprop->HeightObj].push_back(ipnt2_t(x,y));
					for(int i=0;i<6;i++)
					{
						ipnt2_t np;
						HexUtils::GetFrontPnts0(ipnt2_t(x,y), i, &np);
						if(IsOutOfRange(np)) continue;
						cell *newc=&m_grid[np.x + np.y*m_SizeX];
						if(newc->m_prop==cur->m_prop)continue;
						HeightFront[cprop->HeightObj].push_back(heightfront(cur,newc,i));
						HeightFront[cprop->HeightObj].push_back(heightfront(newc,cur,HexUtils::GetReverseDir(i)));
					}
				}
			}
		}
	}  
}

//=====================================================================================//
//                                 HexGrid::~HexGrid()                                 //
//=====================================================================================//
HexGrid::~HexGrid()
{
	RiskArea::GetInst()->Reset();
	m_instance = 0;
}

//=====================================================================================//
//                           void HexGrid::SetActiveJoints()                           //
//=====================================================================================//
void HexGrid::SetActiveJoints(unsigned val)
{ 
	m_active_joints = val;
	AIUtils::CalcAndShowActiveExits(); 
}


