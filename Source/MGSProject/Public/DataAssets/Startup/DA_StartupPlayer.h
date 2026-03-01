/*
 * 파일명 : DA_StartupPlayer.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "DA_StartupPlayer.generated.h"

USTRUCT(BlueprintType)
struct FPlayerAbilitySet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	// 부여가능한 능력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBaseGameplayAbility> AbilityToGrant;
	
	bool IsValid() const;
};

UCLASS()
class MGSPROJECT_API UDA_StartupPlayer : public UDA_StartupBase
{
	GENERATED_BODY()
	
protected:
	virtual void GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level = 1) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "StartupData", meta = (TitleProperty = "InputTag"))
	TArray<FPlayerAbilitySet> PlayerStartupAbilitySets;
	
};
