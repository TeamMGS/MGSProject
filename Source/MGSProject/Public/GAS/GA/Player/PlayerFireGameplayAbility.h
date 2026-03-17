/*
 * 파일명: PlayerFireGameplayAbility.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-16
 */

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerFireGameplayAbility.generated.h"

class ABaseGun;
class AMGSPlayerController;
class APlayerCharacter;

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
	// 자동 연사 (타이머 돌면서 FireSingleShot 처리)
	void HandleAutomaticFire();
	// 실제 한 발 처리
	bool FireSingleShot();
	// 스프레드 보정값 계산
	float CalculateStateSpreadMultiplier(const APlayerCharacter* PlayerCharacter) const;
	// 총구 위치 기준으로 발사체를 스폰하고 발사 방향을 적용
	bool SpawnProjectileShot(APlayerCharacter* PlayerCharacter, const AMGSPlayerController* PlayerController,
		ABaseGun* EquippedGun, float AimReferenceDistance, float SpreadRadius) const;
	// 반동 적용
	void ApplyWeaponRecoil(AMGSPlayerController* PlayerController, const ABaseGun* EquippedGun) const;

private:
	// 총구 트레이스 시작점으로 사용할 소켓 이름(무기 메시 기준)
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	FName MuzzleSocketName = TEXT("Muzzle");
	// 현재 스프레드 반경
	float CurrentSpreadRadius = 0.0f;
	// 현재 연사 간격
	float CurrentFireInterval = 0.12f;
	// 자동연사 타이머
	FTimerHandle AutoFireTimerHandle;
	
	// Debug
	// 라인트레이스 디버그 시각화 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableFireTraceDebug = true;
	// 디버그 라인 출력 지속 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugTraceDuration = 1.0f;
	// 히트/미스 충돌 로그 출력 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableFireTraceLog = true;
		
};
