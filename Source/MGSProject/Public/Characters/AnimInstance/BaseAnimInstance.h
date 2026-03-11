/*
 * 파일명 : BaseAnimInstance.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-09
 */
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MGSAnimInstanceTypes.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "BaseAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MGSPROJECT_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "MGS|MotionMatching", meta = (BlueprintThreadSafe))
	void UpdateMotionMatchingNode(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "MGS|MotionMatching", meta = (BlueprintThreadSafe))
	void PostSelectionMotionMatchingNode(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	// Aim Offset 활성화
	UFUNCTION(BlueprintPure, Category = "MGS|Animation", meta = (BlueprintThreadSafe))
	bool ShouldEnableAimOffset() const;
	
	// 헬퍼함수
	UFUNCTION(BlueprintPure, Category = "MGS|Animation", meta = (BlueprintThreadSafe))
	bool HasLocomotionTag(FGameplayTag TagToCheck) const;
	
	// Steering 노드가 작동할 방향을 제공
	UFUNCTION(BlueprintPure, Category = "MGS|Steering", meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;
	
	// 제자리 회전용 Steering 활성화 여부
	UFUNCTION(BlueprintPure, Category = "MGS|Steering", meta = (BlueprintThreadSafe))
	bool IsTurnInPlaceSteeringEnabled(const FAnimNodeReference& Node) const;

	// 일반 이동용 Steering 활성화 여부 (기존 EnableSteering 수정)
	UFUNCTION(BlueprintPure, Category = "MGS|Steering", meta = (BlueprintThreadSafe))
	bool IsNormalSteeringEnabled(const FAnimNodeReference& Node) const;
	
	UFUNCTION(BlueprintPure, Category = "MGS|Warping", meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace Get_OrientationWarpingWarpingSpace() const;
	
	UFUNCTION(BlueprintPure, Category = "MGS|Procedural", meta = (BlueprintThreadSafe))
	FFootPlacementPlantSettings Get_FootPlacementPlantSettings() const;

	UFUNCTION(BlueprintPure, Category = "MGS|Procedural", meta = (BlueprintThreadSafe))
	FFootPlacementInterpolationSettings Get_FootPlacementInterpolationSettings() const;
	
protected:
	// 데이터 프록시 (데이터 복사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MGS|Animation")
	FMGSCharacterDataProxy CharacterData;

	// 가공된 값 (ABP에서 주로 참조)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MGS|Animation")
	FMGSEssentialValues EssentialValues;

	// 모션 매칭 핸들러
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Animation")
	FMGSMotionMatchingHandler MMHandler;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Animation")
	FMGSTrajectoryHandler TrajectoryHandler;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MGS|Animation")
	FMGSLocomotionState LocomotionState;
	
	// 발 접지(Foot Placement)를 위한 세부 설정 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGS|Settings")
	FMGSProceduralSettings ProceduralSettings;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
