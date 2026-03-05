/*
 * 파일명 : MGSPlayerController.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#include "Characters/Player/MGSPlayerController.h"

#include "Characters/Player/PlayerCharacter.h"
#include "Components/Input/MGSInputComponent.h"
#include "Components/UI/PlayerHUDPresenterComponent.h"
#include "DataAssets/Input/DA_InputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/MGSGameplayTags.h"
#include "InputActionValue.h"

AMGSPlayerController::AMGSPlayerController()
{
	PlayerHUDPresenterComponent = CreateDefaultSubobject<UPlayerHUDPresenterComponent>(TEXT("PlayerHUDPresenterComponent"));
}

void AMGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(InputConfigDataAsset, TEXT("InputConfigDataAsset is not assigned on %s"), *GetName());
	SetupInputMappingContext();

	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->SetPlayerStatusWidgetClass(PlayerStatusWidgetClass);
		PlayerHUDPresenterComponent->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BindInputActions();
}

void AMGSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::OnUnPossess()
{
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->ClearHUDDataBindings();
	}

	Super::OnUnPossess();
}

void AMGSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->ClearHUDDataBindings();
	}

	Super::EndPlay(EndPlayReason);
}

void AMGSPlayerController::SetupInputMappingContext() const
{
	if (!IsLocalPlayerController() || !InputConfigDataAsset || !InputConfigDataAsset->InputMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputConfigDataAsset->InputMappingContext, 0);
	}
}

void AMGSPlayerController::BindInputActions()
{
	if (!InputConfigDataAsset)
	{
		return;
	}

	UMGSInputComponent* MGSInputComponent = Cast<UMGSInputComponent>(InputComponent);
	if (!MGSInputComponent)
	{
		return;
	}

	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	MGSInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
}

void AMGSPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		PlayerCharacter->Input_Move(InputActionValue);
	}
}

void AMGSPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		PlayerCharacter->Input_Look(InputActionValue);
	}
}

void AMGSPlayerController::Input_AbilityInputPressed(FGameplayTag InputTag)
{
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		PlayerCharacter->Input_AbilityInputPressed(InputTag);
	}
}

void AMGSPlayerController::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		PlayerCharacter->Input_AbilityInputReleased(InputTag);
	}
}

