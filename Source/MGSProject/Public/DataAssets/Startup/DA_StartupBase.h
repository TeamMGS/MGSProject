/*
 * 파일명 : DA_StartupBase.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_StartupBase.generated.h"

class UGameplayEffect;
class UMGSAbilitySystemComponent;
class UBaseGameplayAbility;

UCLASS()
class MGSPROJECT_API UDA_StartupBase : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// ASC에 자동 활성화/반응형 GA 및 자동 활성화 GE 부여
	virtual void GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level = 1);
	
protected:
	// 자동 활성화 GA 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ActivateOnGivenAbilities;
	
	// 반응형 GA 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ReactiveAbilities;
	
	// 자동 활성화 GE 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UGameplayEffect>> StartupGameplayEffects;
	
	// ASC에 GA 목록 부여
	void GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs, UMGSAbilitySystemComponent* InASC, int32 Level);
	
};
