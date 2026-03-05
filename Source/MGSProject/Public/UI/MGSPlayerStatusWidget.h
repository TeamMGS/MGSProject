/*
 * 파일명 : MGSPlayerStatusWidget.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSPlayerStatusWidget.generated.h"

UCLASS(Abstract, BlueprintType)
class MGSPROJECT_API UMGSPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealth(float InCurrentHp, float InMaxHp);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateAmmo(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateSpread(float InCurrentSpreadRadius, float InMaxSpreadRadius);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWeaponInfoVisible(bool bInVisible);

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetSpreadPercent() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnHealthUpdated(float InCurrentHp, float InMaxHp, float InHealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnAmmoUpdated(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnSpreadUpdated(float InCurrentSpreadRadius, float InMaxSpreadRadius, float InSpreadPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnWeaponInfoVisibilityChanged(bool bInVisible);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentHp = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxHp = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CurrentMagazineAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 MaxMagazineAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CarriedAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentSpreadRadius = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxSpreadRadius = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	bool bWeaponInfoVisible = false;
};
