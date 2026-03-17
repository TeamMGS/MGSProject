/*
 * 파일명 : EnemyReloadGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Enemy/EnemyGameplayAbility.h"
#include "EnemyReloadGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UEnemyReloadGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyReloadGameplayAbility();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableReloadLog = true;

	UFUNCTION()
	void OnAmmoRefillEventReceived(FGameplayEventData Payload);
};
