/*
 * 파일명 : MGSGameMode.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "GameFramework/MGSGameMode.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Characters/Player/PlayerCharacter.h"

AMGSGameMode::AMGSGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AMGSPlayerController::StaticClass();
	PlayerStateClass = AMGSPlayerState::StaticClass();
}
