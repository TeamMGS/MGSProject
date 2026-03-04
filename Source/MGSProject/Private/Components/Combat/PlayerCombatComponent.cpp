/*
 * 파일명 : PlayerCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "Components/Combat/PlayerCombatComponent.h"

#include "GAS/MGSGameplayTags.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryWeaponTag = MGSGameplayTags::Weapon_Player_Primary;
	SecondaryWeaponTag = MGSGameplayTags::Weapon_Player_Secondary;
}

bool UPlayerCombatComponent::EquipPrimaryWeapon()
{
	return EquipWeaponByTag(PrimaryWeaponTag);
}

bool UPlayerCombatComponent::EquipSecondaryWeapon()
{
	return EquipWeaponByTag(SecondaryWeaponTag);
}


