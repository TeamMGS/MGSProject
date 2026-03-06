/*
 * 파일명 : AICoreAIController.h
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-05
 */
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICoreAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS()
class MGSPROJECT_API AAICoreAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAICoreAIController();

protected:
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	void ConfigurePerception();
};
