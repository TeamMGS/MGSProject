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
#include "Engine/Engine.h"
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

	const FString TagString = TargetStateTag.ToString();
	if (TagString.StartsWith(TEXT("Ability")))
	{
		// 어빌리티 실행 (성공 여부에 관계없이 State Tree를 멈추지 않도록 무조건 Succeeded 반환)
		bool bSuccess = EnemyCharacter->ActivateEnemyAbilityByTag(TargetStateTag);
	}
	else if (TagString.StartsWith(TEXT("State")))
	{
		// C++ State 설정 함수 호출
		EnemyCharacter->SetEnemyStateTagFromAI(TargetStateTag);
	}
	else
	{
		// 기타 태그는 기존 방식대로 유지
		if (UMGSAbilitySystemComponent* ASC = EnemyCharacter->GetMGSAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(TargetStateTag);
		}
	}

	return EStateTreeRunStatus::Succeeded;
}

