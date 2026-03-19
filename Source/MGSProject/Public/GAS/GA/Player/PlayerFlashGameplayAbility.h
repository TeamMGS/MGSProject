/*
 * 파일명 : PlayerFlashGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-19
 * 수정자 : 장대한
 * 수정일 : 2026-03-19
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerFlashGameplayAbility.generated.h"

UCLASS()
class MGSPROJECT_API UPlayerFlashGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPlayerFlashGameplayAbility();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
};
