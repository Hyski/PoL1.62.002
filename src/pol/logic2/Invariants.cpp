#include "logicdefs.h"
#include "Invariants.h"

namespace PoL
{

int Inv::DexterityMax = 100;					/// ������������ ��������
int Inv::StrengthPlusDexterityMax = 180;		/// ������������ ���� + ��������
int Inv::StrengthPlusWisdomMax = 200;			/// ������������ ���� + ���������
int Inv::StrengthPlusAccuracyMax = 160;			/// ������������ ���� + ��������
int Inv::AccuracyPlusMechanicsMax = 200;		/// ������������ �������� + ��������
float Inv::TurnAngle4OneMp = PId2;				/// ������������ ���� �������� �� ������� �������� 1 mp
int Inv::MpsForPoseChange = 7;					/// ���������� mp �� ����� ���� �� ������� � ������� � �������
float Inv::HexCostFactor1 = 10.0f;				/// ���������� mp �� ���� ���� ������ = 
float Inv::HexCostFactor2 = 1.0f;				///   hex_cost_factor1 * (hex_cost_factor2 - dexterity/hex_cost_factor3)
float Inv::HexCostFactor3 = 100.0f;				///
float Inv::HexCostRunFactor = 1.5f;				/// � ����� ���������� ��� ������ �������� ���������� ��� ����
float Inv::HexCostCrouchFactor = 2.0f;			/// � ����� ���������� ��� ������ �������� ���������� ��� ��������
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
float Inv::SnapshotAccuracyFactor = 2.0f;		/// �������� ��� snapshot accuracy = accuracy / snapshot_accuracy_factor
float Inv::AutoshotAccuracyFactor = 2.5f;		/// �������� ��� autoshot accuracy = accuracy / autoshot_accuracy_factor
float Inv::AccuracyFallFactor = 1.0f;			/// �������� ������ ��� ���� accuracy = accuracy - AccuracyFallFactor * dist
int Inv::AccuracyPerTurn = 20;					/// ���������� �������� �� ���
int Inv::DexterityPerTurn = 20;					/// ���������� �������� �� ���
float Inv::HeadHitPrb = 0.1f;					/// ����������� ��������� � ������
float Inv::HandsHitPrb = 0.2f;					/// ����������� ��������� � ����
float Inv::LegsHitPrb = 0.3f;					/// ����������� ��������� � ����
float Inv::BodyHitPrb = 0.5f;					/// ����������� ��������� � ����
float Inv::DamageFallOffFactor = 10.0f;			/// ����������� damage = (basic_damage - dist*fall_off/damage_fall_off_factor)*armor
int Inv::FriendlyDeathMoraleFactor = 40;		/// ������� ������ ��� �������� ����� ����� friendly_death_morale_factor*(1 - fibre)
int Inv::MoraleIncreaseFactor = 10;				/// ���� ������ � ��� ����� morale_increase_factor * fibre
float Inv::ReactPrbFactor = 25.0f;				/// ����������� ������������ ������� (reaction1 - reaction2)/react_prb_factor
float Inv::PickUpMpsPerKg = 1.0f;				/// Mp �� ���� ���������, �������� � �����
float Inv::SnapshotMpsFactor = 2.5f;			/// ���������� mp �� snapshot ����� aimshot_mps / snapshot_mps_factor
float Inv::AutoshotMpsFactor = 1.5f;			/// ���������� mp �� autoshot ����� aimshot_mps / autoshot_mps_factor
int Inv::GrenadeMaxBounce = 0;					/// ������������ ���������� �������� �������
float Inv::GrenadeThrowFactor1 = 7.0f;			/// ������ �������� ������� ����� r = (GTF1 - GTF2*(dexterity-GTF3))*dist/GTF4
float Inv::GrenadeThrowFactor2 = 0.07f;			///
float Inv::GrenadeThrowFactor3 = 10.0f;			///
float Inv::GrenadeThrowFactor4 = 10.0f;			///
float Inv::GrenadeAccuracyFactor = 100.0f;		/// �������� ������ ������� ����� GAF / dist
float Inv::GrenadeDistanceFactor = 2.0f;		/// ������������ ��������� ������ ������� ����� strength / GDF
float Inv::AccuracyMinScatter = 0.20f;			/// �������� ��������, ������ �������� �������� ��������� ������ 100%
float Inv::AccuracyMaxScatter = 0.45f;			/// �������� ��������, ������ �������� �������� ��������� �� ������� �������
int Inv::MovepointsForUse = 3;					/// ���������� ����������, ���������� �� �� �������
int Inv::MovepointsForShipment = 8;				/// ���������� ����������, ���������� �� ������� � �������
int Inv::MovepointsForLanding = 8;				/// ���������� ����������, ���������� �� ������� �� �������
float Inv::ExplosionDamageBase = 70.0f;			/// ���� ����������� ������ EDB + EDF*max_health, �� ������ ��������� �� �����
float Inv::ExplosionDamageFactor = 0.0f;		///
int Inv::MovepointsForSwap = 8;					/// ���������� ���������� �� ���� �������
bool Inv::AllVisibleWhenMovesUnlimited = false;	/// ��� ����� � ������ �������������� �����
bool Inv::Money = true;							/// �������� �� ������
int Inv::StartMoney = 100;						/// ��������� ������
bool Inv::Blood = false;						/// ���� �� ����� � ����

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