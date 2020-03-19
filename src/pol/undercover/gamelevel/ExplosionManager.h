#ifndef _EXPLOSIONMANAGER_HEADER_
#define _EXPLOSIONMANAGER_HEADER_
//
//���������� � ���������    
//
struct hit_s{

	enum
	{          
		BW_DMG,   //basic weapon damage
		AW_DMG,   //additional weapon damage
		AA_DMG,   //additional ammo damage

		MAX_DMSGS,
	};

	enum damage_type
	{
		DT_NONE,
		DT_STRIKE,
		DT_SHOCK,
		DT_ENERGY,
		DT_ELECTRIC,
		DT_EXPLOSIVE,
		DT_FLAME,
	};

	struct damage
	{
		damage_type Type;
		float       Value;
	};

	point3   m_to;				//����
	point3   m_from;			//������
	float    m_radius;			//������ ������
	damage   m_dmg[MAX_DMSGS];	//������ �����������

	float m_fall_off;			//��������� ������
};

//
// ������ ����������� �������
//
class LogicDamageStrategy{
public:

	virtual ~LogicDamageStrategy(){}

	//����� ����������� ��������
	virtual void DamageEntity(int  entity, const hit_s& hit) = 0;
	virtual void DestroyObject(const std::string &ObjName) = 0;
	virtual void LightFire(const point3 &pos, float radius, float damage) = 0;
};

//
// ����� - ������������ �������� ����� � ����������� �� ���� ���������
//

class ExplosionManager{
public:
	virtual ~ExplosionManager(){}

	//����������� � ��������� � ������
	virtual void OnAirHit(int  actor, const hit_s& hit) = 0;
	//����������� � ��������� � �������
	virtual void OnLevelHit(int  actor, const hit_s& hit) = 0;
	//����������� � ��������� � ���
	virtual void OnShieldHit(int  actor, const hit_s& hit) = 0; 
	//����������� � ��������� � ��������
	virtual void OnEntityHit(int  actor, int  victim, const hit_s& hit) = 0;
	//����������� � ��������� � ������
	virtual void OnObjectHit(int  actor, const std::string& obj, const hit_s& hit) = 0;
	//burn
	virtual void BurnObject(const std::string &obj, float damage) = 0;

public:
	static ExplosionManager* Get(){return m_Instance;};
	void SetLogicDamageStrategy(LogicDamageStrategy *dm){m_EDManager=dm;};


protected:
	static ExplosionManager *m_Instance;
	LogicDamageStrategy *m_EDManager;
};
#endif