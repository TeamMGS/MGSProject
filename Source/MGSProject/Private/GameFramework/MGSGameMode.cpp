/*
 * 파일명 : MGSGameMode.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */

#include "GameFramework/MGSGameMode.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MapCaptureActor.h"

AMGSGameMode::AMGSGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AMGSPlayerController::StaticClass();
	PlayerStateClass = AMGSPlayerState::StaticClass();
}

void AMGSGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Set MapCapture from the current level
	MapCaptureActor = Cast<AMapCaptureActor>(UGameplayStatics::GetActorOfClass(this, AMapCaptureActor::StaticClass()));
}

void AMGSGameMode::MissionComplete() const
{
	const AMGSPlayerController* PlayerController = Cast<AMGSPlayerController>(PlayerControllerClass);
	if (!PlayerController)
	{
		return;
	}
	// Request the game over UI from the player controller
	PlayerController->RequestShowGameOverUI(true);
}
