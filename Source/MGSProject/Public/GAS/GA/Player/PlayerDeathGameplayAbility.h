/*
 * 파일명 : PlayerDeathGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerDeathGameplayAbility.generated.h"

UCLASS()
class MGSPROJECT_API UPlayerDeathGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerDeathGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
