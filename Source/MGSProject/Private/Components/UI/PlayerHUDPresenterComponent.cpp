/*
 * 파일명 : PlayerHUDPresenterComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 :  장대한
 * 수정일 :  2026-03-05
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

UPlayerHUDPresenterComponent::UPlayerHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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

	CreatePlayerStatusWidget(); // 위젯 생성
	ClearHUDDataBindings(); // 기존 바인딩 정리

	const APlayerCharacter* PlayerCharacter = PlayerController->GetPawn<APlayerCharacter>();
	if (!PlayerCharacter)
	{
		return;
	}

	// 캐싱
	CachedASC = PlayerCharacter->GetMGSAbilitySystemComponent();
	CachedCharacterAttributeSet = PlayerCharacter->GetCharacterAttributeSet();
	CachedWeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet();
	CachedPlayerCombatComponent = PlayerCharacter->GetPlayerCombatComponent();

	if (!CachedASC || !CachedCharacterAttributeSet || !CachedWeaponAttributeSet)
	{
		return;
	}

	// Attribute 델리게이트 등록
	BindAttributeChangedDelegate(UCharacterAttributeSet::GetCurrentHpAttribute(), CurrentHpChangedHandle, &ThisClass::HandleCurrentHpChanged);
	BindAttributeChangedDelegate(UCharacterAttributeSet::GetMaxHpAttribute(), MaxHpChangedHandle, &ThisClass::HandleMaxHpChanged);
	BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentMagazineAmmoAttribute(), CurrentMagazineAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
	BindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxMagazineAmmoAttribute(), MaxMagazineAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
	BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentCarriedAmmoAttribute(), CurrentCarriedAmmoChangedHandle, &ThisClass::HandleAmmoAttributeChanged);
	BindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentSpreadRadiusAttribute(), CurrentSpreadRadiusChangedHandle, &ThisClass::HandleSpreadAttributeChanged);
	BindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxSpreadRadiusAttribute(), MaxSpreadRadiusChangedHandle, &ThisClass::HandleSpreadAttributeChanged);

	// 무기 장착 변경 델리게이트 등록
	if (CachedPlayerCombatComponent)
	{
		EquippedWeaponChangedHandle = CachedPlayerCombatComponent->GetOnEquippedWeaponChangedDelegate()
			.AddUObject(this, &ThisClass::HandleEquippedWeaponChanged);
	}

	// 초기값 HUD 푸시 
	PushInitialHUDValues();
}

void UPlayerHUDPresenterComponent::ClearHUDDataBindings()
{
	// 등록된 모든 Attribute 델리게이트 해제
	UnbindAttributeChangedDelegate(UCharacterAttributeSet::GetCurrentHpAttribute(), CurrentHpChangedHandle);
	UnbindAttributeChangedDelegate(UCharacterAttributeSet::GetMaxHpAttribute(), MaxHpChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentMagazineAmmoAttribute(), CurrentMagazineAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxMagazineAmmoAttribute(), MaxMagazineAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentCarriedAmmoAttribute(), CurrentCarriedAmmoChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetCurrentSpreadRadiusAttribute(), CurrentSpreadRadiusChangedHandle);
	UnbindAttributeChangedDelegate(UWeaponAttributeSet::GetMaxSpreadRadiusAttribute(), MaxSpreadRadiusChangedHandle);

	// 무기 장착 델리게이트 해제
	if (CachedPlayerCombatComponent && EquippedWeaponChangedHandle.IsValid())
	{
		CachedPlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().Remove(EquippedWeaponChangedHandle);
		EquippedWeaponChangedHandle.Reset();
	}

	// 캐시 nullptr 정리
	CachedASC = nullptr;
	CachedCharacterAttributeSet = nullptr;
	CachedWeaponAttributeSet = nullptr;
	CachedPlayerCombatComponent = nullptr;
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

void UPlayerHUDPresenterComponent::BindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle, FAttributeChangedHandler Handler)
{
	if (!CachedASC)
	{
		return;
	}

	UnbindAttributeChangedDelegate(Attribute, Handle);
	Handle = CachedASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, Handler);
}

void UPlayerHUDPresenterComponent::UnbindAttributeChangedDelegate(const FGameplayAttribute& Attribute, FDelegateHandle& Handle)
{
	if (!CachedASC || !Handle.IsValid())
	{
		return;
	}

	CachedASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
	Handle.Reset();
}

void UPlayerHUDPresenterComponent::PushInitialHUDValues() const
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	if (CachedCharacterAttributeSet)
	{
		PlayerStatusWidget->UpdateHealth(CachedCharacterAttributeSet->GetCurrentHp(), CachedCharacterAttributeSet->GetMaxHp());
	}
	else
	{
		PlayerStatusWidget->UpdateHealth(0.f, 0.f);
	}

	UpdateAmmoOnHUD();
	UpdateSpreadOnHUD();
	UpdateWeaponInfoVisibility();
}

void UPlayerHUDPresenterComponent::UpdateWeaponInfoVisibility() const
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const bool bHasEquippedWeapon = CachedPlayerCombatComponent && CachedPlayerCombatComponent->GetCharacterCurrentEquippedWeapon();
	PlayerStatusWidget->SetWeaponInfoVisible(bHasEquippedWeapon);
}

void UPlayerHUDPresenterComponent::UpdateAmmoOnHUD() const
{
	if (!PlayerStatusWidget || !CachedWeaponAttributeSet)
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

	PlayerStatusWidget->UpdateSpread(
		FMath::Max(0.f, CachedWeaponAttributeSet->GetCurrentSpreadRadius()),
		FMath::Max(0.f, CachedWeaponAttributeSet->GetMaxSpreadRadius()));
}

void UPlayerHUDPresenterComponent::HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const float MaxHp = CachedCharacterAttributeSet ? CachedCharacterAttributeSet->GetMaxHp() : 0.f;
	PlayerStatusWidget->UpdateHealth(AttributeChangeData.NewValue, MaxHp);
}

void UPlayerHUDPresenterComponent::HandleMaxHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (!PlayerStatusWidget)
	{
		return;
	}

	const float CurrentHp = CachedCharacterAttributeSet ? CachedCharacterAttributeSet->GetCurrentHp() : 0.f;
	PlayerStatusWidget->UpdateHealth(CurrentHp, AttributeChangeData.NewValue);
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
	UpdateWeaponInfoVisibility();
	UpdateAmmoOnHUD();
	UpdateSpreadOnHUD();
}
