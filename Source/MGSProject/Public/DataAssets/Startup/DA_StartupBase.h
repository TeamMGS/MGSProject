/*
 * 파일명 : DA_Startup.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-04
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
	// ASC에 부여된 직후 자동으로 발동할 시작 어빌리티 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ActivateOnGivenAbilities;
	
	// 상황/입력 반응형으로 사용하는 시작 어빌리티 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UBaseGameplayAbility>> ReactiveAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UGameplayEffect>> StartupGameplayEffects;
	
	// 전달받은 어빌리티 배열을 ASC에 부여
	void GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs, UMGSAbilitySystemComponent* InASC, int32 Level);
	
};


