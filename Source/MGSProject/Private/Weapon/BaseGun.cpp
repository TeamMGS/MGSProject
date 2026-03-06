/*
 * 파일명: BaseGun.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자:  장대한
 * 수정일:  2026-03-05
 */

#include "Weapon/BaseGun.h"

#include "Characters/BaseCharacter.h"
#include "DataAssets/Weapon/DA_WeaponDefinition.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "MGSStructType.h"
#include "Projectiles/BaseBullet.h"

namespace
{
	constexpr int32 DefaultMaxMagazineAmmo = 30;
	constexpr int32 DefaultStartMagazineAmmo = 30;
	constexpr int32 DefaultMaxCarriedAmmo = 120;
	constexpr int32 DefaultStartCarriedAmmo = 120;
	constexpr float DefaultAimReferenceDistance = 12000.f;
	constexpr float DefaultBaseDamage = 20.f;
	constexpr float DefaultFireInterval = 0.12f;
	constexpr float DefaultBaseSpreadRadius = 0.f;
	constexpr float DefaultMaxSpreadRadius = 120.f;
	constexpr float DefaultSpreadRadiusIncreasePerShot = 6.f;
	constexpr float DefaultAimFOV = 65.f;
	const FVector DefaultAimCameraSocketOffset = FVector(0.f, 55.f, 12.f);
	constexpr float DefaultRecoilPitchPerShot = 0.8f;
	constexpr float DefaultRecoilYawPerShotMin = -0.25f;
	constexpr float DefaultRecoilYawPerShotMax = 0.25f;
	constexpr float DefaultRecoilADSScale = 0.7f;
	constexpr float DefaultFireCameraShakeScale = 1.0f;
}

void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(WeaponDefinition, TEXT("%s has no WeaponDefinition assigned."), *GetName());
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
	return FMath::Clamp(GetDefinitionStartMagazineAmmo(), 0, GetDefinitionMaxMagazineAmmo());
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

	ensureMsgf(false, TEXT("GetCarriedAmmo is reading definition fallback because WeaponAttributeSet is missing for %s (owner: %s)."), *GetName(), *GetNameSafe(GetOwner()));
	return FMath::Clamp(GetDefinitionStartCarriedAmmo(), 0, GetDefinitionMaxCarriedAmmo());
}

float ABaseGun::GetAimReferenceDistance() const
{
	if (const UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet())
	{
		return FMath::Max(100.f, WeaponAttributeSet->GetAimReferenceDistance());
	}

	return GetDefinitionAimReferenceDistance();
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

float ABaseGun::GetRecoilPitchPerShot() const
{
	return GetDefinitionRecoilPitchPerShot();
}

float ABaseGun::GetRecoilYawPerShotMin() const
{
	return GetDefinitionRecoilYawPerShotMin();
}

float ABaseGun::GetRecoilYawPerShotMax() const
{
	return GetDefinitionRecoilYawPerShotMax();
}

float ABaseGun::GetRecoilADSScale() const
{
	return GetDefinitionRecoilADSScale();
}

TSubclassOf<UCameraShakeBase> ABaseGun::GetFireCameraShakeClass() const
{
	return GetDefinitionFireCameraShakeClass();
}

float ABaseGun::GetFireCameraShakeScale() const
{
	return GetDefinitionFireCameraShakeScale();
}

TSubclassOf<ABaseProjectile> ABaseGun::GetProjectileClass() const
{
	return GetDefinitionProjectileClass();
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
	WeaponAttributeSet->SetAimReferenceDistance(GetDefinitionAimReferenceDistance());
	WeaponAttributeSet->SetBaseDamage(GetDefinitionBaseDamage());
	WeaponAttributeSet->SetFireInterval(GetDefinitionFireInterval());
	WeaponAttributeSet->SetBaseSpreadRadius(GetDefinitionBaseSpreadRadius());
	WeaponAttributeSet->SetCurrentSpreadRadius(GetDefinitionBaseSpreadRadius());
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

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded MaxMagazineAmmo fallback."), *GetName());
	return DefaultMaxMagazineAmmo;
}

int32 ABaseGun::GetDefinitionStartMagazineAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->StartMagazineAmmo);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded StartMagazineAmmo fallback."), *GetName());
	return DefaultStartMagazineAmmo;
}

int32 ABaseGun::GetDefinitionMaxCarriedAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->MaxCarriedAmmo);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded MaxCarriedAmmo fallback."), *GetName());
	return DefaultMaxCarriedAmmo;
}

int32 ABaseGun::GetDefinitionStartCarriedAmmo() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0, WeaponDefinition->StartCarriedAmmo);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded StartCarriedAmmo fallback."), *GetName());
	return DefaultStartCarriedAmmo;
}

float ABaseGun::GetDefinitionAimReferenceDistance() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(100.f, WeaponDefinition->AimReferenceDistance);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded AimReferenceDistance fallback."), *GetName());
	return DefaultAimReferenceDistance;
}

float ABaseGun::GetDefinitionBaseDamage() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->BaseDamage);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded BaseDamage fallback."), *GetName());
	return DefaultBaseDamage;
}

float ABaseGun::GetDefinitionFireInterval() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.01f, WeaponDefinition->FireInterval);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded FireInterval fallback."), *GetName());
	return DefaultFireInterval;
}

float ABaseGun::GetDefinitionBaseSpreadRadius() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->BaseSpreadRadius);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded BaseSpreadRadius fallback."), *GetName());
	return DefaultBaseSpreadRadius;
}

float ABaseGun::GetDefinitionMaxSpreadRadius() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->MaxSpreadRadius);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded MaxSpreadRadius fallback."), *GetName());
	return DefaultMaxSpreadRadius;
}

float ABaseGun::GetDefinitionSpreadRadiusIncreasePerShot() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->SpreadRadiusIncreasePerShot);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded SpreadRadiusIncreasePerShot fallback."), *GetName());
	return DefaultSpreadRadiusIncreasePerShot;
}

float ABaseGun::GetDefinitionAimFOV() const
{
	if (WeaponDefinition)
	{
		return FMath::Clamp(WeaponDefinition->AimFOV, 10.f, 170.f);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded AimFOV fallback."), *GetName());
	return DefaultAimFOV;
}

FVector ABaseGun::GetDefinitionAimCameraSocketOffset() const
{
	if (WeaponDefinition)
	{
		return WeaponDefinition->AimCameraSocketOffset;
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded AimCameraSocketOffset fallback."), *GetName());
	return DefaultAimCameraSocketOffset;
}

float ABaseGun::GetDefinitionRecoilPitchPerShot() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->RecoilPitchPerShot);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded RecoilPitchPerShot fallback."), *GetName());
	return DefaultRecoilPitchPerShot;
}

float ABaseGun::GetDefinitionRecoilYawPerShotMin() const
{
	if (WeaponDefinition)
	{
		return WeaponDefinition->RecoilYawPerShotMin;
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded RecoilYawPerShotMin fallback."), *GetName());
	return DefaultRecoilYawPerShotMin;
}

float ABaseGun::GetDefinitionRecoilYawPerShotMax() const
{
	if (WeaponDefinition)
	{
		return WeaponDefinition->RecoilYawPerShotMax;
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded RecoilYawPerShotMax fallback."), *GetName());
	return DefaultRecoilYawPerShotMax;
}

float ABaseGun::GetDefinitionRecoilADSScale() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->RecoilADSScale);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded RecoilADSScale fallback."), *GetName());
	return DefaultRecoilADSScale;
}

TSubclassOf<UCameraShakeBase> ABaseGun::GetDefinitionFireCameraShakeClass() const
{
	if (WeaponDefinition)
	{
		return WeaponDefinition->FireCameraShakeClass;
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. CameraShakeClass is empty."), *GetName());
	return nullptr;
}

float ABaseGun::GetDefinitionFireCameraShakeScale() const
{
	if (WeaponDefinition)
	{
		return FMath::Max(0.f, WeaponDefinition->FireCameraShakeScale);
	}

	ensureMsgf(false, TEXT("%s has no WeaponDefinition. Using hardcoded FireCameraShakeScale fallback."), *GetName());
	return DefaultFireCameraShakeScale;
}

TSubclassOf<ABaseProjectile> ABaseGun::GetDefinitionProjectileClass() const
{
	if (WeaponDefinition && WeaponDefinition->ProjectileClass)
	{
		return WeaponDefinition->ProjectileClass;
	}

	ensureMsgf(false, TEXT("%s has no ProjectileClass in WeaponDefinition. Using ABaseBullet fallback."), *GetName());
	return ABaseBullet::StaticClass();
}

