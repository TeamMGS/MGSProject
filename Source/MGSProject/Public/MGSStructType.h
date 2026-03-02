/*
 * 파일명 : MGSStructType.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "GameplayTagContainer.h"
#include "MGSStructType.generated.h"

class UPlayerGameplayAbility;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FPlayerAbilitySet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	// 부여가능한 능력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UPlayerGameplayAbility> AbilityToGrant;
	
	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct FPlayerWeaponData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> WeaponInputMappingContext;
	
	// 무기 능력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FPlayerAbilitySet> WeaponAbilities;
	
};