/*
* 파일명 : STTask_SetEnemyStateTag.h
 * 생성자 : 김사윤
 * 생성일 : 2026-03-12
 * 수정자 : 김사윤
 * 수정일 : 2026-03-12
 */
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "STTask_SetEnemyStateTag.generated.h"

/**
 * 
 */
UCLASS()
class MGSPROJECT_API USTTask_SetEnemyStateTag : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

public:
	UPROPERTY(EditAnywhere, Category = "StateTree")
	FGameplayTag TargetStateTag;
};
