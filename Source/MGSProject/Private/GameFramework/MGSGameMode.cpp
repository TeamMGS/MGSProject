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
#include "Mission/MissionTarget.h"
#include "UI/MapCaptureActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	InitLevelTargetCount();
}

void AMGSGameMode::InitLevelTargetCount()
{
	TArray<AActor*> Targets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMissionTarget::StaticClass(), Targets);
	LevelTargetCount = Targets.Num();
}

void AMGSGameMode::MissionComplete() const
{
	AMGSPlayerController* PlayerController = Cast<AMGSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController)
	{
		return;
	}
	
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	
	// 1. Pawn을 Character로 캐스팅하여 CharacterMovementComponent에 접근합니다.
	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		if (UCharacterMovementComponent* CharMovement = PlayerCharacter->GetCharacterMovement())
		{
			// 3. 기존의 멈춤 함수 호출 (가속도 초기화)
			CharMovement->StopMovementImmediately(); 
            
			// 4. 강제로 속도(Velocity) 벡터를 0으로 덮어씌웁니다.
			CharMovement->Velocity = FVector::ZeroVector;
            
			// 변경된 속도 값을 컴포넌트에 즉각 업데이트하도록 강제합니다.
			CharMovement->UpdateComponentVelocity();
		}
	}
	// Request the game over UI from the player controller
	PlayerController->RequestShowGameOverUI(true);
}
