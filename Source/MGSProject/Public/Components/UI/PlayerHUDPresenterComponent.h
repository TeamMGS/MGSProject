/*
 * 파일명 : PlayerHUDPresenterComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 :  장대한
 * 수정일 :  2026-03-05
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHUDPresenterComponent.generated.h"

class UMGSAbilitySystemComponent;
class UCharacterAttributeSet;
class UWeaponAttributeSet;
class UPlayerCombatComponent;
class UMGSPlayerStatusWidget;
struct FGameplayAttribute;
struct FGameplayTag;
struct FOnAttributeChangeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MGSPROJECT_API UPlayerHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHUDPresenterComponent();

	void SetPlayerStatusWidgetClass(TSubclassOf<UMGSPlayerStatusWidget> InWidgetClass);
	void RefreshHUDDataBindings();
	void ClearHUDDataBindings();

private:
	using FAttributeChangedHandler = void (UPlayerHUDPresenterComponent::*)(const FOnAttributeChangeData&);

	void CreatePlayerStatusWidget(); // 위젯 생성
	void BindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle, FAttributeChangedHandler Handler); // 델리게이트 공통 바인딩
	void UnbindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle); // 델리게이트 공통 바인딩 해제
	void PushInitialHUDValues() const; // HUD 반영
	void UpdateWeaponInfoVisibility() const; // 장착 무기 존재 여부로 패널 표시
	void UpdateAmmoOnHUD() const; // 탄창/최대/예비탄 반영
	void UpdateSpreadOnHUD() const; // 현재/최대 탄착군 반영

	void HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData);
	void HandleMaxHpChanged(const FOnAttributeChangeData& AttributeChangeData);
	void HandleAmmoAttributeChanged(const FOnAttributeChangeData& AttributeChangeData);
	void HandleSpreadAttributeChanged(const FOnAttributeChangeData& AttributeChangeData);
	void HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag);

private:
	UPROPERTY(Transient)
	TSubclassOf<UMGSPlayerStatusWidget> PlayerStatusWidgetClass; // 위젯 클래스

	UPROPERTY(Transient)
	TObjectPtr<UMGSPlayerStatusWidget> PlayerStatusWidget; // 위젯 

	UPROPERTY(Transient)
	TObjectPtr<UMGSAbilitySystemComponent> CachedASC; // ASC 캐시

	UPROPERTY(Transient)
	TObjectPtr<UCharacterAttributeSet> CachedCharacterAttributeSet; // Character AttribuetSet 캐시

	UPROPERTY(Transient)
	TObjectPtr<UWeaponAttributeSet> CachedWeaponAttributeSet; // Weapon AttribuetSet 캐시

	UPROPERTY(Transient)
	TObjectPtr<UPlayerCombatComponent> CachedPlayerCombatComponent; // CombatComponent 캐시

	FDelegateHandle CurrentHpChangedHandle;
	FDelegateHandle MaxHpChangedHandle;
	FDelegateHandle CurrentMagazineAmmoChangedHandle;
	FDelegateHandle MaxMagazineAmmoChangedHandle;
	FDelegateHandle CurrentCarriedAmmoChangedHandle;
	FDelegateHandle CurrentSpreadRadiusChangedHandle;
	FDelegateHandle MaxSpreadRadiusChangedHandle;
	FDelegateHandle EquippedWeaponChangedHandle;
};

