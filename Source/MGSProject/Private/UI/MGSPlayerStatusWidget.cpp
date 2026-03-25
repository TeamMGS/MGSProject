/*
 * 파일명 : MGSPlayerStatusWidget.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-05
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#include "UI/MGSPlayerStatusWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/SizeBox.h"
#include "Components/NamedSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "UI/MapCaptureActor.h"

void UMGSPlayerStatusWidget::NativeDestruct()
{
	StopMapMarkerRefresh();
	Super::NativeDestruct();
}

void UMGSPlayerStatusWidget::UpdateHealth(float InCurrentHp, float InMaxHp)
{
	CurrentHp = FMath::Max(0.f, InCurrentHp);
	MaxHp = FMath::Max(0.f, InMaxHp);
	BP_OnHealthUpdated(CurrentHp, MaxHp, GetHealthPercent());
}

void UMGSPlayerStatusWidget::UpdateAmmo(int32 InCurrentMagazineAmmo, int32 InMaxMagazineAmmo, int32 InCarriedAmmo)
{
	CurrentMagazineAmmo = FMath::Max(0, InCurrentMagazineAmmo);
	MaxMagazineAmmo = FMath::Max(0, InMaxMagazineAmmo);
	CarriedAmmo = FMath::Max(0, InCarriedAmmo);
	BP_OnAmmoUpdated(CurrentMagazineAmmo, MaxMagazineAmmo, CarriedAmmo);
}

void UMGSPlayerStatusWidget::UpdateSpread(float InCurrentSpreadRadius, float InMaxSpreadRadius)
{
	CurrentSpreadRadius = FMath::Max(0.f, InCurrentSpreadRadius);
	MaxSpreadRadius = FMath::Max(0.f, InMaxSpreadRadius);
	BP_OnSpreadUpdated(CurrentSpreadRadius, MaxSpreadRadius, GetSpreadPercent());
}

void UMGSPlayerStatusWidget::UpdateWeaponInfo(bool bInVisible, UTexture2D* InWeaponInfoImage)
{
	bWeaponInfoVisible = bInVisible;
	CurrentWeaponInfoImage = InWeaponInfoImage;
	BP_OnWeaponInfoVisibilityChanged(bWeaponInfoVisible);
}

void UMGSPlayerStatusWidget::UpdatePickupWeaponPrompt(bool bInVisible, const FText& InWeaponName, UTexture2D* InWeaponInfoImage)
{
	bPickupWeaponPromptVisible = bInVisible;
	PickupWeaponPromptName = InWeaponName;
	PickupWeaponPromptImage = InWeaponInfoImage;
	BP_OnPickupWeaponPromptUpdated(bPickupWeaponPromptVisible, PickupWeaponPromptName, PickupWeaponPromptImage.Get());
}

void UMGSPlayerStatusWidget::UpdateMap()
{
	if (!MapSizeBox)
	{
		return;
	}

	// 보이는 상태면
	if (MapSizeBox->IsVisible())
	{
		// 안보이도록 함
		MapSizeBox->SetVisibility(ESlateVisibility::Hidden);
		StopMapMarkerRefresh();
	}
	// 안보이는 상태면
	else
	{
		// 보이도록 함
		MapSizeBox->SetVisibility(ESlateVisibility::Visible);
		StartMapMarkerRefresh();
	}
}

void UMGSPlayerStatusWidget::SetMapCaptureActor(AMapCaptureActor* InMapCaptureActor)
{
	MapCaptureActor = InMapCaptureActor;
}

AMapCaptureActor* UMGSPlayerStatusWidget::GetMapCaptureActor() const
{
	if (MapCaptureActor)
	{
		return MapCaptureActor.Get();
	}

	return GetWorld() ? Cast<AMapCaptureActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapCaptureActor::StaticClass())) : nullptr;
}

bool UMGSPlayerStatusWidget::GetPlayerMapMarkerData(const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition, float& OutYawDegrees) const
{
	APlayerController* PlayerController = GetOwningPlayer();
	AMapCaptureActor* ResolvedMapCaptureActor = GetMapCaptureActor();
	return ResolvedMapCaptureActor && PlayerController
		? ResolvedMapCaptureActor->GetPlayerMarkerData(PlayerController, MapPixelSize, OutCanvasPosition, OutYawDegrees)
		: false;
}

bool UMGSPlayerStatusWidget::GetObjectiveMapMarkerData(const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition) const
{
	AMapCaptureActor* ResolvedMapCaptureActor = GetMapCaptureActor();
	return ResolvedMapCaptureActor
		? ResolvedMapCaptureActor->GetObjectiveMarkerData(MapPixelSize, OutCanvasPosition)
		: false;
}

bool UMGSPlayerStatusWidget::IsMapVisible() const
{
	return MapSizeBox && MapSizeBox->IsVisible();
}

float UMGSPlayerStatusWidget::GetHealthPercent() const
{
	if (MaxHp <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentHp / MaxHp, 0.f, 1.f);
}

float UMGSPlayerStatusWidget::GetSpreadPercent() const
{
	if (MaxSpreadRadius <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentSpreadRadius / MaxSpreadRadius, 0.f, 1.f);
}

UTexture2D* UMGSPlayerStatusWidget::GetCurrentWeaponInfoImage() const
{
	return CurrentWeaponInfoImage.Get();
}

FText UMGSPlayerStatusWidget::GetPickupWeaponPromptName() const
{
	return PickupWeaponPromptName;
}

UTexture2D* UMGSPlayerStatusWidget::GetPickupWeaponPromptImage() const
{
	return PickupWeaponPromptImage.Get();
}

void UMGSPlayerStatusWidget::SetWeaponInfoVisible(bool bInVisible)
{
	UpdateWeaponInfo(bInVisible, CurrentWeaponInfoImage.Get());
}

void UMGSPlayerStatusWidget::SetNarrationContent(UUserWidget* InContent)
{
	if (NarrationSlot)
	{
		NarrationSlot->SetContent(InContent);
	}
}

void UMGSPlayerStatusWidget::ShowGameOver(const FString& Text) const
{
	GameOverTextBlock->SetText(FText::FromString(Text));
	GameOverSizeBox->SetVisibility(ESlateVisibility::Visible);
}

void UMGSPlayerStatusWidget::StartMapMarkerRefresh()
{
	UWorld* World = GetWorld();
	if (!World || !MapSizeBox)
	{
		return;
	}

	ForceLayoutPrepass();
	RefreshMapMarkerData();

	World->GetTimerManager().ClearTimer(MapMarkerRefreshTimerHandle);
	World->GetTimerManager().SetTimer(
		MapMarkerRefreshTimerHandle,
		this,
		&ThisClass::RefreshMapMarkerData,
		FMath::Max(0.01f, MapMarkerRefreshInterval),
		true);
}

void UMGSPlayerStatusWidget::StopMapMarkerRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MapMarkerRefreshTimerHandle);
	}
}

void UMGSPlayerStatusWidget::RefreshMapMarkerData()
{
	if (!MapSizeBox || !MapSizeBox->IsVisible())
	{
		StopMapMarkerRefresh();
		return;
	}

	const FVector2D MapPixelSize = MapCanvasPanel
		? MapCanvasPanel->GetCachedGeometry().GetLocalSize()
		: MapSizeBox->GetCachedGeometry().GetLocalSize();
	if (MapPixelSize.X <= KINDA_SMALL_NUMBER || MapPixelSize.Y <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	FVector2D PlayerCanvasPosition = FVector2D::ZeroVector;
	float PlayerYawDegrees = 0.0f;
	const bool bPlayerVisible = GetPlayerMapMarkerData(MapPixelSize, PlayerCanvasPosition, PlayerYawDegrees);

	FVector2D ObjectiveCanvasPosition = FVector2D::ZeroVector;
	const bool bObjectiveVisible = GetObjectiveMapMarkerData(MapPixelSize, ObjectiveCanvasPosition);

	BP_OnMapMarkerDataUpdated(
		bPlayerVisible,
		PlayerCanvasPosition,
		PlayerYawDegrees,
		bObjectiveVisible,
		ObjectiveCanvasPosition);
}
