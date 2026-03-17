/*
 * 파일명 : MGSExecCalc_ProjectileDamage.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-16
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MGSExecCalc_ProjectileDamage.generated.h"

UCLASS()
class MGSPROJECT_API UMGSExecCalc_ProjectileDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UMGSExecCalc_ProjectileDamage();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
