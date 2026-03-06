/*
 * 파일명 : EnemyCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 김사윤
 * 수정일 : 2026-03-06
 */

#include "Characters/Enemies/EnemyCharacter.h"

#include "Characters/Enemies/EnemyAIController.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"

AEnemyCharacter::AEnemyCharacter()
{
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	MGSAbilitySystemComponent = CreateDefaultSubobject<UMGSAbilitySystemComponent>(TEXT("MGSAbilitySystemComponent"));
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	
	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));

	DefaultEnemyStateTag = MGSGameplayTags::State_Enemy_Clear;
}

UPawnCombatComponent* AEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

UMGSAbilitySystemComponent* AEnemyCharacter::GetMGSAbilitySystemComponent() const
{
	return MGSAbilitySystemComponent;
}

UCharacterAttributeSet* AEnemyCharacter::GetCharacterAttributeSet() const
{
	return CharacterAttributeSet;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetEnemyStateTag(DefaultEnemyStateTag);

	if (bEnableDebugStateInput)
	{
		BindDebugStateInputs();
	}
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (MGSAbilitySystemComponent)
	{
		MGSAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
	InitEnemyStartupData();
}

void AEnemyCharacter::InitEnemyStartupData()
{
	if (StartupData.IsNull())
	{
		return;
	}
	
	// StartupData 에셋을 비동기 로드로 불러온다.
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		StartupData.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[this]()
			{
				if (UDA_StartupBase* LoadedData = StartupData.Get())
				{
					if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
					{
						LoadedData->GiveToAbilitySystemComponent(ASC);
					}
				}
			}
		)
	);
}

void AEnemyCharacter::SetEnemyStateTag(const FGameplayTag& NewStateTag)
{
	if (NewStateTag == CurrentEnemyStateTag)
	{
		return;
	}

	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		if (CurrentEnemyStateTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(CurrentEnemyStateTag);
		}
		if (NewStateTag.IsValid())
		{
			ASC->AddLooseGameplayTag(NewStateTag);
		}
	}

	CurrentEnemyStateTag = NewStateTag;
	ApplyStateMaterial(NewStateTag);
}

void AEnemyCharacter::ApplyStateMaterial(const FGameplayTag& NewStateTag)
{
	UMaterialInterface* TargetMaterial = nullptr;
	if (NewStateTag == MGSGameplayTags::State_Enemy_Clear)
	{
		TargetMaterial = ClearStateMaterial;
	}
	else if (NewStateTag == MGSGameplayTags::State_Enemy_Suspicious)
	{
		TargetMaterial = SuspiciousStateMaterial;
	}
	else if (NewStateTag == MGSGameplayTags::State_Enemy_Investigation)
	{
		TargetMaterial = InvestigationStateMaterial;
	}
	else if (NewStateTag == MGSGameplayTags::State_Enemy_Combat)
	{
		TargetMaterial = CombatStateMaterial;
	}

	if (TargetMaterial && GetMesh())
	{
		GetMesh()->SetMaterial(StateMaterialSlotIndex, TargetMaterial);
	}
}

void AEnemyCharacter::BindDebugStateInputs()
{
	if (bDebugStateInputBound)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("EnemyCharacter: No local PlayerController available"));
		}
		return;
	}

	UInputComponent* PCInputComponent = PlayerController->InputComponent;
	if (!PCInputComponent)
	{
		PCInputComponent = NewObject<UInputComponent>(PlayerController, TEXT("EnemyDebugInputComponent"));
		PCInputComponent->RegisterComponent();
		PlayerController->PushInputComponent(PCInputComponent);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("EnemyCharacter: Debug state keys bound (7/8/9/0)"));
	}

	PCInputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AEnemyCharacter::DebugSetStateClear);
	PCInputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &AEnemyCharacter::DebugSetStateSuspicious);
	PCInputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &AEnemyCharacter::DebugSetStateInvestigation);
	PCInputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &AEnemyCharacter::DebugSetStateCombat);

	bDebugStateInputBound = true;
}

void AEnemyCharacter::DebugSetStateClear()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("EnemyCharacter: State -> Clear"));
	}
	SetEnemyStateTag(MGSGameplayTags::State_Enemy_Clear);
}

void AEnemyCharacter::DebugSetStateSuspicious()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("EnemyCharacter: State -> Suspicious"));
	}
	SetEnemyStateTag(MGSGameplayTags::State_Enemy_Suspicious);
}

void AEnemyCharacter::DebugSetStateInvestigation()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("EnemyCharacter: State -> Investigation"));
	}
	SetEnemyStateTag(MGSGameplayTags::State_Enemy_Investigation);
}

void AEnemyCharacter::DebugSetStateCombat()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("EnemyCharacter: State -> Combat"));
	}
	SetEnemyStateTag(MGSGameplayTags::State_Enemy_Combat);
}


