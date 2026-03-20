#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_SetNextPatrolTarget.generated.h"

UCLASS()
class MGSPROJECT_API UBTTask_SetNextPatrolTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetNextPatrolTarget();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolManagerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CurrentPatrolIndexKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPatrolActorKey;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	FName PatrolMapPropertyName;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
