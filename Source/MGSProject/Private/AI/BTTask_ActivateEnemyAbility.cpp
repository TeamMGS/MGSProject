/*
 * 파일명 : BTTask_ActivateEnemyAbility.cpp
 * 생성자 : AI Assistant
 * 생성일 : 2026-03-23
 */

#include "AI/BTTask_ActivateEnemyAbility.h"

#include "AIController.h"
#include "Characters/Enemies/EnemyCharacter.h"

UBTTask_ActivateEnemyAbility::UBTTask_ActivateEnemyAbility()
{
	NodeName = TEXT("Activate Enemy Ability");
}

EBTNodeResult::Type UBTTask_ActivateEnemyAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(AIController->GetPawn());
	if (!EnemyCharacter)
	{
		return EBTNodeResult::Failed;
	}

	if (!AbilityTagToActivate.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// 태그를 사용하여 EnemyCharacter의 어빌리티를 발동합니다.
	// 예: Ability.Enemy.Fire, Ability.Enemy.Reload 등
	bool bSuccess = EnemyCharacter->ActivateEnemyAbilityByTag(AbilityTagToActivate);

	// 어빌리티 실행 성공 시(발동 조건 달성) Succeeded 반환
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
