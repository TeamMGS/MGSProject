/*
 * 파일명 : EnemyEquipPrimaryWeaponGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-12
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/EnemyGameplayAbility.h"
#include "EnemyEquipPrimaryWeaponGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UEnemyEquipPrimaryWeaponGameplayAbility : public UEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyEquipPrimaryWeaponGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
};
