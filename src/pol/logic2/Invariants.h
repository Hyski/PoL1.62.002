#pragma once

namespace PoL
{

//=====================================================================================//
//                                     struct Inv                                      //
//=====================================================================================//
struct Inv
{
	static int DexterityMax;				/// Максимальная ловкость
	static int StrengthPlusDexterityMax;	/// Максимальная сила + ловкость
	static int StrengthPlusWisdomMax;		/// Максимальная сила + интеллект
	static int StrengthPlusAccuracyMax;		/// Максимальная сила + меткость
	static int AccuracyPlusMechanicsMax;	/// Максимальная меткость + вождение
	static float TurnAngle4OneMp;			/// Максимальный угол поворота на который тратится 1 mp
	static int MpsForPoseChange;			/// Количество mp на смену позы из сидячей в стоячую и обратно
	static float HexCostFactor1;			/// Количество mp на один хекс ходьбы = 
	static float HexCostFactor2;			///   hex_cost_factor1 * (hex_cost_factor2 - dexterity/hex_cost_factor3)
	static float HexCostFactor3;			///
	static float HexCostRunFactor;			/// В такое количество раз меньше тратится мувпойнтов при беге
	static float HexCostCrouchFactor;		/// В такое количество раз больше тратится мувпойнтов при ползании
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
	static float SnapshotAccuracyFactor;	/// Меткость при snapshot accuracy = accuracy / snapshot_accuracy_factor
	static float AutoshotAccuracyFactor;	/// Меткость при autoshot accuracy = accuracy / autoshot_accuracy_factor
	static float AccuracyFallFactor;		/// Меткость падает при беге accuracy = accuracy - AccuracyFallFactor * dist
	static int AccuracyPerTurn;				/// Увеличение меткости за ход
	static int DexterityPerTurn;			/// Увеличение ловкости за ход
	static float HeadHitPrb;				/// Вероятность попадания в голову
	static float HandsHitPrb;				/// Вероятность попадания в руки
	static float LegsHitPrb;				/// Вероятность попадания в ноги
	static float BodyHitPrb;				/// Вероятность попадания в тело
	static float DamageFallOffFactor;		/// Повреждение damage = (basic_damage - dist*fall_off/damage_fall_off_factor)*armor
	static int FriendlyDeathMoraleFactor;	/// Падение морали при убийстве друга равно friendly_death_morale_factor*(1 - fibre)
	static int MoraleIncreaseFactor;		/// Рост морали в тур равно morale_increase_factor * fibre
	static float ReactPrbFactor;			/// Вероятность срабатывания реакции (reaction1 - reaction2)/react_prb_factor
	static float PickUpMpsPerKg;			/// Mp за один килограмм, поднятый с земли
	static float SnapshotMpsFactor;			/// Количество mp на snapshot равно aimshot_mps / snapshot_mps_factor
	static float AutoshotMpsFactor;			/// Количество mp на autoshot равно aimshot_mps / autoshot_mps_factor
	static int GrenadeMaxBounce;			/// Максимальное количество отскоков гранаты
	static float GrenadeThrowFactor1;		/// Радиус разброса гранаты равен r = (GTF1 - GTF2*(dexterity-GTF3))*dist/GTF4
	static float GrenadeThrowFactor2;		///
	static float GrenadeThrowFactor3;		///
	static float GrenadeThrowFactor4;		///
	static float GrenadeAccuracyFactor;		/// Точность броска гранаты равна GAF / dist
	static float GrenadeDistanceFactor;		/// Максимальная дальность броска гранаты равна strength / GDF
	static float AccuracyMinScatter;		/// Значение разброса, меньше которого точность считается равной 100%
	static float AccuracyMaxScatter;		/// Значение разброса, больше которого точность считается по обычной формуле
	static int MovepointsForUse;			/// Количество мувпойнтов, тратящихся на юз объекта
	static int MovepointsForShipment;		/// Количество мувпойнтов, тратящихся на посадку в технику
	static int MovepointsForLanding;		/// Количество мувпойнтов, тратящихся на высадку из техники
	static float ExplosionDamageBase;		/// Если повреждение больше EDB + EDF*max_health, то чувака разрывает на части
	static float ExplosionDamageFactor;		///
	static int MovepointsForSwap;			/// Количество мувпойнтов на своп чуваков
	static bool AllVisibleWhenMovesUnlimited;	/// Все видны в режиме неограниченных ходов
	static bool Money;						/// Включены ли деньги
	static int StartMoney;					/// Начальные деньги
	static bool Blood;						/// Есть ли кровь в игре

	static void init();
};

}