/*
 * 파일명: WeaponAttributeSet.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자:  장대한
 * 수정일:  2026-03-05
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

	// 현재 탄약
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentMagazineAmmo)

	// 최대 탄약
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxMagazineAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxMagazineAmmo)

	// 현재 탄창
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData CurrentCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentCarriedAmmo)

	// 최대 탄창
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	FGameplayAttributeData MaxCarriedAmmo;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxCarriedAmmo)

	// 사거리
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData FireRange;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireRange)

	// 기본 데미지
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData BaseDamage;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseDamage)

	// 연사 간격
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire")
	FGameplayAttributeData FireInterval;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireInterval)

	// 기본 스프레드(집탄률)
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData BaseSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BaseSpreadRadius)

	// 현재 스프레드
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData CurrentSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentSpreadRadius)

	// 최대 스프레드
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData MaxSpreadRadius;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxSpreadRadius)

	// 스프레드 증가율
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Fire|Spread")
	FGameplayAttributeData SpreadRadiusIncreasePerShot;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SpreadRadiusIncreasePerShot)

	// 조준 줌인 FOV
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	FGameplayAttributeData AimFOV;
	WEAPON_ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AimFOV)
	
};
