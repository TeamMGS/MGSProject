/*
 * 파일명 : PlayerDeathGameplayAbility.cpp
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */

#include "GAS/GA/Player/PlayerDeathGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UPlayerDeathGameplayAbility::UPlayerDeathGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Death);
	SetAssetTags(AbilityAssetTags);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Character_Dead);
}

void UPlayerDeathGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->CancelAllAbilities(this);
		AbilitySystemComponent->AddLooseGameplayTag(MGSGameplayTags::State_Character_Dead);
	}

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, PlayerCharacter->GetActorLocation());
		}

		PlayerCharacter->StopAimObstructionTrace();

		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}

		if (UCapsuleComponent* CapsuleComponent = PlayerCharacter->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// 몽타주 재생 태스크 실행
	if (DeathMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DeathMontage
		);

		// 완료, 블렌드 아웃, 중단, 취소 시 모두 어빌리티 종료 처리
		PlayMontageTask->OnCompleted.AddDynamic(this, &UPlayerDeathGameplayAbility::OnDeathMontageFinished);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UPlayerDeathGameplayAbility::OnDeathMontageFinished);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UPlayerDeathGameplayAbility::OnDeathMontageFinished);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UPlayerDeathGameplayAbility::OnDeathMontageFinished);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주가 없으면 즉시 종료
		OnDeathMontageFinished();
	}
}

void UPlayerDeathGameplayAbility::OnDeathMontageFinished()
{
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		// 마지막 프레임에서 애니메이션을 일시정지 시킵니다.
		if (USkeletalMeshComponent* Mesh = PlayerCharacter->GetMesh())
		{
			Mesh->bPauseAnims = true;
		}
	}

	// 애니메이션은 멈췄지만, 로직상 어빌리티는 종료해줍니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
