/*
 * 파일명 : MGSAbilitySystemComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MGSAbilitySystemComponent.generated.h"

UCLASS()
class MGSPROJECT_API UMGSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void OnAbilityInputPressed(const FGameplayTag& InputTag);
	void OnAbilityInputReleased(const FGameplayTag& InputTag);
	
};
