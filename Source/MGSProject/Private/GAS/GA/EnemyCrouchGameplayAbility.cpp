/*
 * 파일명 : EnemyCrouchGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-12
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/EnemyCrouchGameplayAbility.h"

#include "Characters/Enemies/EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"

UEnemyCrouchGameplayAbility::UEnemyCrouchGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Enemy_Crouch);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;

	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Enemy_Crouching);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Enemy_Movement_Sprint);
	BlockAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Enemy_Sprint);
}

bool UEnemyCrouchGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AEnemyCharacter* EnemyCharacter = ActorInfo ? Cast<AEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!EnemyCharacter)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = EnemyCharacter->GetCharacterMovement();
	return MovementComponent && !MovementComponent->IsFalling();
}

void UEnemyCrouchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	bool bApplied = false;
	if (AEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo())
	{
		bApplied = EnemyCharacter->SetCrouchState(true);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bApplied);
}
