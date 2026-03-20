/*
 * 파일명 : PlayerInteractGameplayAbility.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-20
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */
#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerInteractGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class MGSPROJECT_API UPlayerInteractGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPlayerInteractGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnRelease(float TimeHeld); // 키를 뗐을 때

	UFUNCTION()
	void OnSuccess(); // 2초 완료 시
	
	// 에디터에서 할당할 상호작용(집기) 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UAnimMontage> InteractMontage;
private:
	// 현재 상호작용 중인 대상 저장
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
};
