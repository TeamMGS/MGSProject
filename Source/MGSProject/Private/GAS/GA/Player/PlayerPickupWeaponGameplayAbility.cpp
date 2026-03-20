#include "GAS/GA/Player/PlayerPickupWeaponGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseWeapon.h"

UPlayerPickupWeaponGameplayAbility::UPlayerPickupWeaponGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_PickupWeapon);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

bool UPlayerPickupWeaponGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const APlayerCharacter* PlayerCharacter = ActorInfo ? Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter)
	{
		return false;
	}

	if (const UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		if (MovementComponent->IsFalling())
		{
			return false;
		}
	}

	return true;
}

void UPlayerPickupWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bool bPickupSuccess = false;
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseWeapon* PickupCandidate = PlayerCombatComponent ? PlayerCombatComponent->GetNearbyDroppedWeapon() : nullptr;
	if (!PickupCandidate)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (PlayerCombatComponent)
	{
		bPickupSuccess = PlayerCombatComponent->PickupDroppedWeapon(PickupCandidate);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bPickupSuccess);
}
