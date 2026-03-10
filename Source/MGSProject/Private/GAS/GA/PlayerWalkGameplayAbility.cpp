/*
 * 파일명 : PlayerWalkGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-03
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/PlayerWalkGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"

UPlayerWalkGameplayAbility::UPlayerWalkGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Walk);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;

	ActivationOwnedTags.AddTag(MGSGameplayTags::State_Player_Movement_Walk);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Player_Movement_Sprint);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Player_Crouching);
	BlockAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Sprint);
	BlockAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Crouch);
}

bool UPlayerWalkGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		return !MovementComponent->IsFalling();
	}

	return false;
}

void UPlayerWalkGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			CachedMoveSpeed = MovementComponent->MaxWalkSpeed;
			bHasCachedMoveSpeed = true;
			MovementComponent->MaxWalkSpeed = WalkSpeed;
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UPlayerWalkGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPlayerWalkGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bHasCachedMoveSpeed)
	{
		if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
		{
			if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
			{
				MovementComponent->MaxWalkSpeed = CachedMoveSpeed;
			}
		}

		bHasCachedMoveSpeed = false;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
