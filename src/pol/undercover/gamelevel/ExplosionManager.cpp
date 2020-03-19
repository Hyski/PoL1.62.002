#include "precomp.h"
#include "ExplosionManager.h"

#include "../iworld.h"
#include "../character.h"
#include "gamelevel.h"
#include "../GameLevel/DynObjectPool.h"

ExplosionManager *ExplosionManager::m_Instance;

class ParadiseExpMan:public ExplosionManager
{
public:
	ParadiseExpMan() {m_Instance=this;};
	~ParadiseExpMan() {};

	//����������� � ��������� � ������
	void OnAirHit(int  actor, const hit_s& hit);
	//����������� � ��������� � �������
	void OnLevelHit(int  actor, const hit_s& hit);
	//����������� � ��������� � ���
	void OnShieldHit(int  actor, const hit_s& hit); 
	//����������� � ��������� � ��������
	void OnEntityHit(int  actor, int  victim, const hit_s& hit);
	//����������� � ��������� � ������
	void OnObjectHit(int  actor, const std::string& obj, const hit_s& hit);
	//burn
	void BurnObject(const std::string &obj, float damage);
private:
	void CommonExp(int  actor, const std::string& obj, const hit_s& hit);
};

ParadiseExpMan ExplosionManagerInstance;//������������ ���������

//����������� � ��������� � ������
void ParadiseExpMan::OnAirHit(int  actor, const hit_s& hit)
{
	CommonExp(actor,"",hit);
}
//����������� � ��������� � �������
void ParadiseExpMan::OnLevelHit(int  actor, const hit_s& hit)
{
	CommonExp(actor,"",hit);
}
//����������� � ��������� � ���
void ParadiseExpMan::OnShieldHit(int  actor, const hit_s& hit)
{
	CommonExp(actor,"",hit);
} 
//����������� � ��������� � ��������
void ParadiseExpMan::OnEntityHit(int  actor, int  /*victim*/, const hit_s& hit) 
{
	CommonExp(actor,"",hit);
}
//����������� � ��������� � ������
void ParadiseExpMan::OnObjectHit(int  actor, const std::string& obj, const hit_s& hit)
{
	CommonExp(actor,obj,hit);
}

void ParadiseExpMan::BurnObject(const std::string &obj, float damage)
{
	DynObjectPool *ObjPool = IWorld::Get()->GetLevel()->GetLevelObjects();
	hit_s hit;
	hit.m_to=NULLVEC;            //����
	hit.m_from=NULLVEC;          //������
	hit.m_radius=0;        //������ ������
	hit.m_dmg[0].Type=hit_s::DT_FLAME; //������ �����������
	hit.m_dmg[0].Value=damage; //������ �����������
	hit.m_dmg[1].Type=hit_s::DT_NONE; //������ �����������
	hit.m_dmg[1].Value=0; //������ �����������
	hit.m_dmg[2].Type=hit_s::DT_NONE; //������ �����������
	hit.m_dmg[2].Value=0; //������ �����������
	ObjPool->DoExplosion(obj,hit,m_EDManager);

}

//=====================================================================================//
//                          void ParadiseExpMan::CommonExp()                           //
//=====================================================================================//
void ParadiseExpMan::CommonExp(int  actor, const std::string& obj, const hit_s& hit)
{
	DynObjectPool *ObjPool = IWorld::Get()->GetLevel()->GetLevelObjects();
	CharacterPool *CharPool=IWorld::Get()->GetCharacterPool();
	//������� ����� � �������
	CharPool->DoExplosion(actor, hit,m_EDManager);
	//������� �������
	ObjPool->DoExplosion(obj,hit,m_EDManager);
	//m_EDManager->LightFire(hit.m_to, hit.m_radius, 15);


}


