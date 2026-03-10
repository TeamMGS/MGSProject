/*
* 파일명 : EnemyAIController.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-05
 */


#include "Characters/Enemies/EnemyAIController.h"

#include "StateTree.h"
#include "Components/StateTreeComponent.h"

AEnemyAIController::AEnemyAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!StateTreeComponent)
	{
		return;
	}

	if (DefaultStateTree)
	{
		StateTreeComponent->SetStateTree(DefaultStateTree);
		StateTreeComponent->StartLogic();
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("UnPossess"));
	}

	Super::OnUnPossess();
}

