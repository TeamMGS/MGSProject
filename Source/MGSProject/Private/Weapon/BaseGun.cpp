/*
 * 파일명: BaseGun.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#include "Weapon/BaseGun.h"

#include "Characters/BaseCharacter.h"
#include "DataAssets/Weapon/DA_WeaponDefinition.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "MGSStructType.h"


void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	MaxMagazineAmmo = GetDefinitionMaxMagazineAmmo();
	CurrentMagazineAmmo = FMath::Clamp(GetDefinitionStartMagazineAmmo(), 0, MaxMagazineAmmo);
	CarriedAmmo = FMath::Clamp(GetDefinitionStartCarriedAmmo(), 0, GetDefinitionMaxCarriedAmmo());
	FireRange = GetDefinitionFireRange();
	BaseDamage = GetDefinitionBaseDamage();
	FireInterval = GetDefinitionFireInterval();
	BaseSpreadRadius = GetDefinitionBaseSpreadRadius();
	MaxSpreadRadius = GetDefinitionMaxSpreadRadius();
	SpreadRadiusIncreasePerShot = GetDefinitionSpreadRadiusIncreasePerShot();
	AimFOV = GetDefinitionAimFOV();
	AimCameraSocketOffset = GetDefinitionAimCameraSocketOffset();

	CurrentMagazineAmmo = FMath::Clamp(CurrentMagazineAmmo, 0, MaxMagazineAmmo);
	CarriedAmmo = FMath::Max(CarriedAmmo, 0);
}

bool ABaseGun::CanFire() const
{
	return GetCurrentMagazineAmmo() > 0;
}

bool ABaseGun::ConsumeAmmo(int32 AmmoToConsume)
{
	if (AmmoToConsume <= 0)
	{
		return false;
	}

	if (UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSetMutable())
	{
		const int32 CurrentAmmo = GetCurrentMagazineAmmo();
		if (CurrentAmmo < AmmoToConsume)
		{
			return false;
		}

		WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(CurrentAmmo - AmmoToConsume));
		return true;
	}

	if (CurrentMagazineAmmo < AmmoToConsume)
	{
		return false;
	}

	CurrentMagazineAmmo -= AmmoToConsume;
	return true;
}

bool ABaseGun::CanReload() const
{
	if (GetCurrentMagazineAmmo() >= GetMaxMagazineAmmo())
	{
		return false;
	}

	return GetCarriedAmmo() > 0;
}

int32 ABaseGun::ReloadAmmo()
{
	if (!CanReload())
	{
		return 0;
	}

	const int32 NeedAmmo = GetMaxMagazineAmmo() - GetCurrentMagazineAmmo();
	const int32 ReloadedAmmo = FMath::Min(NeedAmmo, GetCarriedAmmo());

	if (UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSetMutable())
	{
		WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(GetCurrentMagazineAmmo() + ReloadedAmmo));
		WeaponAttributeSet->SetCurrentCarriedAmmo(static_cast<float>(GetCarriedAmmo() - ReloadedAmmo));
		return ReloadedAmmo;
	}

	CurrentMagazineAmmo += ReloadedAmmo;
	CarriedAmmo -= ReloadedAmmo;
	return ReloadedAmmo;
}

int32 ABaseGun::GetCurrentMagazineAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		const int32 MaxAmmo = GetMaxMagazineAmmo();
		return FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentMagazineAmmo()), 0, MaxAmmo);
	}

	return FMath::Clamp(CurrentMagazineAmmo, 0, GetMaxMagazineAmmo());
}

int32 ABaseGun::GetMaxMagazineAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(1, FMath::RoundToInt(WeaponAttributeSet->GetMaxMagazineAmmo()));
	}

	return GetDefinitionMaxMagazineAmmo();
}

int32 ABaseGun::GetCarriedAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		const int32 MaxCarried = FMath::Max(0, FMath::RoundToInt(WeaponAttributeSet->GetMaxCarriedAmmo()));
		return FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentCarriedAmmo()), 0, MaxCarried);
	}

	return FMath::Max(0, CarriedAmmo);
}

float ABaseGun::GetFireRange() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(100.f, WeaponAttributeSet->GetFireRange());
	}

	return GetDefinitionFireRange();
}

float ABaseGun::GetBaseDamage() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetBaseDamage());
	}

	return GetDefinitionBaseDamage();
}

float ABaseGun::GetFireInterval() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.01f, WeaponAttributeSet->GetFireInterval());
	}

	return GetDefinitionFireInterval();
}

float ABaseGun::GetBaseSpreadRadius() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetBaseSpreadRadius());
	}

	return GetDefinitionBaseSpreadRadius();
}

float ABaseGun::GetMaxSpreadRadius() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetMaxSpreadRadius());
	}

	return GetDefinitionMaxSpreadRadius();
}

float ABaseGun::GetSpreadRadiusIncreasePerShot() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetSpreadRadiusIncreasePerShot());
	}

	return GetDefinitionSpreadRadiusIncreasePerShot();
}

float ABaseGun::GetAimFOV() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Clamp(WeaponAttributeSet->GetAimFOV(), 10.f, 170.f);
	}

	return GetDefinitionAimFOV();
}

FVector ABaseGun::GetAimCameraSocketOffset() const
{
	return GetDefinitionAimCameraSocketOffset();
}

bool ABaseGun::InitializeWeaponAttributes(UWeaponAttributeSet* WeaponAttributeSet) const
{
	if (!WeaponAttributeSet)
	{
		return false;
	}

	const int32 NewMaxMagazineAmmo = GetDefinitionMaxMagazineAmmo();
	const int32 NewMaxCarriedAmmo = GetDefinitionMaxCarriedAmmo();
	const int32 NewCurrentMagazineAmmo = FMath::Clamp(GetDefinitionStartMagazineAmmo(), 0, NewMaxMagazineAmmo);
	const int32 NewCurrentCarriedAmmo = FMath::Clamp(GetDefinitionStartCarriedAmmo(), 0, NewMaxCarriedAmmo);

	WeaponAttributeSet->SetMaxMagazineAmmo(static_cast<float>(NewMaxMagazineAmmo));
	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(NewCurrentMagazineAmmo));
	WeaponAttributeSet->SetMaxCarriedAmmo(static_cast<float>(NewMaxCarriedAmmo));
	WeaponAttributeSet->SetCurrentCarriedAmmo(static_cast<float>(NewCurrentCarriedAmmo));
	WeaponAttributeSet->SetFireRange(GetDefinitionFireRange());
	WeaponAttributeSet->SetBaseDamage(GetDefinitionBaseDamage());
	WeaponAttributeSet->SetFireInterval(GetDefinitionFireInterval());
	WeaponAttributeSet->SetBaseSpreadRadius(GetDefinitionBaseSpreadRadius());
	WeaponAttributeSet->SetMaxSpreadRadius(GetDefinitionMaxSpreadRadius());
	WeaponAttributeSet->SetSpreadRadiusIncreasePerShot(GetDefinitionSpreadRadiusIncreasePerShot());
	WeaponAttributeSet->SetAimFOV(GetDefinitionAimFOV());
	return true;
}

void ABaseGun::ApplyRuntimeState(UWeaponAttributeSet* WeaponAttributeSet, const FWeaponRuntimeState& RuntimeState) const
{
	if (!WeaponAttributeSet)
	{
		return;
	}

	const int32 MaxMag = FMath::Max(1, FMath::RoundToInt(WeaponAttributeSet->GetMaxMagazineAmmo()));
	const int32 MaxCarry = FMath::Max(0, FMath::RoundToInt(WeaponAttributeSet->GetMaxCarriedAmmo()));
	const int32 ClampedCurrentMag = FMath::Clamp(RuntimeState.CurrentMagazineAmmo, 0, MaxMag);
	const int32 ClampedCurrentCarry = FMath::Clamp(RuntimeState.CurrentCarriedAmmo, 0, MaxCarry);

	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(ClampedCurrentMag));
	WeaponAttributeSet->SetCurrentCarriedAmmo(static_cast<float>(ClampedCurrentCarry));
}

FWeaponRuntimeState ABaseGun::MakeRuntimeState(const UWeaponAttributeSet* WeaponAttributeSet) const
{
	if (!WeaponAttributeSet)
	{
		return MakeDefaultRuntimeState();
	}

	FWeaponRuntimeState RuntimeState;
	const int32 MaxMag = FMath::Max(1, FMath::RoundToInt(WeaponAttributeSet->GetMaxMagazineAmmo()));
	const int32 MaxCarry = FMath::Max(0, FMath::RoundToInt(WeaponAttributeSet->GetMaxCarriedAmmo()));
	RuntimeState.CurrentMagazineAmmo = FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentMagazineAmmo()), 0, MaxMag);
	RuntimeState.CurrentCarriedAmmo = FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentCarriedAmmo()), 0, MaxCarry);
	return RuntimeState;
}

FWeaponRuntimeState ABaseGun::MakeDefaultRuntimeState() const
{
	FWeaponRuntimeState RuntimeState;
	const int32 MaxMag = GetDefinitionMaxMagazineAmmo();
	const int32 MaxCarry = GetDefinitionMaxCarriedAmmo();
	RuntimeState.CurrentMagazineAmmo = FMath::Clamp(GetDefinitionStartMagazineAmmo(), 0, MaxMag);
	RuntimeState.CurrentCarriedAmmo = FMath::Clamp(GetDefinitionStartCarriedAmmo(), 0, MaxCarry);
	return RuntimeState;
}

const UWeaponAttributeSet* ABaseGun::GetWeaponAttributeSet() const
{
	const ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetWeaponAttributeSet() : nullptr;
}

UWeaponAttributeSet* ABaseGun::GetWeaponAttributeSetMutable() const
{
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetWeaponAttributeSet() : nullptr;
}

int32 ABaseGun::GetDefinitionMaxMagazineAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(1, WeaponDefinition->MaxMagazineAmmo);
	}

	return FMath::Max(1, MaxMagazineAmmo);
}

int32 ABaseGun::GetDefinitionStartMagazineAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->StartMagazineAmmo);
	}

	return FMath::Max(0, CurrentMagazineAmmo);
}

int32 ABaseGun::GetDefinitionMaxCarriedAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->MaxCarriedAmmo);
	}

	return FMath::Max(0, CarriedAmmo);
}

int32 ABaseGun::GetDefinitionStartCarriedAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->StartCarriedAmmo);
	}

	return FMath::Max(0, CarriedAmmo);
}

float ABaseGun::GetDefinitionFireRange() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(100.f, WeaponDefinition->FireRange);
	}

	return FMath::Max(100.f, FireRange);
}

float ABaseGun::GetDefinitionBaseDamage() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->BaseDamage);
	}

	return FMath::Max(0.f, BaseDamage);
}

float ABaseGun::GetDefinitionFireInterval() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.01f, WeaponDefinition->FireInterval);
	}

	return FMath::Max(0.01f, FireInterval);
}

float ABaseGun::GetDefinitionBaseSpreadRadius() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->BaseSpreadRadius);
	}

	return FMath::Max(0.f, BaseSpreadRadius);
}

float ABaseGun::GetDefinitionMaxSpreadRadius() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->MaxSpreadRadius);
	}

	return FMath::Max(0.f, MaxSpreadRadius);
}

float ABaseGun::GetDefinitionSpreadRadiusIncreasePerShot() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->SpreadRadiusIncreasePerShot);
	}

	return FMath::Max(0.f, SpreadRadiusIncreasePerShot);
}

float ABaseGun::GetDefinitionAimFOV() const
{
	if (WeaponDefinition)
	{
		return FMath::Clamp(WeaponDefinition->AimFOV, 10.f, 170.f);
	}

	return FMath::Clamp(AimFOV, 10.f, 170.f);
}

FVector ABaseGun::GetDefinitionAimCameraSocketOffset() const
{
	if (WeaponDefinition)
	{
		return WeaponDefinition->AimCameraSocketOffset;
	}

	return AimCameraSocketOffset;
}


