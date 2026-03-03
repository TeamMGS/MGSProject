/*
 * 파일명 : MGSGameplayTags.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "NativeGameplayTags.h"

namespace MGSGameplayTags
{
	// Input tags: Enhanced Input -> GAS bridge
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Aim)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Fire)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact)

	// Ability tags: identify gameplay abilities
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Crouch)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Aim)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Fire)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Interact)

	// State tags: runtime avatar state
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Crouching)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Aiming)
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_CombatReady)

	// Event tags: gameplay event payload trigger
	MGSPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Interact)
}
