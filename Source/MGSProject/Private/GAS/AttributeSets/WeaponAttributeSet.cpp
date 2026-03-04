/*
 * 파일명: WeaponAttributeSet.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#include "GAS/AttributeSets/WeaponAttributeSet.h"


UWeaponAttributeSet::UWeaponAttributeSet()
{
	InitCurrentMagazineAmmo(30.f);
	InitMaxMagazineAmmo(30.f);
	InitCurrentCarriedAmmo(120.f);
	InitMaxCarriedAmmo(120.f);
	InitFireRange(12000.f);
	InitBaseDamage(20.f);
	InitFireInterval(0.12f);
	InitBaseSpreadRadius(0.f);
	InitMaxSpreadRadius(120.f);
	InitSpreadRadiusIncreasePerShot(6.f);
	InitAimFOV(65.f);
}


