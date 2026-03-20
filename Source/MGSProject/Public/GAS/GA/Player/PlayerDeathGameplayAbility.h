/*
 * 파일명 : PlayerDeathGameplayAbility.h
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerDeathGameplayAbility.generated.h"

class UAnimMontage;

UCLASS()
class MGSPROJECT_API UPlayerDeathGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerDeathGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	// 몽타쥬 종료시 호출될 함수
	UFUNCTION()
	void OnDeathMontageFinished();
	
	// 에디터에서 할당할 죽음 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;
};
