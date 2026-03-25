/*
 * 파일명 : MapCaptureActor.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-17
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */

#include "UI/MapCaptureActor.h"

#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneCaptureComponent2D.h"

AMapCaptureActor::AMapCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(GetRootComponent());
	SceneCaptureComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->OrthoWidth = 16384.0f;
}

bool AMapCaptureActor::ProjectWorldLocationToMapNormalized(const FVector& WorldLocation, FVector2D& OutNormalizedPosition, bool bClampToBounds) const
{
	if (!SceneCaptureComponent)
	{
		return false;
	}

	const float HalfWidth = SceneCaptureComponent->OrthoWidth * 0.5f;
	float AspectRatio = 1.0f;
	if (const UTextureRenderTarget2D* TextureTarget = SceneCaptureComponent->TextureTarget)
	{
		if (TextureTarget->SizeY > 0)
		{
			AspectRatio = static_cast<float>(TextureTarget->SizeX) / static_cast<float>(TextureTarget->SizeY);
		}
	}

	const float HalfHeight = HalfWidth / FMath::Max(AspectRatio, KINDA_SMALL_NUMBER);
	if (HalfWidth <= KINDA_SMALL_NUMBER || HalfHeight <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector WorldDelta = WorldLocation - SceneCaptureComponent->GetComponentLocation();
	const float LocalX = FVector::DotProduct(WorldDelta, SceneCaptureComponent->GetRightVector());
	const float LocalY = FVector::DotProduct(WorldDelta, SceneCaptureComponent->GetUpVector());

	const float RawX = (LocalX / (HalfWidth * 2.0f)) + 0.5f;
	const float RawY = 0.5f - (LocalY / (HalfHeight * 2.0f));
	const bool bInsideBounds = RawX >= 0.0f && RawX <= 1.0f && RawY >= 0.0f && RawY <= 1.0f;

	OutNormalizedPosition.X = bClampToBounds ? FMath::Clamp(RawX, 0.0f, 1.0f) : RawX;
	OutNormalizedPosition.Y = bClampToBounds ? FMath::Clamp(RawY, 0.0f, 1.0f) : RawY;
	return bInsideBounds || bClampToBounds;
}

bool AMapCaptureActor::ConvertNormalizedToCanvasPosition(const FVector2D& NormalizedPosition, const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition) const
{
	if (MapPixelSize.X <= KINDA_SMALL_NUMBER || MapPixelSize.Y <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutCanvasPosition = FVector2D(NormalizedPosition.X * MapPixelSize.X, NormalizedPosition.Y * MapPixelSize.Y);
	return true;
}

bool AMapCaptureActor::GetPlayerMarkerData(APlayerController* PlayerController, const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition,
	float& OutYawDegrees, bool bClampToBounds) const
{
	const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!PlayerPawn)
	{
		return false;
	}

	FVector2D NormalizedPosition;
	if (!ProjectWorldLocationToMapNormalized(PlayerPawn->GetActorLocation(), NormalizedPosition, bClampToBounds))
	{
		return false;
	}

	OutYawDegrees = PlayerPawn->GetActorRotation().Yaw - SceneCaptureComponent->GetComponentRotation().Yaw;
	return ConvertNormalizedToCanvasPosition(NormalizedPosition, MapPixelSize, OutCanvasPosition);
}

bool AMapCaptureActor::GetObjectiveMarkerData(const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition, bool bClampToBounds) const
{
	if (!ObjectiveActor)
	{
		return false;
	}

	FVector2D NormalizedPosition;
	if (!ProjectWorldLocationToMapNormalized(ObjectiveActor->GetActorLocation(), NormalizedPosition, bClampToBounds))
	{
		return false;
	}

	return ConvertNormalizedToCanvasPosition(NormalizedPosition, MapPixelSize, OutCanvasPosition);
}

AActor* AMapCaptureActor::GetObjectiveActor() const
{
	return ObjectiveActor.Get();
}

void AMapCaptureActor::ChangeNextTarget()
{
	if (TargetActors.IsEmpty())
	{
		ObjectiveActor = nullptr;
		CurrentTargetIndex = INDEX_NONE;
		return;
	}
	
	CurrentTargetIndex = (CurrentTargetIndex + 1) % TargetActors.Num();
	ObjectiveActor = TargetActors[CurrentTargetIndex];
}
