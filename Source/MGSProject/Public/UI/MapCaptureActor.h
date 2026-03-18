/*
 * 파일명 : MapCaptureActor.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-17
 * 수정자 : 장대한
 * 수정일 : 2026-03-17
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapCaptureActor.generated.h"

UCLASS()
class MGSPROJECT_API AMapCaptureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapCaptureActor();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

};
