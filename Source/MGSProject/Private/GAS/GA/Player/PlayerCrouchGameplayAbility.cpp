/*
 * 파일명 : PlayerCrouchGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-03
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/Player/PlayerCrouchGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"

UPlayerCrouchGameplayAbility::UPlayerCrouchGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Crouch);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;

	ActivationOwnedTags.AddTag(MGSGameplayTags::State_Player_Crouching);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Player_Movement_Sprint);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Player_Movement_Walk);

	BlockAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Sprint);
	BlockAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Walk);
	CancelAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Sprint);
	CancelAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Walk);
}

bool UPlayerCrouchGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		// 공중 상태 확인
		return !MovementComponent->IsFalling();
	}

	return false;
}

void UPlayerCrouchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SetCanBeCanceled(true);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		// 웅크리기
		PlayerCharacter->Crouch();
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UPlayerCrouchGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UPlayerCrouchGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		// 웅크리기 해제
		PlayerCharacter->UnCrouch();
		// 이동 상태 복원
		PlayerCharacter->RequestRestoreHeldMovementAbilityInputNextTick();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
