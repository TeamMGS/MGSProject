/*
 * 파일명: PlayerReloadGameplayAbility.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#include "GAS/GA/PlayerReloadGameplayAbility.h"

#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseGun.h"


UPlayerReloadGameplayAbility::UPlayerReloadGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Reload);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

bool UPlayerReloadGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UPlayerCombatComponent* PlayerCombatComponent = Cast<UPlayerCombatComponent>(GetPawnCombatComponentFromActorInfo());
	if (!PlayerCombatComponent)
	{
		return false;
	}

	const ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	return EquippedGun && EquippedGun->CanReload();
}

void UPlayerReloadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const int32 ReloadedAmmo = EquippedGun->ReloadAmmo();
	if (bEnableReloadLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[Reload] Reloaded=%d Current=%d/%d Carried=%d"),
			ReloadedAmmo,
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, ReloadedAmmo <= 0);
}



