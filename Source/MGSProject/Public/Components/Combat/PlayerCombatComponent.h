/*
 * 파일명 : PlayerCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "PlayerCombatComponent.generated.h"

UCLASS()
class MGSPROJECT_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipSecondaryWeapon();

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetPrimaryWeaponTag() const { return PrimaryWeaponTag; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetSecondaryWeaponTag() const { return SecondaryWeaponTag; }

private:
	// 1번 키 장착 대상(주무기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag PrimaryWeaponTag;

	// 2번 키 장착 대상(보조무기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag SecondaryWeaponTag;
};


