/*
 * 파일명 : EnemyDefaultMovementGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-12
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/EnemyGameplayAbility.h"
#include "EnemyDefaultMovementGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UEnemyDefaultMovementGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyDefaultMovementGameplayAbility();

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
	
};
