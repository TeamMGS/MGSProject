/*
 * 파일명 : EnemyReloadGameplayAbility.cpp
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#include "GAS/GA/Enemy/EnemyReloadGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseGun.h"

UEnemyReloadGameplayAbility::UEnemyReloadGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Enemy_Reload);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Enemy_Attacking);
}

bool UEnemyReloadGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UEnemyCombatComponent* EnemyCombatComponent = Cast<UEnemyCombatComponent>(GetPawnCombatComponentFromActorInfo());
	if (!EnemyCombatComponent)
	{
		return false;
	}

	const ABaseGun* EquippedGun = Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon());
	return EquippedGun && EquippedGun->CanReload();
}

void UEnemyReloadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = EnemyCombatComponent ? Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = EquippedGun->GetWeaponData().ReloadMontage;
	if (!MontageToPlay)
	{
		const int32 ReloadedAmmo = EquippedGun->ReloadAmmo();

		EndAbility(Handle, ActorInfo, ActivationInfo, true, ReloadedAmmo <= 0);
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MGSGameplayTags::Event_Player_Weapon_Reload_Ammo_Refill);

	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAmmoRefillEventReceived);

	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);

	PlayMontageTask->ReadyForActivation();
	WaitEventTask->ReadyForActivation();

	ExecuteEnemyGameplayCue(MGSGameplayTags::GameplayCue_Weapon_Reload, EquippedGun->GetActorLocation(), EquippedGun);
}

void UEnemyReloadGameplayAbility::OnAmmoRefillEventReceived(FGameplayEventData Payload)
{
	(void)Payload;

	UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = EnemyCombatComponent ? Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		return;
	}

	EquippedGun->ReloadAmmo();
}
