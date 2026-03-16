/*
 * 파일명 : PlayerJumpGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-03
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/Player/PlayerJumpGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "GAS/MGSGameplayTags.h"

UPlayerJumpGameplayAbility::UPlayerJumpGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Jump);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;

	CancelAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Sprint);
	CancelAbilitiesWithTag.AddTag(MGSGameplayTags::Ability_Player_Walk);
}

bool UPlayerJumpGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	if (PlayerCharacter)
	{
		// 플레이어 캐릭터가 점프 가능한지 조회
		return PlayerCharacter->CanJump();
	}

	return false;
}

void UPlayerJumpGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo(); PlayerCharacter && PlayerCharacter->CanJump())
	{
		// 점프
		PlayerCharacter->Jump();
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UPlayerJumpGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		// 점프 종료
		PlayerCharacter->StopJumping();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPlayerJumpGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		// 점프 종료
		PlayerCharacter->StopJumping();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
