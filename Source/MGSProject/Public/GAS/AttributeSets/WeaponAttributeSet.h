/*
 * 파일명: WeaponAttributeSet.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
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

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentMagazineAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxMagazineAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentCarriedAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxCarriedAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData FireRange;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireRange)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData BaseDamage;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData FireInterval;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireInterval)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData BaseSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseSpreadRadius)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData MaxSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxSpreadRadius)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData SpreadRadiusIncreasePerShot;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SpreadRadiusIncreasePerShot)

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	FGameplayAttributeData AimFOV;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AimFOV)
};


