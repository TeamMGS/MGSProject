/*
 * 파일명 : PlayerHUDPresenterComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Components/UI/PlayerHUDPresenterComponent.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "UI/MGSPlayerStatusWidget.h"
#include "Weapon/BaseWeapon.h"

UPlayerHUDPresenterComponent::UPlayerHUDPresenterComponent()
{
	// Tick 사용 안함
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHUDPresenterComponent::SetPlayerStatusWidgetClass(TSubclassOf<UMGSPlayerStatusWidget> InWidgetClass)
{
	PlayerStatusWidgetClass = InWidgetClass;
}

void UPlayerHUDPresenterComponent::RefreshHUDDataBindings()
{
	AMGSPlayerController* PlayerController = Cast<AMGSPlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalPlayerController())
	{
		return;
	}

	// Create and Add widget
	CreatePlayerStatusWidget();
	// Clear data
	ClearHUDDataBindings();

	const APlayerCharacter* PlayerCharacter = PlayerController->GetPawn<APlayerCharacter>();
	if (!PlayerCharacter)
	{
		return;
	}

	// Cache
	CachedASC = PlayerCharacter->GetMGSAbilitySystemComponent();
	CachedCharacterAttributeSet = PlayerCharacter->GetCharacterAttributeSet();
	CachedWeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet();
	CachedPlayerCombatComponent = PlayerCharacter->GetPlayerCombatComponent();

	if (!CachedASC || !CachedCharacterAttributeSet)
	{
		return;
	}

	// Bind function
	// Character AttributeSet
	// Max hp
	BindAttributeChangedDelegate(UCharacterAttributeSet::GetMaxHpAttribute(), MaxHpChangedHandle, &ThisClass::HandleMaxHpChanged);
	// Current hp
	BindAttributeChangedDelegate(UCharacterAttributeSet::GetCurrentHpAttribute(), CurrentHpChangedHandle, &ThisClass::HandleCurrentHpChanged);
	// Weapon AttributeSet
	if (CachedWeaponAttributeSet)
	{
		// Max ammo
		BindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxMagazineAmmoAttribute(), MaxMagazineAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
		// Current ammo
		BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentMagazineAmmoAttribute(), CurrentMagazineAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
		// Current carried ammo
		BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentCarriedAmmoAttribute(), CurrentCarriedAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
		// Max Spread
		BindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxSpreadRadiusAttribute(), MaxSpreadRadiusChangedHandle, &ThisClass::HandleSpreadAttributeChanged);
		// Current Spread
		BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentSpreadRadiusAttribute(), CurrentSpreadRadiusChangedHandle, &ThisClass::HandleSpreadAttributeChanged);
	}
	// Combat Component
	if (CachedPlayerCombatComponent)
	{
		// Equip
		EquippedWeaponChangedHandle = CachedPlayerCombatComponent->GetOnEquippedWeaponChangedDelegate()
			.AddUObject(this, &ThisClass::HandleEquippedWeaponChanged);
		// Drop
		NearbyDroppedWeaponChangedHandle = CachedPlayerCombatComponent->GetOnNearbyDroppedWeaponChangedDelegate()
			.AddUObject(this, &ThisClass::HandleNearbyDroppedWeaponChanged);
	}

	// Update HUD
	PushInitialHUDValues();
}

void UPlayerHUDPresenterComponent::CreatePlayerStatusWidget()
{
	AMGSPlayerController* PlayerController = Cast<AMGSPlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalPlayerController() || PlayerStatusWidget || !PlayerStatusWidgetClass)
	{
		return;
	}

	PlayerStatusWidget = CreateWidget<UMGSPlayerStatusWidget>(PlayerController, PlayerStatusWidgetClass);
	if (!PlayerStatusWidget)
	{
		return;
	}

	PlayerStatusWidget->AddToPlayerScreen(1);
}

void UPlayerHUDPresenterComponent::ClearHUDDataBindings()
{
	// Unbind function
	// Character AttributeSet
	UnbindAttributeChangedDelegate(UCharacterAttributeSet::GetCurrentHpAttribute(), CurrentHpChangedHandle);
	UnbindAttributeChangedDelegate(UCharacterAttributeSet::GetMaxHpAttribute(), MaxHpChangedHandle);
	// Weapon AttributeSet
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentMagazineAmmoAttribute(), CurrentMagazineAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxMagazineAmmoAttribute(), MaxMagazineAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentCarriedAmmoAttribute(), CurrentCarriedAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentSpreadRadiusAttribute(), CurrentSpreadRadiusChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxSpreadRadiusAttribute(), MaxSpreadRadiusChangedHandle);
	// Combat Component
	if (CachedPlayerCombatComponent && EquippedWeaponChangedHandle.IsValid())
	{
		CachedPlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().Remove(EquippedWeaponChangedHandle);
		EquippedWeaponChangedHandle.Reset();
	}
	if (CachedPlayerCombatComponent && NearbyDroppedWeaponChangedHandle.IsValid())
	{
		CachedPlayerCombatComponent->GetOnNearbyDroppedWeaponChangedDelegate().Remove(NearbyDroppedWeaponChangedHandle);
		NearbyDroppedWeaponChangedHandle.Reset();
	}

	// Cache
	CachedASC = nullptr;
	CachedCharacterAttributeSet = nullptr;
	CachedWeaponAttributeSet = nullptr;
	CachedPlayerCombatComponent = nullptr;
}

void UPlayerHUDPresenterComponent::VisibleMap()
{
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->UpdateMap();
	}
}

void UPlayerHUDPresenterComponent::PushInitialHUDValues() const
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	// Update HP
	UpdateHpOnHUD();
	// Update ammo
	UpdateAmmoOnHUD();
	// Update spread
	UpdateSpreadOnHUD();
	// Update weapon image
	UpdateWeaponInfoVisibility();
	// Update drop weapon
	const ABaseWeapon* NearbyDroppedWeapon = CachedPlayerCombatComponent
		? CachedPlayerCombatComponent->GetNearbyDroppedWeapon()
		: nullptr;
	UpdatePickupWeaponPrompt(NearbyDroppedWeapon);
}

void UPlayerHUDPresenterComponent::UpdateHpOnHUD() const
{
	if (CachedCharacterAttributeSet)
	{
		PlayerStatusWidget->UpdateHealth(CachedCharacterAttributeSet->GetCurrentHp(), CachedCharacterAttributeSet->GetMaxHp());
	}
	else
	{
		PlayerStatusWidget->UpdateHealth(0.0f, 0.0f);
	}
}

void UPlayerHUDPresenterComponent::UpdateAmmoOnHUD() const
{
	if (!PlayerStatusWidget || !CachedWeaponAttributeSet)
	{
		return;
	}

	if (!GetEquippedWeapon())
	{
		return;
	}

	PlayerStatusWidget->UpdateAmmo(
		FMath::Max(0, FMath::RoundToInt(CachedWeaponAttributeSet->GetCurrentMagazineAmmo())),
		FMath::Max(0, FMath::RoundToInt(CachedWeaponAttributeSet->GetMaxMagazineAmmo())),
		FMath::Max(0, FMath::RoundToInt(CachedWeaponAttributeSet->GetCurrentCarriedAmmo())));
}

void UPlayerHUDPresenterComponent::UpdateSpreadOnHUD() const
{
	if (!PlayerStatusWidget || !CachedWeaponAttributeSet)
	{
		return;
	}

	if (!GetEquippedWeapon())
	{
		return;
	}

	PlayerStatusWidget->UpdateSpread(
		FMath::Max(0.f, CachedWeaponAttributeSet->GetCurrentSpreadRadius()),
		FMath::Max(0.f, CachedWeaponAttributeSet->GetMaxSpreadRadius()));
}

void UPlayerHUDPresenterComponent::UpdateWeaponInfoVisibility() const
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const ABaseWeapon* EquippedWeapon = GetEquippedWeapon();
	const bool bHasEquippedWeapon = EquippedWeapon != nullptr;
	UTexture2D* WeaponInfoImage = nullptr;
	if (EquippedWeapon)
	{
		WeaponInfoImage = EquippedWeapon->GetWeaponData().WeaponInfoImage;
	}

	PlayerStatusWidget->UpdateWeaponInfo(bHasEquippedWeapon, WeaponInfoImage);
}

void UPlayerHUDPresenterComponent::UpdatePickupWeaponPrompt(const ABaseWeapon* NearbyDroppedWeapon) const
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const bool bHasNearbyDroppedWeapon = NearbyDroppedWeapon != nullptr;

	FText PickupWeaponName;
	UTexture2D* PickupWeaponInfoImage = nullptr;
	if (NearbyDroppedWeapon)
	{
		const FPlayerWeaponData& NearbyWeaponData = NearbyDroppedWeapon->GetWeaponData();
		PickupWeaponName = NearbyWeaponData.WeaponDisplayName;
		PickupWeaponInfoImage = NearbyWeaponData.WeaponInfoImage;

		if (PickupWeaponName.IsEmpty())
		{
			const FGameplayTag NearbyWeaponTag = NearbyDroppedWeapon->GetWeaponTag();
			PickupWeaponName = NearbyWeaponTag.IsValid()
				? FText::FromName(NearbyWeaponTag.GetTagName())
				: FText::FromString(TEXT("Weapon"));
		}
	}

	PlayerStatusWidget->UpdatePickupWeaponPrompt(bHasNearbyDroppedWeapon, PickupWeaponName, PickupWeaponInfoImage);
}

const ABaseWeapon* UPlayerHUDPresenterComponent::GetEquippedWeapon() const
{
	return CachedPlayerCombatComponent ? CachedPlayerCombatComponent->GetCharacterCurrentEquippedWeapon() : nullptr;
}

void UPlayerHUDPresenterComponent::BindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle, FAttributeChangedHandler Handler)
{
	if (!CachedASC)
	{
		return;
	}

	// Clear bind
	UnbindAttributeChangedDelegate(Attribute, Handle);
	// Bind
	Handle = CachedASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, Handler);
}

void UPlayerHUDPresenterComponent::UnbindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle)
{
	if (!CachedASC || !Handle.IsValid())
	{
		return;
	}

	// Unbind
	CachedASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
	Handle.Reset();
}

void UPlayerHUDPresenterComponent::HandleMaxHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const float CurrentHp = CachedCharacterAttributeSet ? CachedCharacterAttributeSet->GetCurrentHp() : 0.0f;
	PlayerStatusWidget->UpdateHealth(CurrentHp, AttributeChangeData.NewValue);
}

void UPlayerHUDPresenterComponent::HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const float MaxHp = CachedCharacterAttributeSet ? CachedCharacterAttributeSet->GetMaxHp() : 0.0f;
	PlayerStatusWidget->UpdateHealth(AttributeChangeData.NewValue, MaxHp);
}

void UPlayerHUDPresenterComponent::HandleAmmoAttributeChanged(const FOnAttributeChangeData& /*AttributeChangeData*/)
{
	UpdateAmmoOnHUD();
}

void UPlayerHUDPresenterComponent::HandleSpreadAttributeChanged(const FOnAttributeChangeData& /*AttributeChangeData*/)
{
	UpdateSpreadOnHUD();
}

void UPlayerHUDPresenterComponent::HandleEquippedWeaponChanged(FGameplayTag /*PreviousWeaponTag*/, FGameplayTag /*CurrentWeaponTag*/)
{
	UpdateAmmoOnHUD();
	UpdateSpreadOnHUD();
	UpdateWeaponInfoVisibility();
}

void UPlayerHUDPresenterComponent::HandleNearbyDroppedWeaponChanged(const ABaseWeapon* NearbyDroppedWeapon)
{
	UpdatePickupWeaponPrompt(NearbyDroppedWeapon);
}
