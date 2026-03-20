/*
 * 파일명 : BaseGun.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "Weapon/BaseGun.h"

#include "Characters/BaseCharacter.h"
#include "DataAssets/Weapon/DA_WeaponDefinition.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GAS/GE/MGSDamageGameplayEffect.h"
#include "MGSStructType.h"
#include "Projectiles/BaseBullet.h"

void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	checkf(WeaponDefinition, TEXT("%s has no WeaponDefinition assigned."), *GetName());
}

const UDA_WeaponDefinition& ABaseGun::GetWeaponDefinitionChecked() const
{
	checkf(WeaponDefinition, TEXT("%s has no WeaponDefinition assigned."), *GetName());
	return *WeaponDefinition;
}

bool ABaseGun::CanFire() const
{
	if (!ensureMsgf(GetWeaponAttributeSet(), TEXT("WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return false;
	}

	// 현재 탄약이 있으면 발사 가능
	return GetCurrentMagazineAmmo() > 0;
}

bool ABaseGun::ConsumeAmmo(int32 AmmoToConsume)
{
	if (AmmoToConsume <= 0)
	{
		return false;
	}

	UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSetMutable();
	if (!ensureMsgf(WeaponAttributeSet, TEXT("ConsumeAmmo failed because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return false;
	}

	const int32 CurrentAmmo = GetCurrentMagazineAmmo();
	if (CurrentAmmo < AmmoToConsume)
	{
		return false;
	}

	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(CurrentAmmo - AmmoToConsume));
	return true;
}

bool ABaseGun::RefundAmmo(int32 AmmoToRefund)
{
	if (AmmoToRefund <= 0)
	{
		return false;
	}

	UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSetMutable();
	if (!ensureMsgf(WeaponAttributeSet, TEXT("RefundAmmo failed because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return false;
	}

	const int32 MaxAmmo = GetMaxMagazineAmmo();
	const int32 CurrentAmmo = GetCurrentMagazineAmmo();
	if (CurrentAmmo >= MaxAmmo)
	{
		return false;
	}

	const int32 RefundedAmmo = FMath::Min(AmmoToRefund, MaxAmmo - CurrentAmmo);
	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(CurrentAmmo + RefundedAmmo));
	return RefundedAmmo > 0;
}

bool ABaseGun::CanReload() const
{
	if (!ensureMsgf(GetWeaponAttributeSet(), TEXT("WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return false;
	}

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

	UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSetMutable();
	if (!ensureMsgf(WeaponAttributeSet, TEXT("ReloadAmmo failed because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return 0;
	}

	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(GetCurrentMagazineAmmo() + ReloadedAmmo));
	WeaponAttributeSet->SetCurrentCarriedAmmo(static_cast<float>(GetCarriedAmmo() - ReloadedAmmo));
	return ReloadedAmmo;
}

int32 ABaseGun::GetCurrentMagazineAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		const int32 MaxAmmo = GetMaxMagazineAmmo();
		return FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentMagazineAmmo()), 0, MaxAmmo);
	}

	ensureMsgf(false, TEXT("GetCurrentMagazineAmmo is reading definition fallback because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner()));
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	const int32 MaxAmmo = FMath::Max(1, WeaponDef.MaxMagazineAmmo);
	const int32 StartAmmo = FMath::Max(0, WeaponDef.StartMagazineAmmo);
	return FMath::Clamp(StartAmmo, 0, MaxAmmo);
}

int32 ABaseGun::GetMaxMagazineAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(1, FMath::RoundToInt(WeaponAttributeSet->GetMaxMagazineAmmo()));
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(1, WeaponDef.MaxMagazineAmmo);
}

int32 ABaseGun::GetCarriedAmmo() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		const int32 MaxCarried = FMath::Max(0, FMath::RoundToInt(WeaponAttributeSet->GetMaxCarriedAmmo()));
		return FMath::Clamp(FMath::RoundToInt(WeaponAttributeSet->GetCurrentCarriedAmmo()), 0, MaxCarried);
	}

	ensureMsgf(false, TEXT("GetCarriedAmmo is reading definition fallback because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner()));
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	const int32 MaxCarried = FMath::Max(0, WeaponDef.MaxCarriedAmmo);
	const int32 StartCarried = FMath::Max(0, WeaponDef.StartCarriedAmmo);
	return FMath::Clamp(StartCarried, 0, MaxCarried);
}

float ABaseGun::GetAimReferenceDistance() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(100.f, WeaponAttributeSet->GetAimReferenceDistance());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(100.f, WeaponDef.AimReferenceDistance);
}

float ABaseGun::GetBaseDamage() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetBaseDamage());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.BaseDamage);
}

TSubclassOf<UGameplayEffect> ABaseGun::GetDamageGameplayEffectClass() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return WeaponDef.DamageGameplayEffectClass
		? WeaponDef.DamageGameplayEffectClass
		: TSubclassOf<UGameplayEffect>(UMGSDamageGameplayEffect::StaticClass());
}

float ABaseGun::GetFireInterval() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.01f, WeaponAttributeSet->GetFireInterval());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.01f, WeaponDef.FireInterval);
}

float ABaseGun::GetBaseSpreadRadius() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetBaseSpreadRadius());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.BaseSpreadRadius);
}

float ABaseGun::GetMaxSpreadRadius() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetMaxSpreadRadius());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.MaxSpreadRadius);
}

float ABaseGun::GetSpreadRadiusIncreasePerShot() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(0.f, WeaponAttributeSet->GetSpreadRadiusIncreasePerShot());
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.SpreadRadiusIncreasePerShot);
}

float ABaseGun::GetAimFOV() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Clamp(WeaponAttributeSet->GetAimFOV(), 10.f, 170.f);
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Clamp(WeaponDef.AimFOV, 10.f, 170.f);
}

FVector ABaseGun::GetAimCameraSocketOffset() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return WeaponDef.AimCameraSocketOffset;
}

float ABaseGun::GetRecoilPitchPerShot() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.RecoilPitchPerShot);
}

float ABaseGun::GetRecoilYawPerShotMin() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return WeaponDef.RecoilYawPerShotMin;
}

float ABaseGun::GetRecoilYawPerShotMax() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return WeaponDef.RecoilYawPerShotMax;
}

float ABaseGun::GetRecoilADSScale() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.RecoilADSScale);
}

TSubclassOf<UCameraShakeBase> ABaseGun::GetFireCameraShakeClass() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return WeaponDef.FireCameraShakeClass;
}

float ABaseGun::GetFireCameraShakeScale() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	return FMath::Max(0.f, WeaponDef.FireCameraShakeScale);
}

TSubclassOf<ABaseProjectile> ABaseGun::GetProjectileClass() const
{
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	if (WeaponDef.ProjectileClass)
	{
		return WeaponDef.ProjectileClass;
	}

	ensureMsgf(false, TEXT("%s has no ProjectileClass in WeaponDefinition. Using ABaseBullet fallback."), *GetName());
	return ABaseBullet::StaticClass();
}

bool ABaseGun::InitializeWeaponAttributes(UWeaponAttributeSet* WeaponAttributeSet) const
{
	if (!WeaponAttributeSet)
	{
		return false;
	}

	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	const int32 NewMaxMagazineAmmo = FMath::Max(1, WeaponDef.MaxMagazineAmmo);
	const int32 NewMaxCarriedAmmo = FMath::Max(0, WeaponDef.MaxCarriedAmmo);
	const int32 NewCurrentMagazineAmmo = FMath::Clamp(FMath::Max(0, WeaponDef.StartMagazineAmmo), 0, NewMaxMagazineAmmo);
	const int32 NewCurrentCarriedAmmo = FMath::Clamp(FMath::Max(0, WeaponDef.StartCarriedAmmo), 0, NewMaxCarriedAmmo);
	const float NewAimReferenceDistance = FMath::Max(100.f, WeaponDef.AimReferenceDistance);
	const float NewBaseDamage = FMath::Max(0.f, WeaponDef.BaseDamage);
	const float NewFireInterval = FMath::Max(0.01f, WeaponDef.FireInterval);
	const float NewBaseSpreadRadius = FMath::Max(0.f, WeaponDef.BaseSpreadRadius);
	const float NewMaxSpreadRadius = FMath::Max(0.f, WeaponDef.MaxSpreadRadius);
	const float NewSpreadRadiusIncreasePerShot = FMath::Max(0.f, WeaponDef.SpreadRadiusIncreasePerShot);
	const float NewAimFOV = FMath::Clamp(WeaponDef.AimFOV, 10.f, 170.f);

	WeaponAttributeSet->SetMaxMagazineAmmo(static_cast<float>(NewMaxMagazineAmmo));
	WeaponAttributeSet->SetCurrentMagazineAmmo(static_cast<float>(NewCurrentMagazineAmmo));
	WeaponAttributeSet->SetMaxCarriedAmmo(static_cast<float>(NewMaxCarriedAmmo));
	WeaponAttributeSet->SetCurrentCarriedAmmo(static_cast<float>(NewCurrentCarriedAmmo));
	WeaponAttributeSet->SetAimReferenceDistance(NewAimReferenceDistance);
	WeaponAttributeSet->SetBaseDamage(NewBaseDamage);
	WeaponAttributeSet->SetFireInterval(NewFireInterval);
	WeaponAttributeSet->SetBaseSpreadRadius(NewBaseSpreadRadius);
	WeaponAttributeSet->SetCurrentSpreadRadius(NewBaseSpreadRadius);
	WeaponAttributeSet->SetMaxSpreadRadius(NewMaxSpreadRadius);
	WeaponAttributeSet->SetSpreadRadiusIncreasePerShot(NewSpreadRadiusIncreasePerShot);
	WeaponAttributeSet->SetAimFOV(NewAimFOV);
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
	const UDA_WeaponDefinition& WeaponDef = GetWeaponDefinitionChecked();
	FWeaponRuntimeState RuntimeState;
	const int32 MaxMag = FMath::Max(1, WeaponDef.MaxMagazineAmmo);
	const int32 MaxCarry = FMath::Max(0, WeaponDef.MaxCarriedAmmo);
	const int32 StartMag = FMath::Max(0, WeaponDef.StartMagazineAmmo);
	const int32 StartCarry = FMath::Max(0, WeaponDef.StartCarriedAmmo);
	RuntimeState.CurrentMagazineAmmo = FMath::Clamp(StartMag, 0, MaxMag);
	RuntimeState.CurrentCarriedAmmo = FMath::Clamp(StartCarry, 0, MaxCarry);
	return RuntimeState;
}

FVector ABaseGun::GetMuzzleLocation() const
{
	return WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
}

USoundBase* ABaseGun::GetFireSound() const
{
	return WeaponDefinition->FireSound;
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
