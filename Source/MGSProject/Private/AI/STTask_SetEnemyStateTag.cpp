/*
* 파일명 : STTask_SetEnemyStateTag.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-12
 * 수정자 : 김사윤
 * 수정일 : 2026-03-12
 */
#include "AI/STTask_SetEnemyStateTag.h"

#include "AIController.h"
#include "Characters/Enemies/EnemyCharacter.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus USTTask_SetEnemyStateTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (!TargetStateTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	UObject* OwnerObject = Context.GetOwner();
	if (!OwnerObject)
	{
		return EStateTreeRunStatus::Failed;
	}

	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(OwnerObject);
	if (!EnemyCharacter)
	{
		if (const AAIController* AIController = Cast<AAIController>(OwnerObject))
		{
			EnemyCharacter = Cast<AEnemyCharacter>(AIController->GetPawn());
		}
	}

	if (!EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	EnemyCharacter->SetEnemyStateTagFromAI(TargetStateTag);
	return EStateTreeRunStatus::Succeeded;
}

