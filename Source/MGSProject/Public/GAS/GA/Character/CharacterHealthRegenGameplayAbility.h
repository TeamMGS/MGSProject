/*
 * 파일명 : CharacterHealthRegenGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-24
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/BaseGameplayAbility.h"
#include "CharacterHealthRegenGameplayAbility.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class MGSPROJECT_API UCharacterHealthRegenGameplayAbility : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UCharacterHealthRegenGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	void HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData);
	void RestartRegenDelay();
	void StartRegeneration();
	void StopRegeneration();
	void ApplyRegenerationTick();
	bool CanRegenerate() const;
	float GetCurrentHp() const;
	float GetMaxHp() const;
	void ClearTimers();

private:
	UPROPERTY(EditDefaultsOnly, Category = "HealthRegen", meta = (ClampMin = "0.0"))
	float RegenStartDelay = 7.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HealthRegen", meta = (ClampMin = "0.0"))
	float HealPerSecond = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HealthRegen", meta = (ClampMin = "0.01"))
	float RegenTickInterval = 1.0f;

	FDelegateHandle CurrentHpChangedDelegateHandle;
	FTimerHandle RegenDelayTimerHandle;
	FTimerHandle RegenTickTimerHandle;
	bool bIsHpDelegateBound = false;
};
