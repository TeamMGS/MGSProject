/*
 * 파일명 : DA_StartupEnemy.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "DA_StartupEnemy.generated.h"

class UEnemyGameplayAbility;

UCLASS()
class MGSPROJECT_API UDA_StartupEnemy : public UDA_StartupBase
{
	GENERATED_BODY()
	
public:
	virtual void GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level = 1) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UEnemyGameplayAbility>> EnemyCombatAbilities;
	
};
