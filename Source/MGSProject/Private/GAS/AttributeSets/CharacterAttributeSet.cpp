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
	InitCurrentHp(100.0f);
	InitMaxHp(100.0f);
}

void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Get max hp
	if (Attribute == GetMaxHpAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
		return;
	}

	// Get current hp
	if (Attribute == GetCurrentHpAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, FMath::Max(1.0f, GetMaxHp()));
	}
}

void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Set max hp
	if (Data.EvaluatedData.Attribute == GetMaxHpAttribute())
	{
		SetMaxHp(FMath::Max(1.0f, GetMaxHp()));
		SetCurrentHp(FMath::Clamp(GetCurrentHp(), 0.0f, GetMaxHp()));
		return;
	}

	// Set current hp
	if (Data.EvaluatedData.Attribute == GetCurrentHpAttribute())
	{
		SetCurrentHp(FMath::Clamp(GetCurrentHp(), 0.0f, FMath::Max(1.0f, GetMaxHp())));
	}
}
