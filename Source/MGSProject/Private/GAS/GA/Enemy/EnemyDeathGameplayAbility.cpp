/*
 * 파일명 : EnemyDeathGameplayAbility.cpp
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#include "GAS/GA/Enemy/EnemyDeathGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Characters/Enemies/EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"

UEnemyDeathGameplayAbility::UEnemyDeathGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Enemy_Death);
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Character_Dead);
}

void UEnemyDeathGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (AEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* MovementComponent = EnemyCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}

		if (UCapsuleComponent* CapsuleComponent = EnemyCharacter->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController()))
		{
			AIController->StopMovement();

			if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
			{
				BrainComponent->StopLogic(TEXT("Enemy died"));
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
