/*
 * 파일명 : CharacterAttributeSet.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/AttributeSets/CharacterAttributeSet.h"

#include "GameplayEffectExtension.h"

UCharacterAttributeSet::UCharacterAttributeSet()
{
	InitCurrentHp(1.0f);
	InitMaxHp(1.0f);
}

void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHpAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
		return;
	}

	if (Attribute == GetCurrentHpAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, FMath::Max(1.f, GetMaxHp()));
	}
}

void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMaxHpAttribute())
	{
		SetMaxHp(FMath::Max(1.f, GetMaxHp()));
		SetCurrentHp(FMath::Clamp(GetCurrentHp(), 0.f, GetMaxHp()));
		return;
	}

	if (Data.EvaluatedData.Attribute == GetCurrentHpAttribute())
	{
		SetCurrentHp(FMath::Clamp(GetCurrentHp(), 0.f, FMath::Max(1.f, GetMaxHp())));
	}
}


