/*
* 파일명 : STTask_SetEnemyStateTag.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-12
 * 수정자 : 김사윤
 * 수정일 : 2026-03-12
 */
#include "AI/STTask_SetEnemyTag.h"

#include "AIController.h"
#include "Characters/Enemies/EnemyCharacter.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus USTTask_SetEnemyTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
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

	UMGSAbilitySystemComponent* ASC = EnemyCharacter->GetMGSAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	ASC->AddLooseGameplayTag(TargetStateTag);
	return EStateTreeRunStatus::Succeeded;
}

