//
// ������� ���������
//

#ifndef _PUNCH_THINGFACTORY_H_
#define _PUNCH_THINGFACTORY_H_

class BaseThing;

//
// ������� ���������
//
class ThingFactory
{
public:
    //�������� ������� ������� ���������
    static ThingFactory* GetInst();

	//������� ����
    virtual BaseThing* CreateKey(const rid_t& rid) = 0;
    //������� ������ (���� sum == 0 ==> ����� ������� �� ���� ���������)
    virtual BaseThing* CreateMoney(int sum) = 0;
    //������� ������
    virtual BaseThing* CreateArmor(const rid_t& rid) = 0;
    //������� ������� ����
    virtual BaseThing* CreateShield(const rid_t& rid) = 0;
    //������� ������
    virtual BaseThing* CreateCamera(const rid_t& rid) = 0;
    //������� �������
    virtual BaseThing* CreateGrenade(const rid_t& rid) = 0;
    //������� �������
    virtual BaseThing* CreateScanner(const rid_t& rid) = 0;
    //c������ �������
    virtual BaseThing* CreateImplant(const rid_t& rid) = 0;
    //������� ������� (���� count == 0 ==> ��� ������ ������)
    virtual BaseThing* CreateAmmo(const rid_t& rid, int cout) = 0;
    //������� ������� (count == 0 ==> � ������� ������� �����)
    virtual BaseThing* CreateMedikit(const rid_t& rid, int charge) = 0;
    //������� ������ (ammo_count != 0 ==> � ������ ��������� �������)
    virtual BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count) = 0;

protected:
    //������ �� ���������, �� ��� ����� �����������
    ThingFactory(){}
};

//
// ����� ��� ����������� ������ � ������� ��������
//
class ThingScriptParser
{
public:

    //
    // ����� - ���������� ��������� 
    //
    class Acceptor{
    public:
        
        virtual ~Acceptor(){}
        
        // ������� �������
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // �������:
        //            true  - ������� ������
        //            false - ������� �� ������
        //
        virtual bool AcceptThing(BaseThing* thing) = 0;

        //������� ��������� �� �����������
        virtual void AcceptOrgInfo(const rid_t rid) = 0;
        //������� ���������� � ���������
        virtual void AcceptEntInfo(entity_type type, const rid_t& rid) = 0;
        //������� ���������� � ���������
        virtual void AcceptThingInfo(thing_type type, const rid_t& rid) = 0;
    };

    ThingScriptParser(Acceptor* acceptor = 0, ThingFactory* factory = 0);
    ~ThingScriptParser();

    //������ ���������� �������
    int GetLastPos();
    //���������� ������� ���������
    void SetFactory(ThingFactory* factory);
    //���������� ���������� ���������
    void SetAcceptor(Acceptor* acceptor);

    // ��������� ������, �������� �������� ����������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:        
    //          true  - �� ���� ������
    //          false - ���� ������ 
    // NOTE:
    //       ���� ��������� �� ������� Acceptor == 0,  
    //       �� ���������� ������ �������� ������������ 
    //       ������� ��� ���������� ���������
    //
    bool Parse(const std::string& script);

private:

    enum token_type{
        T_KEY,          // key
        T_MONEY,        // money
        T_IMPLANT,      // implant
        T_CAMERA,       // camera
        T_SHIELD,       // shield
        T_WEAPON,       // weapon
        T_AMMO,         // ammo
        T_ARMOR,        // armor
        T_SCANNER,      // scanner
        T_GRENADE,      // grenade
        T_MEDIKIT,      // medic

        T_INFO,         // info
        T_HUMAN,        // human
        T_TRADER,       // trader
        T_VEHICLE,      // vehicle
        T_ORGANIZATION, // organization

        T_LBRACKET,     // (
        T_RBRACKET,     // )
        T_COMMA,        // ,

        T_NUMBER,
        T_IDENT,
        T_END, 
    };

    token_type GetToken(std::string* str, int* num); 

    BaseThing* ParseKey();
    BaseThing* ParseAmmo();
    BaseThing* ParseMoney();
    BaseThing* ParseArmor();
    BaseThing* ParseWeapon();
    BaseThing* ParseCamera();
    BaseThing* ParseShield();
    BaseThing* ParseScanner();
    BaseThing* ParseImplant();
    BaseThing* ParseGrenade();
    BaseThing* ParseMedikit();

    void ParseInfo();

private:

    class LexerError{};
    class ParseError{};

    Acceptor*     m_acceptor;
    ThingFactory* m_factory; 

    std::istringstream m_input; 
};

//
// ������ ������� ��� ��������� ������ ���������
//
class ItemSetParser{
public:

    //
    // ����� - ���������� ��������� 
    //
    class Acceptor{
    public:
        
        virtual ~Acceptor(){}
        
        // ������� �������
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // �������:
        //            true  - ������� ������
        //            false - ������� �� ������
        //
        virtual void Accept(const rid_t& item_set) = 0;
    };
    
    ItemSetParser(Acceptor* acceptor = 0);

    // ��������� ������, �������� ����� � ������ ������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:        
    //          true  - �� ���� ������
    //          false - ���� ������ 
    // NOTE:
    //       ���� ��������� �� ������� ThingScriptParser == 0,  
    //       �� ���������� ������ �������� ������������ ������� 
    //       ��� ���������� ���������
    //
    bool Parse(const std::string& script);
    
    //������ ������� ���������� ������������ �������
    int GetLastPos();
    //���������� ������ ������ ���������
    void SetAcceptor(Acceptor* acceptor);
    
private:
    
    enum token_type{
        
        T_LBRACKET, //(
        T_RBRACKET, //)
            
        T_RND,      //~  ������� ��������� ������� �����
        T_ADD,      //+  ������� ������ � ���� 
            
        T_NULL,     //none ������� �����
            
        T_NUMBER,
        T_IDENT,
        T_END,
    };
    
    typedef std::vector<rid_t> rid_vec_t;
    
    struct info_s{
        std::string m_ident;
        int         m_number;
    };
    
    token_type GetToken(info_s* info);
    
    struct node_s{
        token_type m_type;
        rid_vec_t  m_rids;
        
        std::vector<node_s> m_nodes;
    };
    
    void ParseBlock(node_s* node);
    void SpawnThings(node_s& node);
    void SpawnThings(const rid_t& rid);

private:
        
    class LexerErr{};
    class ParseErr{};
        
    std::istringstream m_input;
    Acceptor*          m_acceptor; 
};

//
// �������� ����������
//
class InfoAcceptor{
public:

    virtual ~InfoAcceptor(){}

    //������� ��������� �� �����������
    virtual void AcceptOrganization(const rid_t rid) = 0;
    //������� ���������� � ���������
    virtual void AcceptThing(thing_type type, const rid_t& rid) = 0;
    //������� ���������� � ���������
    virtual void AcceptEntity(entity_type type, const rid_t& rid) = 0;
};

namespace AIUtils{

    //��������� ������ �� item_sets.xls   
    bool GetItemSetStr(const rid_t& rid, std::string* set_str, int* line);
}

#endif // _PUNCH_THINGFACTORY_H_