/*
 * 파일명 : MGSPlayerStatusWidget.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSPlayerStatusWidget.generated.h"

class UTexture2D;

UCLASS(Abstract, BlueprintType)
class MGSPROJECT_API UMGSPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Update
	// HP
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealth(float InCurrentHp, float InMaxHp);
	// Ammo
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateAmmo(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);
	// Spread
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateSpread(float InCurrentSpreadRadius, float InMaxSpreadRadius);
	// Weapon
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateWeaponInfo(bool bInVisible, UTexture2D* InWeaponInfoImage);
	// Drop
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdatePickupWeaponPrompt(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage);
	
	// Get
	// Current HP Percent
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthPercent() const;
	// Current Spread Percent
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetSpreadPercent() const;
	// Weapon Image
	UFUNCTION(BlueprintPure, Category = "HUD")
	UTexture2D* GetCurrentWeaponInfoImage() const;
	// Drop Weapon Name
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetPickupWeaponPromptName() const;
	// Drop Weapon Image
	UFUNCTION(BlueprintPure, Category = "HUD")
	UTexture2D* GetPickupWeaponPromptImage() const;
	// Set
	// Weapon
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWeaponInfoVisible(bool bInVisible);

protected:
	// BP
	// Update HP UI
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnHealthUpdated(float InCurrentHp, float InMaxHp, float InHealthPercent);
	// Update Ammo UI
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnAmmoUpdated(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);
	// Update Spread UI
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnSpreadUpdated(float InCurrentSpreadRadius, float InMaxSpreadRadius, float InSpreadPercent);
	// Update Weapon UI
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnWeaponInfoVisibilityChanged(bool bInVisible);
	// Update Drop Weapon UI
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnPickupWeaponPromptUpdated(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage);

private:
	// Max HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxHp = 0.0f;
	// Current HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentHp = 0.0f;
	// Max Ammo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 MaxMagazineAmmo = 0;
	// Current Ammo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CurrentMagazineAmmo = 0;
	// Carried Ammo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CarriedAmmo = 0;
	// Max Spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxSpreadRadius = 0.0f;
	// Current Spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentSpreadRadius = 0.0f;
	// Weapon UI Flag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	bool bWeaponInfoVisible = false;
	// Weapon Image
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CurrentWeaponInfoImage = nullptr;
	// Drop Weapon UI Flag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	bool bPickupWeaponPromptVisible = false;
	// Drop Weapon Name
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	FText PickupWeaponPromptName;
	// Drop Weapon Image
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> PickupWeaponPromptImage = nullptr;
	
};
