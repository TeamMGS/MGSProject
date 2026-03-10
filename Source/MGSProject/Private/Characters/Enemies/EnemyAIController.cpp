/*
* 파일명 : EnemyAIController.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-10
 */


#include "Characters/Enemies/EnemyAIController.h"

#include "GameplayStateTreeModule/Public/Components/StateTreeAIComponent.h"

AEnemyAIController::AEnemyAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

	// Match AI StateTree expectations: start logic on possess and attach to pawn for EQS.
	bStartAILogicOnPossess = true;
	bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!StateTreeComponent)
	{
		return;
	}

	StateTreeComponent->StartLogic();
}

void AEnemyAIController::OnUnPossess()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("UnPossess"));
	}

	Super::OnUnPossess();
}

