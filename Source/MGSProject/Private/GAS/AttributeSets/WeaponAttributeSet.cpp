/*
 * 파일명 : WeaponAttributeSet.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/AttributeSets/WeaponAttributeSet.h"


UWeaponAttributeSet::UWeaponAttributeSet()
{
	InitMaxMagazineAmmo(30.0f);
	InitCurrentMagazineAmmo(30.0f);
	InitMaxCarriedAmmo(210.f);
	InitCurrentCarriedAmmo(210.f);
	InitBaseDamage(20.0f);
	InitAimReferenceDistance(12000.0f);
	InitFireInterval(0.12f);
	InitBaseSpreadRadius(0.0f);
	InitCurrentSpreadRadius(0.0f);
	InitMaxSpreadRadius(50.0f);
	InitSpreadRadiusIncreasePerShot(2.0f);
	InitAimFOV(65.0f);
}
