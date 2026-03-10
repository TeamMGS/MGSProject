/*
 * 파일명 : EnemyAIController.h
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-10
 */
#pragma once

#include "CoreMinimal.h"
#include "AI/Core/AICoreAIController.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;

/**
 * 
 */
UCLASS()
class MGSPROJECT_API AEnemyAIController : public AAICoreAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

};
