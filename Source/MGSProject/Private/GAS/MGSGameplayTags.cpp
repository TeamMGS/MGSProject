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
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Aim, "InputTag.Aim")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Fire, "InputTag.Fire")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact")

	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Jump, "Ability.Player.Jump")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Crouch, "Ability.Player.Crouch")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Aim, "Ability.Player.Aim")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Fire, "Ability.Player.Fire")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Interact, "Ability.Player.Interact")

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Crouching, "State.Player.Crouching")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_Aiming, "State.Player.Aiming")
	UE_DEFINE_GAMEPLAY_TAG(State_Player_CombatReady, "State.Player.CombatReady")

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Interact, "Event.Player.Interact")
}