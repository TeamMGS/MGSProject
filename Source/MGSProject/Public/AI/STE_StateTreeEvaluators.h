#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"

#include "STE_StateTreeEvaluators.generated.h"

/**
 * STE 순찰 맵 Evaluator용 인스턴스 데이터.
 */
USTRUCT()
struct FSTE_PatrolMapEvaluatorInstanceData
{
	GENERATED_BODY()

	/** ID로 매핑된 순찰 지점. */
	UPROPERTY(EditAnywhere, Category = Input)
	TMap<int32, FVector> PatrolMap;

	/** 순찰 ID(키) */
	UPROPERTY(EditAnywhere, Category = Input)
	int32 TargetIndex = 0;

	/** Move To에 바인딩할 위치 */
	UPROPERTY(VisibleAnywhere, Category = Output)
	FVector FoundLocation = FVector::ZeroVector;

	/** TargetIndex가 PatrolMap에서 발견되면 True */
	UPROPERTY(VisibleAnywhere, Category = Output)
	bool bHasFoundLocation = false;

	/** TargetIndex가 없을 때 소유 액터 위치로 폴백할지 여부 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bFallbackToOwnerLocation = true;
};

/**
 * Map ID로 순찰 위치를 해석하는 Evaluator.
 */
USTRUCT(meta = (DisplayName = "STE Patrol Map Evaluator", Category = "STE"))
struct FSTE_PatrolMapEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTE_PatrolMapEvaluatorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};
