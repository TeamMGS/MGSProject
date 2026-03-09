/*
 * 파일명 : MGSDamageGameplayEffect.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-09
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GE/MGSDamageGameplayEffect.h"

#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/MGSGameplayTags.h"

UMGSDamageGameplayEffect::UMGSDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UCharacterAttributeSet::GetCurrentHpAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataTag = MGSGameplayTags::Data_Damage;
	DamageModifier.ModifierMagnitude = SetByCallerMagnitude;

	Modifiers.Add(DamageModifier);
}
