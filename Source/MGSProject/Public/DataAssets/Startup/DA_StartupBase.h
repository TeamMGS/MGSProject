/*
 * 파일명 : DA_Startup.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
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
	virtual void GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level = 1);
	
protected:
	// 부여받은 직후 활성화 되는 주어진 능력들
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ActivateOnGivenAbilities;
	
	// 특정 조건에 대한 적중반응이나 사망같은 능력들
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ReactiveAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UGameplayEffect>> StartupGameplayEffects;
	
	// 능력 부여
	void GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs, UMGSAbilitySystemComponent* InASC, int32 Level);
	
};
