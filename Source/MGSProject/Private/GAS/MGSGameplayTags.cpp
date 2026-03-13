/*
 * 파일명 : MGSGameplayTags.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "GAS/MGSGameplayTags.h"

namespace MGSGameplayTags
{
	// Input
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Jump, "InputTag.Jump")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Sprint, "InputTag.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Walk, "InputTag.Walk")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Strafe, "InputTag.Strafe")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Aim, "InputTag.Aim")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Fire, "InputTag.Fire")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Reload, "InputTag.Reload")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_PickupWeapon, "InputTag.PickupWeapon")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Primary, "InputTag.Weapon.Primary")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Secondary, "InputTag.Weapon.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Throwable, "InputTag.Weapon.Throwable")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Unequip, "InputTag.Weapon.Unequip")

	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Jump, "Ability.Player.Jump")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Sprint, "Ability.Player.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Walk, "Ability.Player.Walk")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Crouch, "Ability.Player.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Aim, "Ability.Player.Aim")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Fire, "Ability.Player.Fire")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Reload, "Ability.Player.Reload")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Interact, "Ability.Player.Interact")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_PickupWeapon, "Ability.Player.PickupWeapon")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Primary, "Ability.Player.Equip.Primary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Secondary, "Ability.Player.Equip.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Throwable, "Ability.Player.Equip.Throwable")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Equip_Primary, "Ability.Enemy.Equip.Primary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Equip_Secondary, "Ability.Enemy.Equip.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_DefaultMovement, "Ability.Enemy.DefaultMovement")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Walk, "Ability.Enemy.Walk")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Sprint, "Ability.Enemy.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Crouch, "Ability.Enemy.Crouch")

	// Weapon
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Primary, "Weapon.Primary")
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Secondary, "Weapon.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Throwable, "Weapon.Throwable")

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Sprint, "State.Player.Movement.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Run, "State.Player.Movement.Run")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Walk, "State.Player.Movement.Walk")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Falling, "State.Player.Movement.Falling")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_WantsToSprint, "State.Player.Movement.WantsToSprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Idle, "State.Player.Movement.Idle")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Moving, "State.Player.Movement.Moving")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_TurningInPlace, "State.Player.Movement.TurningInPlace")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Pivoting, "State.Player.Movement.Pivoting")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Starting, "State.Player.Movement.Starting")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_FromTraversal, "State.Player.Movement.FromTraversal")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_SpinTransition, "State.Player.Movement.SpinTransition")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_JustLanded_Light, "State.Player.JustLanded.Light")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_JustLanded_Heavy, "State.Player.JustLanded.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Crouching, "State.Player.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Aiming, "State.Player.Aiming")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_CombatReady, "State.Player.CombatReady")

	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Clear, "State.Enemy.Clear")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Suspicious, "State.Enemy.Suspicious")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Investigation, "State.Enemy.Investigation")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Combat, "State.Enemy.Combat")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Crouching, "State.Enemy.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Movement_Default, "State.Enemy.Movement.Default")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Movement_Walk, "State.Enemy.Movement.Walk")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Movement_Sprint, "State.Enemy.Movement.Sprint")

	UE_DEFINE_GAMEPLAY_TAG(State_Player_Gait_Walk, "State.Player.Gait.Walk")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Gait_Run, "State.Player.Gait.Run")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Gait_Sprint, "State.Player.Gait.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Stance_Stand, "State.Player.Stance.Stand")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Stance_Crouch, "State.Player.Stance.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Mode_OnGround, "State.Player.Mode.OnGround")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Mode_InAir, "State.Player.Mode.InAir")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_LOD_Dense, "State.Player.LOD.Dense")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_LOD_Sparse, "State.Player.LOD.Sparse")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_LOD_ExtremeSparse, "State.Player.LOD.ExtremeSparse")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Character_WeaponEquipped_Secondary, "State.Character.WeaponEquipped.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(State_Character_WeaponEquipped_Primary, "State.Character.WeaponEquipped.Primary")
	
	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Interact, "Event.Player.Interact")

	// SetByCaller data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage")
}
