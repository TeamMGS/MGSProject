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
#include "MGSEnumType.h"
#include "MGSStructType.h"
#include "DataAssets/DA_NarrationConfig.h"
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

// 나레이션 종료 알림을 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrationFinished, ENarrationSituation, FinishedSituation);

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
	// 상황별 나레이션 블루프린트 추가 함수
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void PlayNarration(ENarrationSituation Situation);
	
	// 외부(PlayerController 등)에서 이 델리게이트에 함수를 묶을 수 있습니다.
	UPROPERTY(BlueprintAssignable, Category = "Narration")
	FOnNarrationFinished OnNarrationFinished;
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
	// 슬롯비우기
	void ClearNarrationSlot();
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
	
	// 나레이션 설정 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Narration", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDA_NarrationConfig> NarrationConfig;

	// 대사 위젯 클래스 (WBP_Dialogue)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Narration", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> NarrationWidgetClass;
	
	// 다음 단계의 대사를 재생하는 함수
	void PlayNextNarrationStep();

	// 현재 진행 중인 나레이션 단계들 저장
	TArray<FNarrationStepInfo> CurrentNarrationSteps;

	// 현재 몇 번째 단계를 재생 중인지 인덱스
	int32 CurrentStepIndex;

	// 현재 어떤 상황의 대사가 나오고 있는지 저장
	ENarrationSituation CurrentSituation;
	
	// 대사를 자동으로 지우기 위한 타이머 핸들
	FTimerHandle NarrationTimerHandle;
};
