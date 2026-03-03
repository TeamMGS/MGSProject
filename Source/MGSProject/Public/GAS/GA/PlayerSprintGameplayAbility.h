/*
 * 파일명 : PlayerSprintGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-03
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerSprintGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerSprintGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerSprintGameplayAbility();

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

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 700.f;

	float CachedMoveSpeed = 0.f;
	bool bHasCachedMoveSpeed = false;
};