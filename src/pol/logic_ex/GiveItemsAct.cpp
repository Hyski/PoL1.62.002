#include "precomp.h"
#include "QuestContext.h"
#include "GiveItemsAct.h"
#include "QuestReporter.h"
#include <logic2/Thing.h>
#include <logic2/AIUtils.h>
#include <undercover/interface/Screens.h>
#include <logic2/ThingFactory.h>
#include <undercover/interface/SlotsEngine.h>
#include <undercover/interface/InventoryScreen.h>

namespace PoL
{

namespace GiveItemsActDetails
{

//=====================================================================================//
//                                  class ThingsGiver                                  //
//=====================================================================================//
class ThingsGiver : public ::ThingScriptParser::Acceptor, public ::ThingFactory
{
	class Visitor : public EntityVisitor
	{
		typedef boost::function<void(HumanEntity*)> FnH_t;
		typedef boost::function<void(TraderEntity*)> FnT_t;
		typedef boost::function<void(VehicleEntity*)> FnV_t;

		FnH_t m_fnh;
		FnT_t m_fnt;
		FnV_t m_fnv;

	public:
		Visitor(FnH_t fnh) : m_fnh(fnh) {}
		Visitor(FnH_t fnh, FnT_t fnt, FnV_t fnv)
		:	m_fnh(fnh), m_fnt(fnt), m_fnv(fnv)
		{}

		void Visit(HumanEntity *human)
		{
			if(m_fnh) m_fnh(human);
		}
		void Visit(TraderEntity *trader)
		{
			if(m_fnt) m_fnt(trader);
		}
		void Visit(VehicleEntity *vehicle)
		{
			if(m_fnv) m_fnv(vehicle);
		}
	};

public:
    ThingsGiver(BaseEntity* actor)
	:	m_actor(actor)
	{
		m_actor->Accept(Visitor(boost::bind(init,this,_1)));
	}

    ~ThingsGiver()
	{
		m_actor->Accept(Visitor(boost::bind(shut,this,_1)));
	}

private:
	void init(HumanEntity *human)
	{
		SlotsEngine *engine  = Screens::Instance()->Inventory()->GetSlotsEngine();
		SESlot *slot = engine->SlotAt(InventoryScreen::S_BACKPACK);
		HumanContext *context = human->GetEntityContext();

        slot->Clear();

        //заполним рюкзак
        for(HumanContext::iterator thing_itor = context->begin(HPK_BACKPACK);
			thing_itor != context->end(); ++thing_itor)
		{
            SEItem* item = engine->CreateItem(thing_itor->GetInfo()->GetShader().c_str(),
											  thing_itor->GetInfo()->GetSize(), &*thing_itor);

            item->SetPosition(thing_itor->GetPakPos());
            
            if(!human->IsRaised(EA_WAS_IN_INV) && !slot->CanInsert(item))
			{
                throw CASUS("ThingGiver: не хватает места в рюкзаке для передмета!");                    
			}

            slot->Insert(item);
        }
	}

	void shut(HumanEntity *human)
	{
        SlotsEngine *engine = Screens::Instance()->Inventory()->GetSlotsEngine();
        SESlot *slot = engine->SlotAt(InventoryScreen::S_BACKPACK);

        //сохраним коорд. предметов в рюкзаке
        for(SESlot::Iterator slot_itor = slot->Begin(); slot_itor != slot->End(); ++slot_itor)
		{
            slot_itor->GetBaseThing()->SetPakPos(slot_itor->Position());
        }

        slot->Clear();
	}

	void giveThingToHuman(HumanEntity *human, BaseThing *thing)
	{
        SlotsEngine *engine = Screens::Instance()->Inventory()->GetSlotsEngine();
        SESlot *slot = engine->SlotAt(InventoryScreen::S_BACKPACK);
        HumanContext *context = human->GetEntityContext();

        SEItem* item = engine->CreateItem(thing->GetInfo()->GetShader().c_str(), thing->GetInfo()->GetSize(), thing);
        if(slot->CanInsert(item))
		{
            slot->Insert(item);
            context->InsertThing(HPK_BACKPACK, thing);
        }
		else
		{
            //бросим предмет на землю
            if(VehicleEntity* vehicle = context->GetCrew())
			{
                Depot::GetInst()->Push(vehicle->GetGraph()->GetPos2(), thing);
			}
            else
			{
                Depot::GetInst()->Push(human->GetGraph()->GetPos2(), thing);
			}

            delete item;
        }
	}

	void giveThingToTrader(TraderEntity *entity, BaseThing *thing)
	{
		entity->GetEntityContext()->InsertThing(thing);
	}

	void giveThingToVehicle(VehicleEntity *entity, BaseThing *thing)
	{
		Depot::GetInst()->Push(entity->GetGraph()->GetPos2(), thing); 
	}

	/// Принять вещь
    bool AcceptThing(BaseThing* thing)
    {
		m_actor->Accept(Visitor(
			boost::bind(giveThingToHuman,this,_1,thing),
			boost::bind(giveThingToTrader,this,_1,thing),
			boost::bind(giveThingToVehicle,this,_1,thing)
			));
        return true;
    }

	/// принять информацию об организации
	void AcceptOrgInfo(const rid_t rid)
	{
		if(m_actor->GetPlayer() == PT_PLAYER)
		{
			ForceHandbook::GetInst()->Push(0, rid, ForceHandbook::F_NEW_REC);
		}
	}

	/// принять информацию о существах
	void AcceptEntInfo(entity_type type, const rid_t& rid)
	{
		if(m_actor->GetPlayer() == PT_PLAYER)
		{
			EntityHandbook::GetInst()->Push(type, rid, EntityHandbook::F_NEW_REC);
		}
	}

	/// принять информацию о предметах
	void AcceptThingInfo(thing_type type, const rid_t& rid)
	{
		if(m_actor->GetPlayer() == PT_PLAYER)
		{
			ThingHandbook::GetInst()->Push(type, rid, ThingHandbook::F_NEW_REC);
		}
	}

private:
    BaseThing* CreateKey(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateKey(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateMoney(int sum)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateMoney(sum);
		QuestReporter::moneyAdded(sum);
		return result;
	}
    
    BaseThing* CreateArmor(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateArmor(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateShield(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateShield(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateCamera(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateCamera(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateGrenade(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateGrenade(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateScanner(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateScanner(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateImplant(const rid_t& rid)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateImplant(rid);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateAmmo(const rid_t& rid, int cout)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateAmmo(rid,cout);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateMedikit(const rid_t& rid, int charge)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateMedikit(rid,charge);
		QuestReporter::itemAdded(result);
		return result;
	}
    
    BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count)
    {
		BaseThing *result = ThingFactory::GetInst()->CreateWeapon (rid,ammo_rid,ammo_count);
		QuestReporter::itemAdded(result);
		return result;
	}

private:
    BaseEntity* m_actor;    
};

}

using namespace GiveItemsActDetails;

//=====================================================================================//
//                            GiveItemsAct::GiveItemsAct()                             //
//=====================================================================================//
GiveItemsAct::GiveItemsAct(const std::string &itset, const std::string &target)
:	m_started(false),
	m_ended(false),
	m_itemSet(itset),
	m_target(target)
{
}

//=====================================================================================//
//                             void GiveItemsAct::start()                              //
//=====================================================================================//
void GiveItemsAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void GiveItemsAct::update()                             //
//=====================================================================================//
void GiveItemsAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	BaseEntity *actor = 0;

	if(m_target.empty())
	{
		if(!QuestContext::getActor().empty() && !getEntityByName(QuestContext::getActor()))
		{
			actor = getEntityByName(QuestContext::getActor());
		}
	}
	else
	{
		actor = getEntityByName(m_target);
	}

	if(!actor)
	{
		actor = getPlayer();
		ACT_LOG("WARNING: There was no one to give items [" << m_itemSet << "]. Giving 'em to Player\n");
		if(!actor)
		{
			ACT_LOG("WARNING: Player not found. Items are not given to anyone\n");
		}
	}

	if(actor)
	{
		ThingsGiver giver(actor);
		parseThingScript(m_itemSet,ThingScriptParser(&giver,&giver));
	}

	m_ended = true;
}


}