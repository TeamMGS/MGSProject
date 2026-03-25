/*
 * 파일명 : EnemyFireGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Enemy/EnemyGameplayAbility.h"
#include "EnemyFireGameplayAbility.generated.h"

class ABaseGun;
class AEnemyCharacter;

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UEnemyFireGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyFireGameplayAbility();

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

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	bool FireSingleShot(AEnemyCharacter* EnemyCharacter, ABaseGun* EquippedGun);
	bool SpawnProjectileShot(AEnemyCharacter* EnemyCharacter, ABaseGun* EquippedGun,
		float AimReferenceDistance, float SpreadRadius) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableFireTraceLog = true;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableAmmoLog = true;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FName MuzzleSocketName = TEXT("Muzzle");

	float CurrentSpreadRadius = 0.0f;
};
