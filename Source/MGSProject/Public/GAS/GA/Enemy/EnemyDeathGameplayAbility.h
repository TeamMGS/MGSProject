/*
 * 파일명 : EnemyDeathGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Enemy/EnemyGameplayAbility.h"
#include "EnemyDeathGameplayAbility.generated.h"

class UAnimMontage;

UCLASS()
class MGSPROJECT_API UEnemyDeathGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyDeathGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	// 몽타쥬 종료시 호출함수
	UFUNCTION()
	void OnFreezeEventReceived(FGameplayEventData Payload);

	// 에디터에서 설정할 죽음 몽타쥬 
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;
};
