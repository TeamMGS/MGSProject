/*
 * 파일명 : CharacterAttributeSet.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MGSPROJECT_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UCharacterAttributeSet();
	
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FGameplayAttributeData CurrentHp;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, CurrentHp)
	
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FGameplayAttributeData MaxHp;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxHp)
		
};


