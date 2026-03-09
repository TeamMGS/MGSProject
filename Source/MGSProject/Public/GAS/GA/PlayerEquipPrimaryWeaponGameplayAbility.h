/*
 * 파일명 : PlayerEquipPrimaryWeaponGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once


#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerEquipPrimaryWeaponGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerEquipPrimaryWeaponGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerEquipPrimaryWeaponGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
};
