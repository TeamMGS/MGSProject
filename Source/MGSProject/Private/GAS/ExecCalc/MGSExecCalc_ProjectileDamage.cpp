/*
 * 파일명 : MGSExecCalc_ProjectileDamage.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-16
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#include "GAS/ExecCalc/MGSExecCalc_ProjectileDamage.h"

#include "Characters/BaseCharacter.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "GameplayEffectExtension.h"

UMGSExecCalc_ProjectileDamage::UMGSExecCalc_ProjectileDamage()
{
}

void UMGSExecCalc_ProjectileDamage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& OwningSpec = ExecutionParams.GetOwningSpec();
	const float BaseDamage = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MGSGameplayTags::Data_Damage, false, 0.0f));
	if (BaseDamage <= 0.0f)
	{
		return;
	}

	const FHitResult* HitResult = OwningSpec.GetContext().GetHitResult();
	AActor* TargetActor = ExecutionParams.GetTargetAbilitySystemComponent()
		? ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor()
		: nullptr;

	float DamageMultiplier = 1.0f;
	if (HitResult)
	{
		if (const ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(TargetActor))
		{
			DamageMultiplier = FMath::Max(0.0f, HitCharacter->GetDamageMultiplierForHit(*HitResult));
		}
	}

	const float FinalDamage = BaseDamage * DamageMultiplier;
	if (FinalDamage <= 0.0f)
	{
		return;
	}

	if (DamageMultiplier > 1.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("[Headshot] Target=%s Component=%s BaseDamage=%.1f Multiplier=%.2f FinalDamage=%.1f"),
			*GetNameSafe(TargetActor),
			HitResult ? *GetNameSafe(HitResult->GetComponent()) : TEXT("None"),
			BaseDamage,
			DamageMultiplier,
			FinalDamage);
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UCharacterAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage));
}
