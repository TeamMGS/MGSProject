/*
 * 파일명 : MGSLocomotionComponent.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-06
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MGSLocomotionComponent.generated.h"

class ABaseCharacter;
class UMGSCharacterMovementComponent;

UCLASS( meta=(BlueprintSpawnableComponent) )
class MGSPROJECT_API UMGSLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	UMGSLocomotionComponent();
	
	void HandleOnJumped();
	
	void HandleOnLanded(const FVector& LandVelocity);
	
	void UpdateMovementTags(float DeltaSeconds);
	
protected:
	virtual void BeginPlay() override;

	// MGSMovementComponent 델리게이트에 바인딩할 함수 
	UFUNCTION()
	void OnMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
	
	// 점프 직전의 지상 속도 저장 (GASP의 GroundSpeedBeforeJump)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MGS|Locomotion")
	float GroundSpeedBeforeJump = 0.f;
	
	// 어느 정도 속도부터 무거운 착지로 볼 것인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Locomotion")
	float HeavyLandSpeedThreshold = 700.f; // 필요에 따라 에디터에서 조절

private:
	// 착지 태그 유지를 위한 타이머
	float LandingTagTimer = 0.f;
	
	// 공중에 떠 있는 시간을 체크하기 위한 변수 추가
	float CurrentAirTime = 0.f;
	
	// 피벗 태그 유지를 위한 타이머
	float PivotingTagTimer = 0.f;

	// 피벗 애니메이션이 재생되기에 충분한 최소 시간 (GASP 기준 약 0.2~0.3초)
	const float MinPivotingDuration = 0.55f;
	
	FVector LastFrameForward = FVector::ForwardVector;

private:
	// 캐싱용 변수들
	UPROPERTY()
	TObjectPtr<ABaseCharacter> OwningCharacter;

	UPROPERTY()
	TObjectPtr<UMGSCharacterMovementComponent> MGSMovementComponent;
	
	FVector LastLandVelocity = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MGS|Locomotion", meta = (AllowPrivateAccess = "true"))
	FGameplayTag LastMovementStateTag;
	
	// 이전 프레임의 상태를 저장
	FGameplayTag LastGaitTag;
	FGameplayTag LastStanceTag;
	FGameplayTag LastModeTag;
};
