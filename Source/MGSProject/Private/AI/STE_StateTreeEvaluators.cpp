#include "AI/STE_StateTreeEvaluators.h"

#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"

void FSTE_PatrolMapEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (const FVector* Found = InstanceData.PatrolMap.Find(InstanceData.TargetIndex))
	{
		InstanceData.FoundLocation = *Found;
		InstanceData.bHasFoundLocation = true;
		return;
	}

	InstanceData.bHasFoundLocation = false;

	if (InstanceData.bFallbackToOwnerLocation)
	{
		if (const AActor* Owner = Cast<AActor>(Context.GetOwner()))
		{
			// 안전 폴백: 경로 ID가 없을 때 현재 위치 사용
			InstanceData.FoundLocation = Owner->GetActorLocation();
			return;
		}
	}

	InstanceData.FoundLocation = FVector::ZeroVector;
	(void)DeltaTime;
}

#if WITH_EDITOR
FText FSTE_PatrolMapEvaluator::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>STE Patrol Map Evaluator</b>");
}
#endif // WITH_EDITOR
