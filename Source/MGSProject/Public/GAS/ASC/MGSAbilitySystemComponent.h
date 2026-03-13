/*
 * 파일명 : MGSAbilitySystemComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "MGSAbilitySystemComponent.generated.h"

struct FPlayerAbilitySet;
class UPawnCombatComponent;

UCLASS()
class MGSPROJECT_API UMGSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void OnAbilityInputPressed(const FGameplayTag& InputTag); // 태그 입력 시작 처리
	void OnAbilityInputReleased(const FGameplayTag& InputTag); // 태그 입력 해제 처리
	bool IsAbilityInputTagPressed(const FGameplayTag& InputTag) const; // 현재 눌림 상태 조회
	
	// 무기 장착 시 Ability 추가
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (Level = "1"))
	void GrantWeaponAbilities(const TArray<FPlayerAbilitySet>& WeaponAbilities, int32 Level, TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles);
	
	// 무기 해제 시 정리
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void RemoveGrantedWeaponAbilities(const TArray<FGameplayAbilitySpecHandle>& SpecHandlesToRemove);

private:
	FGameplayTagContainer PressedAbilityInputTags; // 현재 눌려 있는 Ability 입력 태그 집합
	
};
