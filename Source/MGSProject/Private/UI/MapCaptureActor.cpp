/*
 * 파일명 : MapCaptureActor.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-17
 * 수정자 : 장대한
 * 수정일 : 2026-03-17
 */

#include "UI/MapCaptureActor.h"

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
