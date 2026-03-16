/*
 * 파일명 : EnemyDeathGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Enemy/EnemyGameplayAbility.h"
#include "EnemyDeathGameplayAbility.generated.h"

UCLASS()
class MGSPROJECT_API UEnemyDeathGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyDeathGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
