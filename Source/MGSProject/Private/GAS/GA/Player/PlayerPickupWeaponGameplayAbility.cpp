/*
 * 파일명 : PlayerPickupWeaponGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-10
 * 수정자 : 장대한
 * 수정일 ; 2026-03-10
 */

#include "GAS/GA/Player/PlayerPickupWeaponGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseWeapon.h"

UPlayerPickupWeaponGameplayAbility::UPlayerPickupWeaponGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_PickupWeapon);
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
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseWeapon* PickupCandidate = PlayerCombatComponent ? PlayerCombatComponent->GetNearbyDroppedWeapon() : nullptr;
	DrawPickupDebugSphere(PlayerCharacter, PickupCandidate);
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

void UPlayerPickupWeaponGameplayAbility::DrawPickupDebugSphere(const APlayerCharacter* PlayerCharacter, const ABaseWeapon* PickupCandidate) const
{
	if (!PlayerCharacter)
	{
		return;
	}

	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	FVector DebugCenter = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() * PickupDebugForwardOffset;
	float DebugRadius = PickupDebugRadius;
	if (PickupCandidate)
	{
		DebugCenter = PickupCandidate->GetActorLocation();
		const float CandidateDebugRadius = PickupCandidate->GetPickupDetectionDebugRadius();
		if (CandidateDebugRadius > KINDA_SMALL_NUMBER)
		{
			DebugRadius = CandidateDebugRadius;
		}
	}

	const FColor DebugColor = PickupCandidate ? FColor::Green : FColor::Red;
	DrawDebugSphere(World, DebugCenter, DebugRadius, 20, DebugColor, false, 1.0f, 0, 1.5f);
}
