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
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

class UAbilityTask_WaitGameplayEvent;

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

		PlayMontageTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Event.Character.DeathFreeze")));

		WaitEventTask->EventReceived.AddDynamic(this, &UPlayerDeathGameplayAbility::OnDeathMontageFinished);
		WaitEventTask->ReadyForActivation();
	}
}

void UPlayerDeathGameplayAbility::OnDeathMontageFinished(FGameplayEventData Payload)
{
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (USkeletalMeshComponent* Mesh = PlayerCharacter->GetMesh())
		{
			// 마지막 프레임 고정
			Mesh->bPauseAnims = true;
			// 재생 속도를 0으로 만들어 몽타주가 끝나지 않게 원천 봉쇄
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_SetPlayRate(DeathMontage, 0.0f);
			}
		}
	}

	// 애니메이션은 멈췄지만, 로직상 어빌리티는 종료해줍니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
