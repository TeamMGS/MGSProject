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

struct FInputActionValue;
class UMotionWarpingComponent;
class UCameraComponent;
class UPlayerCombatComponent;
class USpringArmComponent;
class UDA_SpreadSettings;
class UMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;

UCLASS()
class MGSPROJECT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	// IPawnCombatInterface pure virtual function override
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	
	// Getter
	FORCEINLINE UPlayerCombatComponent* GetPlayerCombatComponent() const { return PlayerCombatComponent; }
	const UDA_SpreadSettings* GetSpreadSettings() const;
	
	// 현재 누르고 있는 입력 어빌리티를 다음 틱에 복원 요청
	void RequestRestoreHeldMovementAbilityInputNextTick();
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Movement virtual function override
	// Update movement mode
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	// Landed
	virtual void Landed(const FHitResult& Hit) override;
	// Crouch
	virtual void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	virtual void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	
private:
#pragma region Components
	// Spring Arm Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	// Combat Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerCombatComponent> PlayerCombatComponent;
	
	// 파쿠르 위치 보정용 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliSource;
	
#pragma endregion
#pragma region Input
public:
	// Native Input
	// Move
	void Input_Move(const FInputActionValue& InputActionValue);
	// Look
	void Input_Look(const FInputActionValue& InputActionValue);
	
	// Ability Input
	// Pressed
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	// Released
	void Input_AbilityInputReleased(FGameplayTag InputTag);
#pragma endregion 

private:
	// Handler
	// 이동 갱신에 따른 핸들러
	UFUNCTION()
	void HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
	// 무기 장착 변경에 따른 핸들러
	UFUNCTION()
	void HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag);
	
	// 현재 누르고 있는 입력 어빌리티 복원
	void TryRestoreHeldMovementAbilityInput();
	// 공중 상태 조회 후 태그 갱신
	void UpdateFallingStateTag();
		
	// Spread
	// 다음 틱에 스프레드 갱신
	void RequestSpreadRefreshNextTick();
	// 스프레드 보정
	void UpdateCurrentSpreadFromState();
	// 스프레드 보정률 계산
	float CalculateCurrentSpreadStateMultiplier() const;
	
	// Crouch
	// 카메라 보간 요청
	void StartCrouchCameraBlend(float TargetOffsetZ);
	// 카메라 블렌딩 갱신
	void UpdateCrouchCameraBlend();

private:
	// DA_SpreadSettings: 상태에 따른 스프레드 보정 스케일 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDA_SpreadSettings> SpreadSettingsData;
	
	// 서있을 때 카메라 Z 오프셋
	static constexpr float CrouchCameraStandingOffsetZ = 0.0f;
	// 웅크릴 때 카메라 Z 오프셋
	static constexpr float CrouchCameraCrouchedOffsetZ = -29.0f;
	// 웅크릴 때 카메라 보간 속도
	static constexpr float CrouchCameraInterpSpeed = 12.0f;
	// 웅크릴 때 카메라 보간 간격
	static constexpr float CrouchCameraBlendTickInterval = 1.0f / 120.0f;

	// 무기 장착 변경 델리게이트 핸들
	FDelegateHandle EquippedWeaponChangedHandle;
	// 스프레드 갱신 진행 여부 플래그
	bool bPendingSpreadRefreshRequest = false;
	// 목표 카메라 Z 오프셋
	float DesiredCrouchCameraOffsetZ = CrouchCameraStandingOffsetZ;
	// 웅크릴 때 카메라 보간 타이머 핸들
	FTimerHandle CrouchCameraBlendTimerHandle;
	
};
