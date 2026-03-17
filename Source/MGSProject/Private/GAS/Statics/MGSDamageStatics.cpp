/*
 * 파일명 : MGSDamageStatics.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-16
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#include "GAS/Statics/MGSDamageStatics.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "GAS/GE/MGSDamageGameplayEffect.h"
#include "GameFramework/Pawn.h"
#include "MGSStructType.h"

bool FMGSDamageStatics::ApplyProjectileDamage(
	const FMGSProjectileAttackPayload& AttackPayload,
	AActor* EffectCauser,
	APawn* InstigatorPawn,
	AActor* TargetActor,
	const FHitResult& Hit)
{
	if (!TargetActor)
	{
		return false;
	}

	// 데미지를 받을 액터의 ASC
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return false;
	}

	// 데미지를 줄 액터
	AActor* SourceActor = AttackPayload.SourceActor.Get();
	if (!SourceActor)
	{
		SourceActor = EffectCauser;
	}

	// 데미지를 줄 액터의 ASC
	UAbilitySystemComponent* SourceASC = AttackPayload.SourceASC.Get();
	if (!SourceASC && SourceActor)
	{
		SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	}

	// GE Class
	const TSubclassOf<UGameplayEffect> DamageGEClass = AttackPayload.DamageGameplayEffectClass
		? AttackPayload.DamageGameplayEffectClass
		: TSubclassOf<UGameplayEffect>(UMGSDamageGameplayEffect::StaticClass());
	// GE Level
	const float EffectLevel = FMath::Max(1.0f, AttackPayload.EffectLevel);

	// GE Context
	FGameplayEffectContextHandle EffectContext = SourceASC
		? SourceASC->MakeEffectContext()
		: TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(AttackPayload.SourceObject.IsValid() ? AttackPayload.SourceObject.Get() : EffectCauser);
	EffectContext.AddInstigator(InstigatorPawn, EffectCauser ? EffectCauser : SourceActor);
	EffectContext.AddHitResult(Hit, true);
	// GE Spec
	FGameplayEffectSpecHandle DamageSpecHandle = SourceASC
		? SourceASC->MakeOutgoingSpec(DamageGEClass, EffectLevel, EffectContext)
		: TargetASC->MakeOutgoingSpec(DamageGEClass, EffectLevel, EffectContext);
	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		return false;
	}
	// 데미지 값 적용
	DamageSpecHandle.Data->SetSetByCallerMagnitude(
		MGSGameplayTags::Data_Damage,
		FMath::Max(0.0f, AttackPayload.BaseDamage));

	// 데미지를 줄 액터의 ASC가 존재할 경우
	if (SourceASC)
	{
		// 데미지를 줄 액터의 ASC에 목표 ASC에 GE 적용
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
		return true;
	}

	// 데미지를 받을 액터의 ASC에 GE 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	return true;
}
