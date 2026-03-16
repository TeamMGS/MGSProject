/*
 * 파일명 : EnemyCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Components/Combat/EnemyCombatComponent.h"

#include "GAS/MGSGameplayTags.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryWeaponTag = MGSGameplayTags::Weapon_Primary;
	SecondaryWeaponTag = MGSGameplayTags::Weapon_Secondary;
}

bool UEnemyCombatComponent::EquipPrimaryWeapon()
{
	return EquipWeaponByTag(PrimaryWeaponTag);
}

bool UEnemyCombatComponent::EquipSecondaryWeapon()
{
	return EquipWeaponByTag(SecondaryWeaponTag);
}
