/*
 * 파일명 : MGSCharacterMovementComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-06
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MGSCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MGSPROJECT_API UMGSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	
	// 현재 입력과 상태를 기반으로 원하는 보행 등급(Gait)을 반환
	FGameplayTag GetDesiredGait() const;

	// GetDesiredGait() 판단을 위한 질주 가능 여부 체크 (GASP의 CanSprint 노드)
	bool CanSprint() const;
	
	// 최대 가속도를 동적으로 계산하는 함수
	float CalculateMaxAcceleration() const;
	
	// 동적 제동력 계산 함수
	float CalculateBrakingDeceleration() const;
	
	// 동적 지면 마찰력 계산 함수
	float CalculateGroundFriction() const;
	
	// 방향별 최대 속도 계산 함수
	float CalculateMaxSpeed() const;
	
	// 앉은 상태의 최대 속도 계산 함수
	float CalculateMaxCrouchSpeed() const;
	
	// 매 프레임 캐릭터의 회전 방식을 결정
	void UpdateRotationMode();
	
protected:
	// 보행 등급별 속도 설정 (X: 정면, Y: 측면, Z: 후면)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Movement")
	FVector WalkSpeeds = FVector(375.f, 360.f, 350.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Movement")
	FVector RunSpeeds = FVector(575.f, 510.f, 550.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Movement")
	FVector SprintSpeeds = FVector(700.f, 700.f, 700.f);
	
	// 앉은 상태의 방향별 속도 설정 (X: 정면, Y: 측면, Z: 후면)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Movement")
	FVector CrouchSpeeds = FVector(150.f, 130.f, 120.f);

	// 방향(Degree)을 0~2 범위로 매핑하는 곡선 (0:정면, 1:측면, 2:후면)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Movement")
	TObjectPtr<UCurveFloat> StrafeSpeedMapCurve;
};
