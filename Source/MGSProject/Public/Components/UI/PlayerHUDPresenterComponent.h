/*
 * 파일명 : PlayerHUDPresenterComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-17
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHUDPresenterComponent.generated.h"

struct FGameplayAttribute;
struct FGameplayTag;
struct FOnAttributeChangeData;
class ABaseWeapon;
class UCharacterAttributeSet;
class UMGSAbilitySystemComponent;
class UMGSPlayerStatusWidget;
class UPlayerCombatComponent;
class UWeaponAttributeSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MGSPROJECT_API UPlayerHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHUDPresenterComponent();

	// Set widget class
	void SetPlayerStatusWidgetClass(TSubclassOf<UMGSPlayerStatusWidget> InWidgetClass);
	// Bind data
	void RefreshHUDDataBindings();
	// Clear data
	void ClearHUDDataBindings();
	// Visible Map
	void VisibleMap();

private:
	using FAttributeChangedHandler = void (UPlayerHUDPresenterComponent::*)(const FOnAttributeChangeData&);

	// Create and Add widget
	void CreatePlayerStatusWidget();
	
	// Update HUD
	void PushInitialHUDValues() const;
	// Update
	// HP
	void UpdateHpOnHUD() const;
	// Ammo
	void UpdateAmmoOnHUD() const;
	// Spread
	void UpdateSpreadOnHUD() const;
	// Weapon
	void UpdateWeaponInfoVisibility() const;
	// Drop
	void UpdatePickupWeaponPrompt(const ABaseWeapon* NearbyDroppedWeapon) const;
	const ABaseWeapon* GetEquippedWeapon() const;
	
	// Bind AttributeSet
	void BindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle, FAttributeChangedHandler Handler);
	// UnBind AttributeSet
	void UnbindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle);
	
	// Handler
	// Max HP
	void HandleMaxHpChanged(const FOnAttributeChangeData& AttributeChangeData);
	// Current HP
	void HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData);
	// Ammo
	void HandleAmmoAttributeChanged(const FOnAttributeChangeData& AttributeChangeData);
	// Spread
	void HandleSpreadAttributeChanged(const FOnAttributeChangeData& AttributeChangeData);
	// Weapon
	void HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag);
	// Drop
	void HandleNearbyDroppedWeaponChanged(const ABaseWeapon* NearbyDroppedWeapon);

private:
	// Delegate Handle
	// Character
	// Max HP
	FDelegateHandle MaxHpChangedHandle;
	// Current HP
	FDelegateHandle CurrentHpChangedHandle;
	// Weapon
	// Max Ammo
	FDelegateHandle MaxMagazineAmmoChangedHandle;
	// Current Ammo
	FDelegateHandle CurrentMagazineAmmoChangedHandle;
	// Current CarriedAmmo
	FDelegateHandle CurrentCarriedAmmoChangedHandle;
	// Max Spread
	FDelegateHandle MaxSpreadRadiusChangedHandle;
	// Current Spread
	FDelegateHandle CurrentSpreadRadiusChangedHandle;
	// Combat Component
	// Equip
	FDelegateHandle EquippedWeaponChangedHandle;
	// Drop
	FDelegateHandle NearbyDroppedWeaponChangedHandle;
	
	// Widget
	// Class
	UPROPERTY(Transient)
	TSubclassOf<UMGSPlayerStatusWidget> PlayerStatusWidgetClass;
	// Object
	UPROPERTY(Transient)
	TObjectPtr<UMGSPlayerStatusWidget> PlayerStatusWidget; 

	// GAS
	// ASC
	UPROPERTY(Transient)
	TObjectPtr<UMGSAbilitySystemComponent> CachedASC;
	// Character AttributeSet
	UPROPERTY(Transient)
	TObjectPtr<UCharacterAttributeSet> CachedCharacterAttributeSet;
	// Weapon AttributeSet
	UPROPERTY(Transient)
	TObjectPtr<UWeaponAttributeSet> CachedWeaponAttributeSet;
	
	// Component
	// Combat Component
	UPROPERTY(Transient)
	TObjectPtr<UPlayerCombatComponent> CachedPlayerCombatComponent;
	
};
