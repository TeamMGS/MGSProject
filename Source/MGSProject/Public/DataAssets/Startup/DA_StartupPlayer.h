/*
 * 파일명 : DA_StartupPlayer.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "DA_StartupPlayer.generated.h"

struct FPlayerAbilitySet;

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
