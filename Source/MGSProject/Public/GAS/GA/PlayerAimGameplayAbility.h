/*
 * 파일명 : PlayerAimGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-03
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerAimGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerAimGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerAimGameplayAbility();

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
	// FOV
	// Cache
	float CachedFOV = 90.0f;
	// Flag
	bool bHasCachedFOV = false;

	// SocketOffset
	// Cache
	FVector CachedCameraSocketOffset = FVector::ZeroVector;
	// Flag
	bool bHasCachedCameraSocketOffset = false;

	// CameraCollisionTest
	// Cache
	bool bCachedCameraCollisionTest = true;
	// Flag
	bool bHasCachedCameraCollisionTest = false;
	
};
