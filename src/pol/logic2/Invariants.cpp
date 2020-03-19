#include "logicdefs.h"
#include "Invariants.h"

namespace PoL
{

int Inv::DexterityMax = 100;					/// Максимальная ловкость
int Inv::StrengthPlusDexterityMax = 180;		/// Максимальная сила + ловкость
int Inv::StrengthPlusWisdomMax = 200;			/// Максимальная сила + интеллект
int Inv::StrengthPlusAccuracyMax = 160;			/// Максимальная сила + меткость
int Inv::AccuracyPlusMechanicsMax = 200;		/// Максимальная меткость + вождение
float Inv::TurnAngle4OneMp = PId2;				/// Максимальный угол поворота на который тратится 1 mp
int Inv::MpsForPoseChange = 7;					/// Количество mp на смену позы из сидячей в стоячую и обратно
float Inv::HexCostFactor1 = 10.0f;				/// Количество mp на один хекс ходьбы = 
float Inv::HexCostFactor2 = 1.0f;				///   hex_cost_factor1 * (hex_cost_factor2 - dexterity/hex_cost_factor3)
float Inv::HexCostFactor3 = 100.0f;				///
float Inv::HexCostRunFactor = 1.5f;				/// В такое количество раз меньше тратится мувпойнтов при беге
float Inv::HexCostCrouchFactor = 2.0f;			/// В такое количество раз больше тратится мувпойнтов при ползании
float Inv::MaxWeightFactor = 2.0f;				/// max_weight = strength / MaxWeightFactor
float Inv::MpsForTurnStrengthFactor = 2.0f;		/// mps_for_turn = MpsForTurnStrengthFactor*strength
float Inv::MpsForTurnWeightFactor = 1.0f;		///                - MpsForTurnWeightFactor*weight
float Inv::AccuracyDistanceFactor = 1.0f;		/// accuracy = ACF * (AUF * unit_accuracy + AAF * fall_off * (dist*ADF)^ADP / weapon_accuracy)
float Inv::AccuracyCommonFactor = 2.0f;			///
float Inv::AccuracyUnitFactor = 1.0f;			///
float Inv::AccuracyAmmoFactor = 1.0f;			///
float Inv::AccuracyDistancePower = 1.0f;		///
float Inv::AccuracyWeaponValue = 1.0f;			///
float Inv::AccuracyWeaponFactor = 100.0f;		///
float Inv::AccuracySitFactor = 0.25f;			/// accuracy = accuracy + accuracy * ASF
float Inv::SnapshotAccuracyFactor = 2.0f;		/// Меткость при snapshot accuracy = accuracy / snapshot_accuracy_factor
float Inv::AutoshotAccuracyFactor = 2.5f;		/// Меткость при autoshot accuracy = accuracy / autoshot_accuracy_factor
float Inv::AccuracyFallFactor = 1.0f;			/// Меткость падает при беге accuracy = accuracy - AccuracyFallFactor * dist
int Inv::AccuracyPerTurn = 20;					/// Увеличение меткости за ход
int Inv::DexterityPerTurn = 20;					/// Увеличение ловкости за ход
float Inv::HeadHitPrb = 0.1f;					/// Вероятность попадания в голову
float Inv::HandsHitPrb = 0.2f;					/// Вероятность попадания в руки
float Inv::LegsHitPrb = 0.3f;					/// Вероятность попадания в ноги
float Inv::BodyHitPrb = 0.5f;					/// Вероятность попадания в тело
float Inv::DamageFallOffFactor = 10.0f;			/// Повреждение damage = (basic_damage - dist*fall_off/damage_fall_off_factor)*armor
int Inv::FriendlyDeathMoraleFactor = 40;		/// Падение морали при убийстве друга равно friendly_death_morale_factor*(1 - fibre)
int Inv::MoraleIncreaseFactor = 10;				/// Рост морали в тур равно morale_increase_factor * fibre
float Inv::ReactPrbFactor = 25.0f;				/// Вероятность срабатывания реакции (reaction1 - reaction2)/react_prb_factor
float Inv::PickUpMpsPerKg = 1.0f;				/// Mp за один килограмм, поднятый с земли
float Inv::SnapshotMpsFactor = 2.5f;			/// Количество mp на snapshot равно aimshot_mps / snapshot_mps_factor
float Inv::AutoshotMpsFactor = 1.5f;			/// Количество mp на autoshot равно aimshot_mps / autoshot_mps_factor
int Inv::GrenadeMaxBounce = 0;					/// Максимальное количество отскоков гранаты
float Inv::GrenadeThrowFactor1 = 7.0f;			/// Радиус разброса гранаты равен r = (GTF1 - GTF2*(dexterity-GTF3))*dist/GTF4
float Inv::GrenadeThrowFactor2 = 0.07f;			///
float Inv::GrenadeThrowFactor3 = 10.0f;			///
float Inv::GrenadeThrowFactor4 = 10.0f;			///
float Inv::GrenadeAccuracyFactor = 100.0f;		/// Точность броска гранаты равна GAF / dist
float Inv::GrenadeDistanceFactor = 2.0f;		/// Максимальная дальность броска гранаты равна strength / GDF
float Inv::AccuracyMinScatter = 0.20f;			/// Значение разброса, меньше которого точность считается равной 100%
float Inv::AccuracyMaxScatter = 0.45f;			/// Значение разброса, больше которого точность считается по обычной формуле
int Inv::MovepointsForUse = 3;					/// Количество мувпойнтов, тратящихся на юз объекта
int Inv::MovepointsForShipment = 8;				/// Количество мувпойнтов, тратящихся на посадку в технику
int Inv::MovepointsForLanding = 8;				/// Количество мувпойнтов, тратящихся на высадку из техники
float Inv::ExplosionDamageBase = 70.0f;			/// Если повреждение больше EDB + EDF*max_health, то чувака разрывает на части
float Inv::ExplosionDamageFactor = 0.0f;		///
int Inv::MovepointsForSwap = 8;					/// Количество мувпойнтов на своп чуваков
bool Inv::AllVisibleWhenMovesUnlimited = false;	/// Все видны в режиме неограниченных ходов
bool Inv::Money = true;							/// Включены ли деньги
int Inv::StartMoney = 100;						/// Начальные деньги
bool Inv::Blood = false;						/// Есть ли кровь в игре

//=====================================================================================//
//                                  void Inv::init()                                   //
//=====================================================================================//
void Inv::init()
{
	PoL::VFileWpr vfile("scripts/pol/unit_inv.txt");
	if(vfile.get() && vfile.get()->Size())
	{
		using mll::utils::table;
		PoL::VFileBuf buf(vfile.get());
		std::istream in(&buf);
		table tbl;
		in >> tbl;

		for(int i = 0; i < tbl.height(); ++i)
		{
			if(tbl.col(0)[i] == "dexterity_max")
			{
				DexterityMax = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "strength+dexterity_max")
			{
				StrengthPlusDexterityMax = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "strength+wisdom_max")
			{
				StrengthPlusWisdomMax = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "strength+accuracy_max")
			{
				StrengthPlusAccuracyMax = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy+drive_max")
			{
				AccuracyPlusMechanicsMax = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "turn_angle_for_1mp")
			{
				TurnAngle4OneMp = boost::lexical_cast<float>(tbl.col(1)[i]);
				TurnAngle4OneMp *= PId2/180.0f;
			}
			else if(tbl.col(0)[i] == "mps_for_pose_change")
			{
				MpsForPoseChange = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hex_cost_factor1")
			{
				HexCostFactor1 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hex_cost_factor2")
			{
				HexCostFactor2 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hex_cost_factor3")
			{
				HexCostFactor3 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hex_cost_run_factor")
			{
				HexCostRunFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hex_cost_crouch_factor")
			{
				HexCostCrouchFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "max_weight_factor")
			{
				MaxWeightFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "mps_for_turn_strength_factor")
			{
				MpsForTurnStrengthFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "mps_for_turn_weight_factor")
			{
				MpsForTurnWeightFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_distance_factor")
			{
				AccuracyDistanceFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_common_factor")
			{
				AccuracyCommonFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_unit_factor")
			{
				AccuracyUnitFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_ammo_factor")
			{
				AccuracyAmmoFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_distance_power")
			{
				AccuracyDistancePower = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_weapon_value")
			{
				AccuracyWeaponValue = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_weapon_factor")
			{
				AccuracyWeaponFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_sit_factor")
			{
				AccuracySitFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "snapshot_accuracy_factor")
			{
				SnapshotAccuracyFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "autoshot_accuracy_factor")
			{
				AutoshotAccuracyFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_fall_factor")
			{
				AccuracyFallFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_per_turn")
			{
				AccuracyPerTurn = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "dexterity_per_turn")
			{
				DexterityPerTurn = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "head_hit_prb")
			{
				HeadHitPrb = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "hands_hit_prb")
			{
				HandsHitPrb = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "legs_hit_prb")
			{
				LegsHitPrb = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "body_hit_prb")
			{
				BodyHitPrb = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "damage_fall_off_factor")
			{
				DamageFallOffFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "friendly_death_morale_factor")
			{
				FriendlyDeathMoraleFactor = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "morale_increase_factor")
			{
				MoraleIncreaseFactor = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "react_prb_factor")
			{
				ReactPrbFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "pick_up_mps_per_kg")
			{
				PickUpMpsPerKg = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "snapshot_mps_factor")
			{
				SnapshotMpsFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "autoshot_mps_factor")
			{
				AutoshotMpsFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_max_bounce")
			{
				GrenadeMaxBounce = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_throw_factor1")
			{
				GrenadeThrowFactor1 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_throw_factor2")
			{
				GrenadeThrowFactor2 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_throw_factor3")
			{
				GrenadeThrowFactor3 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_throw_factor4")
			{
				GrenadeThrowFactor4 = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_accuracy_factor")
			{
				GrenadeAccuracyFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "grenade_distance_factor")
			{
				GrenadeDistanceFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_min_scatter")
			{
				AccuracyMinScatter = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "accuracy_max_scatter")
			{
				AccuracyMaxScatter = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "movepoints_for_use")
			{
				MovepointsForUse = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "movepoints_for_shipment")
			{
				MovepointsForShipment = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "movepoints_for_landing")
			{
				MovepointsForLanding = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "explosion_damage_base")
			{
				ExplosionDamageBase = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "explosion_damage_factor")
			{
				ExplosionDamageFactor = boost::lexical_cast<float>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "movepoints_for_swap")
			{
				MovepointsForSwap = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "all_visible_when_moves_unlimited")
			{
				AllVisibleWhenMovesUnlimited = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "money")
			{
				Money = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "start_money")
			{
				StartMoney = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
			else if(tbl.col(0)[i] == "blood")
			{
				Blood = boost::lexical_cast<int>(tbl.col(1)[i]);
			}
		}
	}
}

}