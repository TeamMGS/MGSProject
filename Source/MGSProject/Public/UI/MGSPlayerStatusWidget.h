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
	// HP 갱신
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealth(float InCurrentHp, float InMaxHp);

	// 탄약 갱신
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateAmmo(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);

	// 스프레드 갱신
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateSpread(float InCurrentSpreadRadius, float InMaxSpreadRadius);

	// 무기 이미지 UI 노출 설정
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWeaponInfoVisible(bool bInVisible);

	// 무기 이미지 UI 갱신
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateWeaponInfo(bool bInVisible, UTexture2D* InWeaponInfoImage);

	// 무기 줍기 프롬프트 데이터 갱신
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdatePickupWeaponPrompt(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage);

	// HP 비율 가져옴
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthPercent() const;

	// 스프레드 비율 가져옴
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetSpreadPercent() const;

	// 무기 이미지 가져옴
	UFUNCTION(BlueprintPure, Category = "HUD")
	UTexture2D* GetCurrentWeaponInfoImage() const;

	// 무기 줍기 프롬프트 무기 이름
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetPickupWeaponPromptName() const;

	// 무기 줍기 프롬프트 무기 이미지
	UFUNCTION(BlueprintPure, Category = "HUD")
	UTexture2D* GetPickupWeaponPromptImage() const;

protected:
	// HP UI 갱신 노드
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnHealthUpdated(float InCurrentHp, float InMaxHp, float InHealthPercent);

	// 탄약 UI 갱신 노드
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnAmmoUpdated(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo);

	// 스프레드 UI 갱신 노드
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnSpreadUpdated(float InCurrentSpreadRadius, float InMaxSpreadRadius, float InSpreadPercent);

	// 무기 이미지 UI 갱신 노드
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnWeaponInfoVisibilityChanged(bool bInVisible);

	// 무기 줍기 프롬프트 데이터 UI 갱신 노드
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_OnPickupWeaponPromptUpdated(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage);

private:
	// 현재 HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentHp = 0.0f;

	// 최대 HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxHp = 0.0f;

	// 현재 탄약
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CurrentMagazineAmmo = 0;

	// 최대 탄약
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 MaxMagazineAmmo = 0;

	// 현재 탄창
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	int32 CarriedAmmo = 0;

	// 현재 스프레드 반지름
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float CurrentSpreadRadius = 0.0f;

	// 최대 스프레드 반지름
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float MaxSpreadRadius = 0.0f;

	// 무기 UI 노출 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	bool bWeaponInfoVisible = false;

	// 무기 UI 이미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CurrentWeaponInfoImage = nullptr;

	// 무기 줍기 프롬프트 노출 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	bool bPickupWeaponPromptVisible = false;

	// 무기 줍기 프롬프트 무기 이름
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	FText PickupWeaponPromptName;

	// 무기 줍기 프롬프트 무기 이미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> PickupWeaponPromptImage = nullptr;
	
};
