/*
 * 파일명 : BaseAnimInstance.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-05
 */
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MGSAnimInstanceTypes.h"
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
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
