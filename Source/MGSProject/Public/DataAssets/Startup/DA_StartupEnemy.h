/*
 * 파일명 : DA_StartupEnemy.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
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
	// ASC에 자동 활성화/반응형 GA 및 자동 활성화 GE 부여
	virtual void GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level = 1) override;
	
private:
	// 적이 시작부터 보유하고 BT/코드에서 활성화할 명령형 GA 목록
	UPROPERTY(EditDefaultsOnly, Category = "StartupData")
	TArray<TSubclassOf<UEnemyGameplayAbility>> EnemyActionAbilities;

};
