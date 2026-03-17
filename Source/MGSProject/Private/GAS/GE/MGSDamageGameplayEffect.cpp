/*
 * 파일명 : MGSDamageGameplayEffect.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-09
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GE/MGSDamageGameplayEffect.h"

#include "GAS/ExecCalc/MGSExecCalc_ProjectileDamage.h"

UMGSDamageGameplayEffect::UMGSDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition DamageExecution;
	DamageExecution.CalculationClass = UMGSExecCalc_ProjectileDamage::StaticClass();
	Executions.Add(DamageExecution);
}
