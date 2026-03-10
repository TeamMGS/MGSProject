/*
 * 파일명: PlayerFireGameplayAbility.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerFireGameplayAbility.generated.h"

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
	void HandleAutomaticFire(); // 자동 연사
	bool FireSingleShot(); // 실제 한 발 처리
	float CalculateStateSpreadMultiplier(const APlayerCharacter* PlayerCharacter) const; // 스프레드 보정값 계산
	void ApplyWeaponRecoil(AMGSPlayerController* PlayerController, ABaseGun* EquippedGun) const; // 반동 적용

	// 총구 위치 기준으로 발사체를 스폰하고 발사 방향을 적용합니다.
	bool SpawnProjectileShot(APlayerCharacter* PlayerCharacter, AMGSPlayerController* PlayerController,
		ABaseGun* EquippedGun, float AimReferenceDistance, float Damage, float SpreadRadius) const;

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

	// 점프/낙하 중 스프레드 배율
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float JumpSpreadMultiplier = 2.4f;

	// 뛰기(스프린트) 중 스프레드 배율
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float SprintSpreadMultiplier = 1.6f;

	// 일반 이동 중 스프레드 배율
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float MovingSpreadMultiplier = 1.3f;

	// 걷기(워크) 중 스프레드 배율
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float WalkSpreadMultiplier = 1.1f;

	// 웅크린 상태 스프레드 배율(1.0 미만이면 감소)
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float CrouchSpreadMultiplier = 0.7f;

	// 조준(ADS) 상태 스프레드 배율(1.0 미만이면 감소)
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float AimSpreadMultiplier = 0.65f;

	// 이동 상태 판정을 위한 최소 수평 속도
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Spread|State", meta = (ClampMin = "0.0"))
	float MinMovingSpeedForSpread = 10.f;

	float CurrentSpreadRadius = 0.f; // 현재 탄착군 반경
	float CurrentFireInterval = 0.12f; // 현재 연사 간격
	FTimerHandle AutoFireTimerHandle; // 자동연사 타이머
};
