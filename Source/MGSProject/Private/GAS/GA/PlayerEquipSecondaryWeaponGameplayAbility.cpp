/*
 * 파일명 : PlayerEquipSecondaryWeaponGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/PlayerEquipSecondaryWeaponGameplayAbility.h"

#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"

UPlayerEquipSecondaryWeaponGameplayAbility::UPlayerEquipSecondaryWeaponGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Equip_Secondary);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

void UPlayerEquipSecondaryWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		// 2번 키는 보조무기 슬롯 장착
		bEquipSuccess = PlayerCombatComponent->EquipSecondaryWeapon();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bEquipSuccess);
}
