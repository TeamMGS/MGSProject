/*
 * 파일명: PlayerEquipSecondaryWeaponGameplayAbility.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#pragma once


#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerEquipSecondaryWeaponGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerEquipSecondaryWeaponGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerEquipSecondaryWeaponGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};



