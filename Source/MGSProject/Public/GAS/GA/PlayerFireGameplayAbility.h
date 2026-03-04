/*
 * 파일명: PlayerFireGameplayAbility.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#pragma once


#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerFireGameplayAbility.generated.h"

struct FHitResult;
class APlayerCharacter;
class AMGSPlayerController;
class ABaseGun;

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerFireGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerFireGameplayAbility();

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
	void HandleAutomaticFire();
	bool FireSingleShot();
	void EnableCombatFacing();
	void RestoreMovementFacing();

	bool FireTraceAndApplyDamage(APlayerCharacter* PlayerCharacter, AMGSPlayerController* PlayerController,
		ABaseGun* EquippedGun, AActor* DamageCauser, AController* InstigatorController, float FireRange, float Damage,
		float SpreadRadius) const;

private:
	// 라인트레이스 디버그 시각화 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableFireTraceDebug = true;

	// 디버그 라인 출력 지속 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugTraceDuration = 1.0f;

	// 히트/미스 충돌 로그 출력 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableFireTraceLog = true;

	// 탄약 상태 로그 출력 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableAmmoLog = true;

	// 총구 트레이스 시작점으로 사용할 소켓 이름(무기 메시 기준)
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FName MuzzleSocketName = TEXT("Muzzle");

	float CurrentSpreadRadius = 0.f;
	float CurrentFireInterval = 0.12f;
	FTimerHandle AutoFireTimerHandle;

	bool bCachedUseControllerRotationYaw = false;
	bool bCachedOrientRotationToMovement = true;
	bool bCachedUseControllerDesiredRotation = false;
	bool bHasCachedMovementFacing = false;
};



