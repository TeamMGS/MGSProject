/*
 * 파일명 : MGSPlayerController.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Characters/Player/MGSPlayerController.h"

#include "Characters/Enemies/EnemyCharacter.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/Input/MGSInputComponent.h"
#include "Components/UI/PlayerHUDPresenterComponent.h"
#include "DataAssets/Input/DA_InputConfig.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "InputActionValue.h"

AMGSPlayerController::AMGSPlayerController()
{
	PlayerHUDPresenterComponent = CreateDefaultSubobject<UPlayerHUDPresenterComponent>(TEXT("PlayerHUDPresenterComponent"));
}

void AMGSPlayerController::ActivateEnemyAbility(const FString& AbilityTagString)
{
	ExecuteEnemyAbilityCommand(FindNearestEnemyCharacter(), AbilityTagString);
}

void AMGSPlayerController::ActivateEnemyAbilityOn(const FString& EnemyName, const FString& AbilityTagString)
{
	ExecuteEnemyAbilityCommand(FindEnemyCharacterByName(EnemyName), AbilityTagString);
}

void AMGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(InputConfigDataAsset, TEXT("InputConfigDataAsset is not assigned on %s"), *GetName());
	
	// IMC Setting
	SetupInputMappingContext();

	// HUD 노출
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->SetPlayerStatusWidgetClass(PlayerStatusWidgetClass);
		PlayerHUDPresenterComponent->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	// HUD 생성
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::OnUnPossess()
{
	// HUD 정리
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->ClearHUDDataBindings();
	}

	Super::OnUnPossess();
}

void AMGSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// HUD 정리
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->ClearHUDDataBindings();
	}

	Super::EndPlay(EndPlayReason);
}

void AMGSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind IA
	BindInputActions();
}

void AMGSPlayerController::SetupInputMappingContext() const
{
	if (!IsLocalPlayerController() || !InputConfigDataAsset || !InputConfigDataAsset->InputMappingContext)
	{
		return;
	}
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// DA_InputConfig의 IMC으로 설정
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

	// Bind Native input
	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Weapon_Unequip, ETriggerEvent::Triggered, this, &ThisClass::Input_UnequipWeapons);
	MGSInputComponent->BindNativeInputAction(InputConfigDataAsset, MGSGameplayTags::InputTag_Map, ETriggerEvent::Triggered, this, &ThisClass::Input_Map);
	// Bind Ability input
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

void AMGSPlayerController::Input_UnequipWeapons()
{
	if (APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>())
	{
		if (UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter->GetPlayerCombatComponent())
		{
			PlayerCombatComponent->UnequipCurrentWeapon();
		}
	}
}

void AMGSPlayerController::Input_Map()
{
	if (PlayerHUDPresenterComponent)
	{
		PlayerHUDPresenterComponent->VisibleMap();
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

AEnemyCharacter* AMGSPlayerController::FindNearestEnemyCharacter() const
{
	UWorld* World = GetWorld();
	const APawn* ControlledPawn = GetPawn();
	if (!World || !ControlledPawn)
	{
		return nullptr;
	}

	const FVector Origin = ControlledPawn->GetActorLocation();
	AEnemyCharacter* NearestEnemy = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<AEnemyCharacter> It(World); It; ++It)
	{
		AEnemyCharacter* EnemyCharacter = *It;
		if (!IsValid(EnemyCharacter))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Origin, EnemyCharacter->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestEnemy = EnemyCharacter;
		}
	}

	return NearestEnemy;
}

AEnemyCharacter* AMGSPlayerController::FindEnemyCharacterByName(const FString& EnemyName) const
{
	if (EnemyName.IsEmpty())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AEnemyCharacter> It(World); It; ++It)
	{
		AEnemyCharacter* EnemyCharacter = *It;
		if (!IsValid(EnemyCharacter))
		{
			continue;
		}

		const bool bMatchesActorName = EnemyCharacter->GetName().Equals(EnemyName, ESearchCase::IgnoreCase);
#if WITH_EDITOR
		const bool bMatchesActorLabel = EnemyCharacter->GetActorLabel().Equals(EnemyName, ESearchCase::IgnoreCase);
#else
		const bool bMatchesActorLabel = false;
#endif
		if (bMatchesActorName || bMatchesActorLabel)
		{
			return EnemyCharacter;
		}
	}

	return nullptr;
}

void AMGSPlayerController::ExecuteEnemyAbilityCommand(AEnemyCharacter* TargetEnemy, const FString& AbilityTagString)
{
	if (!TargetEnemy)
	{
		ClientMessage(TEXT("ActivateEnemyAbility failed: target enemy not found."));
		return;
	}

	const FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(FName(*AbilityTagString), false);
	if (!AbilityTag.IsValid())
	{
		ClientMessage(FString::Printf(TEXT("ActivateEnemyAbility failed: invalid ability tag '%s'."), *AbilityTagString));
		return;
	}

	UMGSAbilitySystemComponent* EnemyASC = TargetEnemy->GetMGSAbilitySystemComponent();
	if (!EnemyASC)
	{
		ClientMessage(FString::Printf(TEXT("ActivateEnemyAbility failed: %s has no ASC."), *TargetEnemy->GetName()));
		return;
	}

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AbilityTag);
	const bool bActivated = EnemyASC->TryActivateAbilitiesByTag(AbilityTags, true);

	ClientMessage(FString::Printf(
		TEXT("ActivateEnemyAbility target=%s tag=%s result=%s"),
		*TargetEnemy->GetName(),
		*AbilityTag.ToString(),
		bActivated ? TEXT("success") : TEXT("failed")));
}
