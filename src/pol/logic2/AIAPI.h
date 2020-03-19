/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� API ��� ������������� ������� � ������� ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__AI_API_H__)
#define __AI_API_H__

class BaseEntity;
class WeaponThing;
class GrenadeThing;
class Activity;
class ActivityHolder;

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� (�����������)
//
/////////////////////////////////////////////////////////////////////////////////
class WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const = 0;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� ��� ������������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
class FixedWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� ��� ����������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
class PatrolWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� ��� ���������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
class AssaultWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ 
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class EnemyComparator
{
public:
	// �������� ������������ - id ��������, �� ��������� � ��������
	// ������������ ��������
	EnemyComparator(eid_t id) { m_id = id; }
	// ���������� true, ���� ������ ������� ������� (human&vehicle)
	virtual bool operator() (eid_t id1, eid_t id2) const;
protected:
	// ���������� �������� ��������� ��������� �������� (human&vehicle)
	virtual float getEntityDanger(eid_t entity) const;
private:
	eid_t m_id;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ (�����������)
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class HexComparator
{
public:
	// ����������� � id �������� � id �����
	HexComparator(eid_t im, eid_t enemy);// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const = 0;
	eid_t getIm() const { return m_im; }// (human&vehicle)
	eid_t getEnemy() const { return m_enemy; }// (human&vehicle)
	point3 getImPos() const { return m_im_pos; }// (human&vehicle)
	point3 getEnemyPos() const { return m_enemy_pos; }// (human&vehicle)
private:
	eid_t m_im;
	eid_t m_enemy;
	point3 m_im_pos;
	point3 m_enemy_pos;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class FixedHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	FixedHexComparator(eid_t im, eid_t enemy);
	// ���������� true, ���� ������ ����� �������
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ����������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class PatrolHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	PatrolHexComparator(eid_t im, eid_t enemy);
	// ���������� true, ���� ������ ����� �������
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ���������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class AssaultHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	AssaultHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� �������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class WalkingTechHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	WalkingTechHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� � �����
// � ������ ���������� ����� ��������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class NoLineOfFireHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	NoLineOfFireHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// �����, ��������������� API (������ ��������)
//
/////////////////////////////////////////////////////////////////////////////////
class AIAPI
{
public:
	// �������� ��������� �� ������������ ������ ������
	//static AIAPI * getInst();

	// ����������
	//~AIAPI();

	// ����� �� �������� �� ������� ����� (� �������� ������� �������� � �����������)
	static bool mayShoot(eid_t, eid_t);

	// �������� ������� �� ������������� ��������, ����������� � ��������
	static float getHealthPercent(eid_t id); // (human&vehicle)
	// �������� ������� �� ����������, ���������� � ��������
	static float getMovePointsPercent(eid_t id); // (human&vehicle)
	// �������� ���������� ����������, ���������� � ��������
	static int getMovePoints(eid_t id); // (human&vehicle)
	// �������� ���������� �����, ���������� � ��������
	static int getSteps(eid_t id); // (human&vehicle)
	// �������� ���������� �����, ������� ����� ������� ��������
	// � �������� ����������� ����������
	static int getSteps(eid_t id, int movepoints); // (human&vehicle)

	// ���������� ��� �������� � �������� (����������� ����������� aim,
	// � snap ����� ��������)
	static void setShootType(eid_t id, float aim, float snap); // (human only)

	// ����� � ���� ������ ���������� ������ (���� ���������� �����
	// ���, ������ false) �� ���� ���� ����������� ����� ����� ����������
	// ���������� ��������� � ������ ������ - ������������
	// ������ �������� - ����� ��������� ���� �����
	static bool takeBestWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// ����� � ���� ������� (���� ��� - ������ false)
	static bool takeGrenade(eid_t id); // (human only)
	// ����� � ���� ���� (���� ��� - ������ false)
	static bool takeShield(eid_t id); // (human only)
	// ���� �� � ������� ���������� ������
	static bool haveTechWeapon(eid_t id); // (vehicle only)
	// �������� ���������� ����� ������������� ���� � ��������
	static int getThingCount(eid_t id, thing_type tt); // (human only)

	// �������� ���������� ���������� ����������� �� �������
	// ������ 0 ���� ��� ������
	static int getShootMovePoints(eid_t id, shot_type st); // (human&vehicle)

	// ������ �������
	typedef std::set<eid_t> entity_list_t;

	// ������ �������
	typedef std::vector<eid_t> entity_vector_t;

	static bool canHurt(BaseEntity *aggressor, BaseEntity *victim);

	// �������� ���� ������� ������ ������
	// ���������� true ���� ���� ���� �� ���� ����
	// (human&vehicle)
	static bool getEnemiesVisibleByEntity(eid_t id, entity_list_t* enemies);

	// �������� ���� ������� ������ �������
	// ���������� true ���� ���� ���� �� ����� ��������
	// ��������� �������� - ��� �������
	// (���� 0 - � ����� �������)
	// (human&vehicle)
	static bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et);

	// �������� ���� ������� ������ �������
	// ���������� true ���� ���� ���� �� ����� ��������
	// ������������� �������� - ��� �������
	// ��������� �������� - ����, � ������� ������ ���������� ��������
	// (���� 0 - � ����� �������)
	// (human&vehicle)
	static bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et, const pnt_vec_t& field);

	// �������� ���� ������� ����������� ������
	// ���������� true ���� ���� ���� �� ���� ����
	// (human&vehicle)
	static bool getEnemiesVisibleBySubteam(const std::string& subteam, entity_list_t* enemies);

	// ��������� ���� �� � ������ ��������
	// (human&vehicle)
	static bool isEntityVisible(eid_t im, eid_t entity);

	// �������� id ������ �������� �� ������� ������
	// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
	// ���������� 0 ���� ��� ������� ������
	// (human&vehicle)
	static eid_t getMostDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// �������� id ������ ���������� �� ������� ������
	// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
	// ���������� 0 ���� ��� ������� ������
	// (human&vehicle)
	static eid_t getLeastDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// �������� id ���������� �������� �� ������
	// ���������� 0 ���� ��� �������
	// (human&vehicle)
	static eid_t getRandomEntity(const entity_vector_t& entities);

	// ��������� ���� �� � ������� ��������� ����� ������ ��� �������
	// ���� ���� - ���������� true � ��������� �����
	static bool getWeaponLocation(eid_t id, ipnt2_t *pnt); // (human&vehicle)
	// ��������� ���� �� � ������� ��������� ����� ���� ���������������� ����
	// ���� ���� - ���������� true � ��������� �����
	static bool getThingLocation(eid_t id, ipnt2_t *pnt, int type); // (human&vehicle)
	// ������� ��� �������, ������� ����� � ������
	static void pickupAllNearAmmo(eid_t id); // (human only)
	// ������� ��� �������, ������� ����� � ������
	static void pickupAllNearMedikit(eid_t id); // (human only)
	// ������� ��� ������, ������� ����� � ������
	static void pickupAllNearWeapon(eid_t id); // (human only)
	// �������� ��� ������ ������ � �������� �� ����� ���� ������ �����
	static void dropUselessWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// ������� ������ ���������� �����, ������� ����� � ������
	// ���������� true ���� ������� ������� ���-������
	static bool pickupNearWeapon(eid_t id); // (human only)

	// ��������� �������� �����
	static void setSitPose(eid_t id); // (human only)
	// ��������� �������� ������
	static void setStandPose(eid_t id); // (human only)
	// ����������� �� ���
	static void setRun(eid_t id); // (human only)
	// ����������� �� ������
	static void setWalk(eid_t id); // (human only)
	// ���������, ����� �� �������
	static bool isSitPose(eid_t id); // (human only)
	// ���������, ����� �� �������
	static bool isStandPose(eid_t id); // (human only)

	// ���������, �������� �� ������� ������������ ����������
	// (human&vehicle)
	static bool isSafeShot(eid_t id, WeaponThing* weapon, const point3& target);
	// ���������, �������� �� ������ ������� ������������ ����������
	// (human only)
	static bool isSafeThrow(eid_t id, GrenadeThing* grenade, const point3& target);
	// ���������� �� �������� ���� ��� ��������
	// (human only)
	static bool shoot(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	static bool shoot(eid_t im, eid_t enemy, ActivityHolder *activity, std::string* reason);
	// (vehicle only)
	static bool shootByVehicle(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	static bool shootByVehicle(eid_t im, const point3& target, Activity** activity, std::string* reason);
	// ���� ��������� �������� ����� ����� ���� �����
	// (human only)
	static bool canAttackByGrenadeMultipleTarget(eid_t im, eid_t enemy, const entity_list_t& enemies);
	// ���������� � ��������� �����, ���� ��� ��������
	// (human only)
	static bool shootRandom(eid_t im, Activity** activity);
	static bool shootRandom(eid_t im, ActivityHolder *activity);
	// ������ ���� ������ �����, ���� ��� ��������
	// (human only)
	static bool throwShield(eid_t im, eid_t enemy, Activity** activity);
	static bool throwShield(eid_t im, eid_t enemy, ActivityHolder* activity);

	// �������� ������ ������� ������, ����������� �� ���������� �� �����
	// ������� �� ����� �����������
	// (human&vehicle)
	static void getBaseField(const ipnt2_t &center, int radius, pnt_vec_t *vector);
	// �������� �� ������� ������ ������ ������, � ������� ����� �����
	// (human&vehicle)
	static void getReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector, bool busaware = false);
	// �������� �� ������� ������ ������ ������, � ������� ����� �������� � ������
	// (human&vehicle)
	static void getPanicReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector);
	// �������� ��������� ���� �� ������� ������
	static ipnt2_t getRandomPoint(const pnt_vec_t &vector);// (human&vehicle)
	// �������� ������ ���� �� ������� ������
	// (human&vehicle)
	static ipnt2_t getBestPoint(const pnt_vec_t &vector, const HexComparator& hc);
	// ���������, ��������� �� �������� � ������� ����
	static bool isEntityInBaseField(eid_t id, const pnt_vec_t &vector);// (human&vehicle)
	// �������� ������� �������� �� ���������
	static ipnt2_t getPos2(eid_t id); // (human&vehicle)
	static ipnt2_t getPos2(BaseEntity* be); // (human&vehicle)
	// �������� ���������� ������� ��������
	static point3 getPos3(eid_t id); // (human&vehicle)
	// �������� ����, �� ������� ��������� ��������
	static float getAngle(eid_t id); // (human&vehicle)

	// ���� �� � �������� �������
	static bool haveMedikit(eid_t id); // (human only)
	// ����� � ���� �������
	static void takeMedikit(eid_t id); // (human only)
	// ������� �����
	static void cure(eid_t doctor, eid_t sick); // (human only)

	// �������� ���������� ���������� �� ��������
	static point3 convertPos2ToPos3(const ipnt2_t& pnt);
	// �������� ��������� �� ��������
	static BaseEntity* getPtr(eid_t id);
	// ������� � ������� ���������
	static void print(eid_t id, const std::string& str);

private:
	
	// ���������, �������� �� ����� � ���� � ��� ������ ������ - �������� ��
	static bool isWeaponLoaded(WeaponThing* wth); // (human&vehicle)
	// �������� ������ ���� ��� ����� � �������� (���������� false ����
	// ������ �������� ������������)
	static bool reloadWeapon(eid_t id, WeaponThing* wth); // (human only)

	// ������ ����������� ����� ����� �� ��� ������� ������ ������ ���
	//AIAPI();
};

#endif // __AI_API_H__