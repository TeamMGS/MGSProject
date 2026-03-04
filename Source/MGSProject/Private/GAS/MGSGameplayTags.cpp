/*
 * 파일명 : MGSGameplayTags.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
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
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Aim, "InputTag.Aim")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Fire, "InputTag.Fire")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Reload, "InputTag.Reload")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Primary, "InputTag.Weapon.Primary")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Secondary, "InputTag.Weapon.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Throwable, "InputTag.Weapon.Throwable")

	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Jump, "Ability.Player.Jump")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Sprint, "Ability.Player.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Walk, "Ability.Player.Walk")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Crouch, "Ability.Player.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Aim, "Ability.Player.Aim")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Fire, "Ability.Player.Fire")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Reload, "Ability.Player.Reload")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Interact, "Ability.Player.Interact")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Primary, "Ability.Player.Equip.Primary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Secondary, "Ability.Player.Equip.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Equip_Throwable, "Ability.Player.Equip.Throwable")

	// Weapon
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Player_Primary, "Weapon.Player.Primary")
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Player_Secondary, "Weapon.Player.Secondary")
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Player_Throwable, "Weapon.Player.Throwable")

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Sprint, "State.Player.Movement.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Walk, "State.Player.Movement.Walk")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Movement_Falling, "State.Player.Movement.Falling")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Crouching, "State.Player.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Aiming, "State.Player.Aiming")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_CombatReady, "State.Player.CombatReady")

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Interact, "Event.Player.Interact")
}


