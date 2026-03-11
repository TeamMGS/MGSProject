/*
 * 파일명 : PlayerCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-10
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/BaseCharacter.h"
#include "PlayerCharacter.generated.h"

class UPlayerCombatComponent;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;

UCLASS()
class MGSPROJECT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	void RequestRestoreHeldMovementAbilityInputNextTick();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	virtual void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
private:
#pragma region Components
	// 스프링 암 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	// 플레이어 컴뱃 컴포넌트 : 무기 관리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerCombatComponent> PlayerCombatComponent;
	
	// 파쿠르 위치 보정용 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliSource;
	
public:
	FORCEINLINE UPlayerCombatComponent* GetPlayerCombatComponent() const { return PlayerCombatComponent; }
	
#pragma endregion
#pragma region Input
public:
	// 이동 : 네이티브 입력
	void Input_Move(const FInputActionValue& InputActionValue);
	// 시점 : 네이티브 입력
	void Input_Look(const FInputActionValue& InputActionValue);
	
	// 어빌리티 입력 눌렀을 때
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	// 어빌리티 입력 땠을 때
	void Input_AbilityInputReleased(FGameplayTag InputTag);
#pragma endregion 

private:
	// 캐릭터 방향과 컨트롤러 방향 동기화
	void ApplyAlwaysAimFacingMode();
	void UpdateFallingStateTag();
	void TryRestoreHeldMovementAbilityInput();
	// 다음 틱에 스프레드 갱신
	void RequestSpreadRefreshNextTick();
	// 스프레드 보정
	void UpdateCurrentSpreadFromState();
	// 스프레드 보정률 계산
	float CalculateCurrentSpreadStateMultiplier() const;
	// 장착 무기 변경 핸들러
	void HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag);
	// 이동에 따른 스프레드 갱신 핸들러
	UFUNCTION()
	void HandleSpreadMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
	// 웅크릴 때 카메라 보간
	void StartCrouchCameraBlend(float TargetOffsetZ);
	// 카메라 블렌딩 갱신
	void UpdateCrouchCameraBlend();

	static constexpr float SpreadJumpMultiplier = 2.4f; // 점프 시 스프레드 보정률
	static constexpr float SpreadSprintMultiplier = 1.6f; // 뛸 시 스프레드 보정률
	static constexpr float SpreadMovingMultiplier = 1.3f; // 움직일 시 스프레드 보정률
	static constexpr float SpreadWalkMultiplier = 1.1f; // 걸을 시 스프레드 보정률
	static constexpr float SpreadCrouchStillMultiplier = 0.7f; // 웅크릴 시 스프레드 보정률
	static constexpr float SpreadAimMultiplier = 0.65f; // 조준 시 스프레드 보정률
	static constexpr float SpreadMovingSpeedThreshold = 10.0f; // 이동 시 스프레드 보정 임계값
	static constexpr float CrouchCameraStandingOffsetZ = 0.0f; // 서있을 때 카메라 Z 오프셋
	static constexpr float CrouchCameraCrouchedOffsetZ = -29.0f; // 웅크릴 때 카메라 Z 오프셋
	static constexpr float CrouchCameraInterpSpeed = 12.0f; // 웅크릴 때 카메라 보간 속도
	static constexpr float CrouchCameraBlendTickInterval = 1.0f / 120.0f; // 웅크릴 때 카메라 보간 간격

	FDelegateHandle EquippedWeaponChangedHandle;
	bool bPendingSpreadRefreshRequest = false;
	float DesiredCrouchCameraOffsetZ = CrouchCameraStandingOffsetZ; // 목표 카메라 Z 오프셋
	FTimerHandle CrouchCameraBlendTimerHandle; // 웅크릴 때 카메라 보간 타이머 핸들
	
};
