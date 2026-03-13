/*
 * 파일명 : MGSPlayerStatusWidget.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#include "UI/MGSPlayerStatusWidget.h"

void UMGSPlayerStatusWidget::UpdateHealth(float InCurrentHp, float InMaxHp)
{
	CurrentHp = FMath::Max(0.f, InCurrentHp);
	MaxHp = FMath::Max(0.f, InMaxHp);
	BP_OnHealthUpdated(CurrentHp, MaxHp, GetHealthPercent());
}

void UMGSPlayerStatusWidget::UpdateAmmo(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo)
{
	CurrentMagazineAmmo = FMath::Max(0, InCurrentMagazineAmmo);
	MaxMagazineAmmo = FMath::Max(0, InMaxMagazineAmmo);
	CarriedAmmo = FMath::Max(0, InCarriedAmmo);
	BP_OnAmmoUpdated(CurrentMagazineAmmo, MaxMagazineAmmo, CarriedAmmo);
}

void UMGSPlayerStatusWidget::UpdateSpread(float InCurrentSpreadRadius, float InMaxSpreadRadius)
{
	CurrentSpreadRadius = FMath::Max(0.f, InCurrentSpreadRadius);
	MaxSpreadRadius = FMath::Max(0.f, InMaxSpreadRadius);
	BP_OnSpreadUpdated(CurrentSpreadRadius, MaxSpreadRadius, GetSpreadPercent());
}

void UMGSPlayerStatusWidget::UpdateWeaponInfo(bool bInVisible, UTexture2D* InWeaponInfoImage)
{
	bWeaponInfoVisible = bInVisible;
	CurrentWeaponInfoImage = InWeaponInfoImage;
	BP_OnWeaponInfoVisibilityChanged(bWeaponInfoVisible);
}

void UMGSPlayerStatusWidget::UpdatePickupWeaponPrompt(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage)
{
	bPickupWeaponPromptVisible = bInVisible;
	PickupWeaponPromptName = InWeaponName;
	PickupWeaponPromptImage = InWeaponInfoImage;
	BP_OnPickupWeaponPromptUpdated(bPickupWeaponPromptVisible, PickupWeaponPromptName, PickupWeaponPromptImage.Get());
}

float UMGSPlayerStatusWidget::GetHealthPercent() const
{
	if (MaxHp <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentHp / MaxHp, 0.f, 1.f);
}

float UMGSPlayerStatusWidget::GetSpreadPercent() const
{
	if (MaxSpreadRadius <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentSpreadRadius / MaxSpreadRadius, 0.f, 1.f);
}

UTexture2D* UMGSPlayerStatusWidget::GetCurrentWeaponInfoImage() const
{
	return CurrentWeaponInfoImage.Get();
}

FText UMGSPlayerStatusWidget::GetPickupWeaponPromptName() const
{
	return PickupWeaponPromptName;
}

UTexture2D* UMGSPlayerStatusWidget::GetPickupWeaponPromptImage() const
{
	return PickupWeaponPromptImage.Get();
}

void UMGSPlayerStatusWidget::SetWeaponInfoVisible(bool bInVisible)
{
	UpdateWeaponInfo(bInVisible, CurrentWeaponInfoImage.Get());
}
