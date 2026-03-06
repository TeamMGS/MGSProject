/*
 * 파일명 : PlayerCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
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
	virtual void PossessedBy(AController* NewController) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
private:
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerCombatComponent> PlayerCombatComponent;
	
	// 파쿠르 위치 보정용 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
public:
	FORCEINLINE UPlayerCombatComponent* GetPlayerCombatComponent() const { return PlayerCombatComponent; }
	
#pragma endregion
#pragma region Input
public:
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);
#pragma endregion 

private:
	void ApplyAlwaysAimFacingMode();
	void UpdateFallingStateTag();
	void TryRestoreHeldMovementAbilityInput();
	
};
