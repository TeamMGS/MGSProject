/*
 * 파일명 : EnemyEquipPrimaryWeaponGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-12
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/Enemy/EnemyEquipPrimaryWeaponGameplayAbility.h"

#include "Components/Combat/EnemyCombatComponent.h"
#include "GAS/MGSGameplayTags.h"

UEnemyEquipPrimaryWeaponGameplayAbility::UEnemyEquipPrimaryWeaponGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Enemy_Equip_Primary);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnGiven;
	bClearAbilityOnEndWhenGiven = false;
}

void UEnemyEquipPrimaryWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bool bEquipSuccess = false;
	if (UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponentFromActorInfo())
	{
		bEquipSuccess = EnemyCombatComponent->EquipPrimaryWeapon();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bEquipSuccess);
}
