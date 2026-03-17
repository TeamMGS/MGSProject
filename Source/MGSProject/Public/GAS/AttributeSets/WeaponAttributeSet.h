/*
 * 파일명 : WeaponAttributeSet.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "WeaponAttributeSet.generated.h"

#define WEAPON_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MGSPROJECT_API UWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWeaponAttributeSet();

	// Ammo
	// Max Ammo
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxMagazineAmmo)
	// Current Ammo
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentMagazineAmmo)
	// Max Carried Ammo
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxCarriedAmmo)
	// Current Carried Ammo
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentCarriedAmmo)

	// Fire
	// Base Damage
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData BaseDamage;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseDamage)
	// Aim Reference Distance
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData AimReferenceDistance;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AimReferenceDistance)
	// Fire Interval
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData FireInterval;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireInterval)

	// Spread
	// Base Spread
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData BaseSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseSpreadRadius)
	// Max Spread
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData MaxSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxSpreadRadius)
	// Current Spread
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData CurrentSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentSpreadRadius)
	// Spread Increase
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData SpreadRadiusIncreasePerShot;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SpreadRadiusIncreasePerShot)

	// Aim
	// Aim FOV
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	FGameplayAttributeData AimFOV;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AimFOV)
	
};
