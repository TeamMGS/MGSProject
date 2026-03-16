/*
 * 파일명 : EnemyCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"

UCLASS()
class MGSPROJECT_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipSecondaryWeapon();

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetPrimaryWeaponTag() const { return PrimaryWeaponTag; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetSecondaryWeaponTag() const { return SecondaryWeaponTag; }

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag PrimaryWeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag SecondaryWeaponTag;
	
};
