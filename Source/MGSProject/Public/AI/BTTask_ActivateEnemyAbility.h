/*
 * 파일명 : BTTask_ActivateEnemyAbility.h
 * 생성자 : AI Assistant
 * 생성일 : 2026-03-23
 */

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateEnemyAbility.generated.h"

UCLASS()
class MGSPROJECT_API UBTTask_ActivateEnemyAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateEnemyAbility();

	// 발동시킬 어빌리티의 태그를 언리얼 에디터에서 선택할 수 있게 합니다.
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTagToActivate;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
