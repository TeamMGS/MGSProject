/*
 * 파일명 : MapCaptureActor.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-17
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapCaptureActor.generated.h"

class AActor;
class APlayerController;
class USceneCaptureComponent2D;
class USceneComponent;

UCLASS()
class MGSPROJECT_API AMapCaptureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapCaptureActor();

	UFUNCTION(BlueprintPure, Category = "Map")
	bool ProjectWorldLocationToMapNormalized(const FVector& WorldLocation, FVector2D& OutNormalizedPosition, bool bClampToBounds = true) const;

	UFUNCTION(BlueprintPure, Category = "Map")
	bool ConvertNormalizedToCanvasPosition(const FVector2D& NormalizedPosition, const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition) const;

	UFUNCTION(BlueprintPure, Category = "Map")
	bool GetPlayerMarkerData(APlayerController* PlayerController, const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition, float& OutYawDegrees, bool bClampToBounds = true) const;

	UFUNCTION(BlueprintPure, Category = "Map")
	bool GetObjectiveMarkerData(const FVector2D& MapPixelSize, FVector2D& OutCanvasPosition, bool bClampToBounds = true) const;

	UFUNCTION(BlueprintPure, Category = "Map")
	AActor* GetObjectiveActor() const;
	
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ChangeNextTarget();
	
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Map|Bounds", meta = (AllowPrivateAccess = "true"))
	FVector2D MapWorldMin = FVector2D(-8192.0f, -8192.0f);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Map|Bounds", meta = (AllowPrivateAccess = "true"))
	FVector2D MapWorldMax = FVector2D(8192.0f, 8192.0f);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Map|Marker", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> ObjectiveActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Map|Marker", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> TargetActors;
	
private:
	int32 CurrentTargetIndex = 0;
	
};
