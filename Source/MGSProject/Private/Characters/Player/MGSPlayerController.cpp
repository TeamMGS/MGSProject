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
#include "Characters/Player/MGSPlayerState.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/Input/MGSInputComponent.h"
#include "Components/UI/PlayerHUDPresenterComponent.h"
#include "DataAssets/Input/DA_InputConfig.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "InputActionValue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MGSPlayerStatusWidget.h"
#include "Sound/SoundCue.h"

AMGSPlayerController::AMGSPlayerController()
{
	PlayerHUDPresenter = CreateDefaultSubobject<UPlayerHUDPresenterComponent>(TEXT("MGS_HUD_Presenter"));
}

void AMGSPlayerController::ActivateEnemyAbility(const FString& AbilityTagString)
{
	ExecuteEnemyAbilityCommand(FindNearestEnemyCharacter(), AbilityTagString);
}

void AMGSPlayerController::ActivateEnemyAbilityOn(const FString& EnemyName, const FString& AbilityTagString)
{
	ExecuteEnemyAbilityCommand(FindEnemyCharacterByName(EnemyName), AbilityTagString);
}

void AMGSPlayerController::ChangeBGM(USoundCue* NewBGMCue)
{
	// 기존에 재생 중인 BGM이 있는지 확인하고 정지
	if (CurrentBGMComponent != nullptr && CurrentBGMComponent->IsPlaying())
	{
		CurrentBGMComponent->FadeOut(1.5f, 0.0f);
	}

	// 새로운 BGM이 전달되었다면 재생
	if (NewBGMCue != nullptr)
	{
		CurrentBGMComponent = UGameplayStatics::SpawnSound2D(this, NewBGMCue);

		if (CurrentBGMComponent != nullptr)
		{
			CurrentBGMComponent->FadeIn(0.5f, 1.0f);
		}
	}
}

void AMGSPlayerController::SetPlayerHp(float NewCurrentHp)
{
	AMGSPlayerState* MGSPlayerState = GetPlayerState<AMGSPlayerState>();
	if (!MGSPlayerState)
	{
		ClientMessage(TEXT("SetPlayerHp failed: player state not found."));
		return;
	}

	if (!MGSPlayerState->SetCurrentHpForDebug(NewCurrentHp))
	{
		ClientMessage(TEXT("SetPlayerHp failed: CharacterAttributeSet or ASC not found."));
		return;
	}

	const UCharacterAttributeSet* CharacterAttributeSet = MGSPlayerState->GetCharacterAttributeSet();
	const float CurrentHp = CharacterAttributeSet ? CharacterAttributeSet->GetCurrentHp() : 0.0f;
	const float MaxHp = CharacterAttributeSet ? CharacterAttributeSet->GetMaxHp() : 0.0f;
	ClientMessage(FString::Printf(
		TEXT("SetPlayerHp applied: HP=%.1f / %.1f (requested %.1f)"),
		CurrentHp,
		MaxHp,
		NewCurrentHp));
}

void AMGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(InputConfigDataAsset, TEXT("InputConfigDataAsset is not assigned on %s"), *GetName());
	
	// IMC Setting
	SetupInputMappingContext();
	
	ChangeBGM(BeginPlayBGM);
	
	// HUD 노출
	if (PlayerHUDPresenter)
	{
		PlayerHUDPresenter->SetPlayerStatusWidgetClass(PlayerStatusWidgetClass);
		PlayerHUDPresenter->RefreshHUDDataBindings();
	}
	
	if (PlayerHUDPresenter)
	{
		// 2초 뒤에 테스트 나레이션 실행 (HUD가 완전히 로드된 후 안전하게 보기 위함)
		FTimerHandle TestTimer;
		GetWorldTimerManager().SetTimer(TestTimer, [this]()
		{
			PlayerHUDPresenter->PlayNarration(ENarrationSituation::GameStart);
		}, 2.0f, false);
	}
}

void AMGSPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	// HUD 생성
	if (PlayerHUDPresenter)
	{
		// BeginPlay와 마찬가지로 클래스를 먼저 넘겨줍니다.
		PlayerHUDPresenter->SetPlayerStatusWidgetClass(PlayerStatusWidgetClass);
		PlayerHUDPresenter->RefreshHUDDataBindings();
	}
}

void AMGSPlayerController::OnUnPossess()
{
	// HUD 정리
	if (PlayerHUDPresenter)
	{
		PlayerHUDPresenter->ClearHUDDataBindings();
	}

	Super::OnUnPossess();
}

void AMGSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// HUD 정리
	if (PlayerHUDPresenter)
	{
		PlayerHUDPresenter->ClearHUDDataBindings();
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
	if (PlayerHUDPresenter)
	{
		PlayerHUDPresenter->VisibleMap();
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

void AMGSPlayerController::StopStartupSound()
{
	if (StartupAudioComponent && StartupAudioComponent->IsPlaying())
	{
		StartupAudioComponent->FadeOut(1.0f, 0.0f);
	}
}
