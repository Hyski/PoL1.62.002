#pragma once

namespace PoL
{

//=====================================================================================//
//                                     struct Inv                                      //
//=====================================================================================//
struct Inv
{
	static int DexterityMax;				/// ������������ ��������
	static int StrengthPlusDexterityMax;	/// ������������ ���� + ��������
	static int StrengthPlusWisdomMax;		/// ������������ ���� + ���������
	static int StrengthPlusAccuracyMax;		/// ������������ ���� + ��������
	static int AccuracyPlusMechanicsMax;	/// ������������ �������� + ��������
	static float TurnAngle4OneMp;			/// ������������ ���� �������� �� ������� �������� 1 mp
	static int MpsForPoseChange;			/// ���������� mp �� ����� ���� �� ������� � ������� � �������
	static float HexCostFactor1;			/// ���������� mp �� ���� ���� ������ = 
	static float HexCostFactor2;			///   hex_cost_factor1 * (hex_cost_factor2 - dexterity/hex_cost_factor3)
	static float HexCostFactor3;			///
	static float HexCostRunFactor;			/// � ����� ���������� ��� ������ �������� ���������� ��� ����
	static float HexCostCrouchFactor;		/// � ����� ���������� ��� ������ �������� ���������� ��� ��������
	static float MaxWeightFactor;			/// max_weight = strength / MaxWeightFactor
	static float MpsForTurnStrengthFactor;	/// mps_for_turn = MpsForTurnStrengthFactor*strength
	static float MpsForTurnWeightFactor;	///                - MpsForTurnWeightFactor*weight
	static float AccuracyDistanceFactor;	/// accuracy = ACF * (AUF * unit_accuracy + AAF * fall_off * (dist*ADF)^ADP / weapon_accuracy)
	static float AccuracyCommonFactor;		///
	static float AccuracyUnitFactor;		///
	static float AccuracyAmmoFactor;		///
	static float AccuracyDistancePower;		///
	static float AccuracyWeaponValue;		///
	static float AccuracyWeaponFactor;		///
	static float AccuracySitFactor;			/// accuracy = accuracy + accuracy * ASF
	static float SnapshotAccuracyFactor;	/// �������� ��� snapshot accuracy = accuracy / snapshot_accuracy_factor
	static float AutoshotAccuracyFactor;	/// �������� ��� autoshot accuracy = accuracy / autoshot_accuracy_factor
	static float AccuracyFallFactor;		/// �������� ������ ��� ���� accuracy = accuracy - AccuracyFallFactor * dist
	static int AccuracyPerTurn;				/// ���������� �������� �� ���
	static int DexterityPerTurn;			/// ���������� �������� �� ���
	static float HeadHitPrb;				/// ����������� ��������� � ������
	static float HandsHitPrb;				/// ����������� ��������� � ����
	static float LegsHitPrb;				/// ����������� ��������� � ����
	static float BodyHitPrb;				/// ����������� ��������� � ����
	static float DamageFallOffFactor;		/// ����������� damage = (basic_damage - dist*fall_off/damage_fall_off_factor)*armor
	static int FriendlyDeathMoraleFactor;	/// ������� ������ ��� �������� ����� ����� friendly_death_morale_factor*(1 - fibre)
	static int MoraleIncreaseFactor;		/// ���� ������ � ��� ����� morale_increase_factor * fibre
	static float ReactPrbFactor;			/// ����������� ������������ ������� (reaction1 - reaction2)/react_prb_factor
	static float PickUpMpsPerKg;			/// Mp �� ���� ���������, �������� � �����
	static float SnapshotMpsFactor;			/// ���������� mp �� snapshot ����� aimshot_mps / snapshot_mps_factor
	static float AutoshotMpsFactor;			/// ���������� mp �� autoshot ����� aimshot_mps / autoshot_mps_factor
	static int GrenadeMaxBounce;			/// ������������ ���������� �������� �������
	static float GrenadeThrowFactor1;		/// ������ �������� ������� ����� r = (GTF1 - GTF2*(dexterity-GTF3))*dist/GTF4
	static float GrenadeThrowFactor2;		///
	static float GrenadeThrowFactor3;		///
	static float GrenadeThrowFactor4;		///
	static float GrenadeAccuracyFactor;		/// �������� ������ ������� ����� GAF / dist
	static float GrenadeDistanceFactor;		/// ������������ ��������� ������ ������� ����� strength / GDF
	static float AccuracyMinScatter;		/// �������� ��������, ������ �������� �������� ��������� ������ 100%
	static float AccuracyMaxScatter;		/// �������� ��������, ������ �������� �������� ��������� �� ������� �������
	static int MovepointsForUse;			/// ���������� ����������, ���������� �� �� �������
	static int MovepointsForShipment;		/// ���������� ����������, ���������� �� ������� � �������
	static int MovepointsForLanding;		/// ���������� ����������, ���������� �� ������� �� �������
	static float ExplosionDamageBase;		/// ���� ����������� ������ EDB + EDF*max_health, �� ������ ��������� �� �����
	static float ExplosionDamageFactor;		///
	static int MovepointsForSwap;			/// ���������� ���������� �� ���� �������
	static bool AllVisibleWhenMovesUnlimited;	/// ��� ����� � ������ �������������� �����
	static bool Money;						/// �������� �� ������
	static int StartMoney;					/// ��������� ������
	static bool Blood;						/// ���� �� ����� � ����

	static void init();
};

}