//
//  ������� ��� ������ � �����
//

#ifndef _PUNCH_PATHUTILS_H_
#define _PUNCH_PATHUTILS_H_

class BusRoute;

class BaseEntity;
class VehicleEntity;

//
// ����� ������ ��� ������� ����
//
class PathUtils
{
public:
    //��������� �������� �� �����
    static void LinkEntity(BaseEntity* entity);
    static void LinkEntity(BaseEntity* entity, const ipnt2_t &);
    //����� �������� � �����
    static void UnlinkEntity(BaseEntity* entity);

    enum flag_type
	{
        F_CALC_NEAR_PNTS      = 1 << 0, //������� ����� ������� � ���������
        F_CALC_OVER_INVISIBLE = 1 << 1, //������� ���� ������ ��������� �������
        F_PASS_ALL_ENTITIES   = 1 << 2, //����������� ���� ������ ���� �������   

        //������ ������ ��� ������ ��������
        F_PLAYER_CALC  = F_CALC_NEAR_PNTS|F_CALC_OVER_INVISIBLE,                 
        F_DEFAULT_CALC = F_PLAYER_CALC,
    };

    // ��������� ���� ������������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // front - ���������� ������� �� ���. ����� �����
    // pass  - ���������� ��� ����� � ��� ����� ������ 
    //
    static void CalcPassField(BaseEntity* entity, pnt_vec_t* front = 0, pnt_vec_t* pass = 0, unsigned flags = F_DEFAULT_CALC);

    // ��������� ���� ������������ ��� ����� �������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // pass  - ���������� ��� ����� � ��� ����� ������ 
    //
    static void CalcPassField(const ipnt2_t& center, float radius, pnt_vec_t* pass = 0);

    //��������� ���� ������� ��� �����
    static void CalcLandField(VehicleEntity* vehicle, pnt_vec_t* front = 0);
    //��������� ������ hex'�� ���������� ���������
    static void CalcTrackField(BaseEntity* entity, ipnt2_t pos, pnt_vec_t* vec);

    //����� ����
    static void CalcPath(BaseEntity* entity, ipnt2_t to, pnt_vec_t& pnts);
  
    //����� �� ��������� �������� � ��� �������
    static bool IsEmptyPlace(const ipnt2_t& pos, vehicle_size vsize);
    
    //��������� ��� ������� ����� �������
    struct near_pnt_t{
        int     m_cost;
        ipnt2_t m_pnt;

        bool IsDefPnt() const;
    };

    //������ ���������� �� ����� ������� � ��������
    static bool IsNear(BaseEntity* entity); 
    //�������� ��������� ����� �������
    static const near_pnt_t& GetNearPnt(BaseEntity* entity);

    //������ ��������� ����� ������� � ���. ��������� hex'��
    static const near_pnt_t& GetNearPnt(const pnt_vec_t& vec);

private:

    //�������� ���� ������
    static void FindLandPnts(ipnt2_t* first, ipnt2_t* last, pnt_vec_t* front);

private:

    //������ ����� �������
    static near_pnt_t  m_near_pnts[MAX_ENTITIES];
};

//
// ����� - ��������� ���������� ��������� �� ������
//
class BusDispatcher
{
public:
    //����������� � �������� �� �������
    static void Reset();

    //���������������� �������
    static void Insert(VehicleEntity* bus, BusRoute* route);
    //����� ������� � ������
    static void Remove(VehicleEntity* bus, BusRoute* route);

private:
    //������ ���� ������� �������� � ��������� ��������
    template<class Entity, class Route, class Operation>
    static void BusRouteWalker(Entity* entity, Route* route, Operation oper)
    {
        BusRoute::pos_s pos;
        
        route->GenLoc(true, &pos);

        pnt_vec_t path;

        while(route->GetPntType(pos.m_pos) != Route::PT_LAST){
            
            path.clear();
            
            route->GetPath(pos.m_pos, &path);

            if(path.empty()) return;
            
            pos.m_pos = path[0];
            
            for(size_t k = 0; k < path.size(); k++)
			{
                oper.SetPos(path[k]);
				oper.DoVisit(entity);
                //entity->Accept(oper);
            }
        }
    }

private:
    static eid_t m_buses[MAX_BUSES];
};

//
// �������� ��������� ������� � ��� �������
//
class AreaManager{
public:

    AreaManager(const pnt_vec_t& vec);
    ~AreaManager();

    // 
    // �������� �� ��������� � �������
    // 
    class Iterator{
    public:

        Iterator(size_t first = 0, unsigned etype = 0, unsigned ptype = 0,
                 unsigned attrs = 0, pnt_vec_t* area = 0);
        
        Iterator& operator ++();
        Iterator  operator ++(int) 
        { Iterator tmp = *this;  operator++(); return tmp;}
        
        //��������� ������� � ���������
        BaseEntity* operator ->();
        BaseEntity& operator * (){ return *operator->();}
        
        //��������� �� !=
        friend bool operator != (const Iterator& i1, const Iterator& i2)
        { return i1.m_first != i2.m_first; }
        
        friend bool operator == (const Iterator& i1, const Iterator& i2)
        { return i1.m_first == i2.m_first; }

    private:

        bool IsSuitable(const ipnt2_t hex_pnt);

    private:
        
        typedef std::set<eid_t> eid_set_t;

        unsigned m_etype;
        unsigned m_ptype;
        unsigned m_attrs;

        size_t m_first;

        eid_set_t  m_eids;
        pnt_vec_t* m_area;
    }

    typedef iterator;

    iterator begin(unsigned entity = ET_ALL_ENTS, unsigned player = PT_ALL_PLAYERS, unsigned attrs = EA_ALL_ATTRIBUTES);
    iterator end();

private:

    pnt_vec_t m_area;
};

#endif // _PUNCH_PATHUTILS_H_