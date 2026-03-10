/*
 * 파일명: PlayerEquipPrimaryWeaponGameplayAbility.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#include "GAS/GA/PlayerEquipPrimaryWeaponGameplayAbility.h"

#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"


UPlayerEquipPrimaryWeaponGameplayAbility::UPlayerEquipPrimaryWeaponGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Equip_Primary);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

void UPlayerEquipPrimaryWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	if (UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo())
	{
		// 1번 키는 주무기 슬롯 장착
		bEquipSuccess = PlayerCombatComponent->EquipPrimaryWeapon();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bEquipSuccess);
}



