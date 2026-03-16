/*
 * 파일명 : EnemyCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Characters/Enemies/EnemyCharacter.h"

#include "Characters/Enemies/EnemyAIController.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAssets/Spread/DA_SpreadSettings.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "MGSDebugHelper.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"
#include "GameplayEffectTypes.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"
#include "Weapon/BaseGun.h"

AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Controller
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	// Movement
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMovementSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// ASC
	MGSAbilitySystemComponent = CreateDefaultSubobject<UMGSAbilitySystemComponent>(TEXT("MGSAbilitySystemComponent"));
	MGSAbilitySystemComponent->SetIsReplicated(true);
	
	// Character AttributeSet
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
	
	// Combat Component
	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
	
	// Head Collision
	HeadHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HeadHitSphere"));
	HeadHitSphere->SetupAttachment(GetMesh(), TEXT("head"));
	HeadHitSphere->InitSphereRadius(HeadHitSphereRadius);
	HeadHitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadHitSphere->SetCollisionObjectType(ECC_Pawn);
	HeadHitSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	HeadHitSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	HeadHitSphere->SetGenerateOverlapEvents(true);
	HeadHitSphere->SetCanEverAffectNavigation(false);

	// AI
	PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
	PerceptionStimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	PerceptionStimuliSource->RegisterForSense(UAISense_Hearing::StaticClass());

	// Tag
	DefaultEnemyStateTag = MGSGameplayTags::State_Enemy_Clear;
}

UPawnCombatComponent* AEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return GetMGSAbilitySystemComponent();
}

UMGSAbilitySystemComponent* AEnemyCharacter::GetMGSAbilitySystemComponent() const
{
	return MGSAbilitySystemComponent;
}

UCharacterAttributeSet* AEnemyCharacter::GetCharacterAttributeSet() const
{
	return CharacterAttributeSet;
}

UWeaponAttributeSet* AEnemyCharacter::GetWeaponAttributeSet() const
{
	return WeaponAttributeSet;
}

const UDA_SpreadSettings* AEnemyCharacter::GetSpreadSettings() const
{
	return SpreadSettingsData ? SpreadSettingsData : GetDefault<UDA_SpreadSettings>();
}

float AEnemyCharacter::GetDamageMultiplierForHit(const FHitResult& Hit) const
{
	if (!HeadHitSphere)
	{
		return Super::GetDamageMultiplierForHit(Hit);
	}

	// Hit Head Sphere Collision
	if (Hit.GetComponent() == HeadHitSphere)
	{
		return HeadshotDamageMultiplier;
	}

	// Hit Location : ImpactPoint(실제 표면 충돌 지점)->Location(트레이스 도형 중심 위치)
	const FVector HitLocation = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
	if (HitLocation.IsNearlyZero())
	{
		return Super::GetDamageMultiplierForHit(Hit);
	}

	// Hit Location이 Head Sphere 중심 반경 안에 있는지 검사
	const float HeadRadius = HeadHitSphere->GetScaledSphereRadius();
	const float DistanceSquaredToHead = FVector::DistSquared(HitLocation, HeadHitSphere->GetComponentLocation());
	if (DistanceSquaredToHead <= FMath::Square(HeadRadius))
	{
		return HeadshotDamageMultiplier;
	}

	return Super::GetDamageMultiplierForHit(Hit);
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnCharacterMovementUpdated.AddDynamic(this, &ThisClass::HandleMovementUpdated);

	if (EnemyCombatComponent)
	{
		EquippedWeaponChangedHandle = EnemyCombatComponent->GetOnEquippedWeaponChangedDelegate().AddUObject(
			this,
			&ThisClass::HandleEquippedWeaponChanged);
	}
	
	// AI
	if (PerceptionStimuliSource)
	{
		PerceptionStimuliSource->RegisterWithPerceptionSystem();
	}
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (MGSAbilitySystemComponent)
	{
		// Init ASC
		MGSAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// Init
	// AttributeSet
	InitializeEnemyAttributes();
	// Tag
	SetEnemyStateTag(DefaultEnemyStateTag);
	// DA_StartupEnemy
	InitEnemyStartupData();
	// MovementMode
	SetDefaultMovementMode();
	RequestSpreadRefreshNextTick();
	
	// Debug
	BindHpChangedDelegate();
	
	if (bEnableDebugStateInput)
	{
		BindDebugStateInputs();
	}
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCharacterMovementUpdated.RemoveDynamic(this, &ThisClass::HandleMovementUpdated);

	if (EnemyCombatComponent && EquippedWeaponChangedHandle.IsValid())
	{
		EnemyCombatComponent->GetOnEquippedWeaponChangedDelegate().Remove(EquippedWeaponChangedHandle);
		EquippedWeaponChangedHandle.Reset();
	}

	// UnBind
	if (MGSAbilitySystemComponent && bHasBoundHpChangedDelegate)
	{
		MGSAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
			.Remove(CurrentHpChangedDelegateHandle);
		CurrentHpChangedDelegateHandle.Reset();
		bHasBoundHpChangedDelegate = false;
	}

	Super::EndPlay(EndPlayReason);
}

void AEnemyCharacter::InitializeEnemyAttributes()
{
	if (!CharacterAttributeSet)
	{
		return;
	}

	// Init HP
	const float MaxHp = FMath::Max(1.0f, DefaultMaxHp);
	const float CurrentHp = FMath::Clamp(DefaultCurrentHp, 0.0f, MaxHp);
	CharacterAttributeSet->SetMaxHp(MaxHp);
	CharacterAttributeSet->SetCurrentHp(CurrentHp);
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
			// Clear current tag
			ASC->RemoveLooseGameplayTag(CurrentEnemyStateTag);
		}
		if (NewStateTag.IsValid())
		{
			// Add tag
			ASC->AddLooseGameplayTag(NewStateTag);
		}
	}

	// Update current tag 
	CurrentEnemyStateTag = NewStateTag;
	
	ApplyStateMaterial(NewStateTag);
	DebugPrintOwnedTags();
}

bool AEnemyCharacter::SetDefaultMovementMode()
{
	return ApplyEnemyMovementMode(MGSGameplayTags::State_Enemy_Movement_Default, DefaultMovementSpeed);
}

bool AEnemyCharacter::SetWalkMovementMode()
{
	return ApplyEnemyMovementMode(MGSGameplayTags::State_Enemy_Movement_Walk, WalkMovementSpeed);
}

bool AEnemyCharacter::SetSprintMovementMode()
{
	return ApplyEnemyMovementMode(MGSGameplayTags::State_Enemy_Movement_Sprint, SprintMovementSpeed);
}

bool AEnemyCharacter::SetCrouchState(bool bShouldCrouch)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	// Set speed
	MovementComponent->MaxWalkSpeedCrouched = FMath::Max(0.0f, CrouchMovementSpeed);

	if (bShouldCrouch)
	{
		if (!bIsCrouched)
		{
			Crouch();
		}
	}
	else if (bIsCrouched)
	{
		UnCrouch();
	}

	// Set state tag
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		if (bShouldCrouch)
		{
			ASC->AddLooseGameplayTag(MGSGameplayTags::State_Enemy_Crouching);
		}
		else
		{
			ASC->RemoveLooseGameplayTag(MGSGameplayTags::State_Enemy_Crouching);
		}
	}

	RequestSpreadRefreshNextTick();
	return true;
}

bool AEnemyCharacter::ActivateEnemyAbilityByTag(const FGameplayTag& AbilityTag)
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (!ASC || !AbilityTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AbilityTag);
	return ASC->TryActivateAbilitiesByTag(AbilityTags, true);
}

bool AEnemyCharacter::CancelEnemyAbilityByTag(const FGameplayTag& AbilityTag)
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (!ASC || !AbilityTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AbilityTag);
	ASC->CancelAbilities(&AbilityTags);
	return true;
}

void AEnemyCharacter::HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	(void)DeltaSeconds;
	(void)OldLocation;
	(void)OldVelocity;

	RequestSpreadRefreshNextTick();
}

void AEnemyCharacter::HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag)
{
	(void)PreviousWeaponTag;
	(void)CurrentWeaponTag;

	RequestSpreadRefreshNextTick();
}

void AEnemyCharacter::RequestSpreadRefreshNextTick()
{
	if (bPendingSpreadRefreshRequest)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bPendingSpreadRefreshRequest = true;
	World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateCurrentSpreadFromState);
}

void AEnemyCharacter::UpdateCurrentSpreadFromState()
{
	bPendingSpreadRefreshRequest = false;

	UWeaponAttributeSet* CurrentWeaponAttributeSet = GetWeaponAttributeSet();
	if (!CurrentWeaponAttributeSet || !EnemyCombatComponent)
	{
		return;
	}

	ABaseGun* EquippedGun = Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon());
	if (!EquippedGun)
	{
		return;
	}

	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Attacking))
	{
		return;
	}

	const float MaxSpread = FMath::Max(0.f, EquippedGun->GetMaxSpreadRadius());
	const float BaseSpread = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, MaxSpread);
	const float StateSpread = FMath::Clamp(BaseSpread * CalculateCurrentSpreadStateMultiplier(), 0.f, MaxSpread);
	if (!FMath::IsNearlyEqual(CurrentWeaponAttributeSet->GetCurrentSpreadRadius(), StateSpread))
	{
		CurrentWeaponAttributeSet->SetCurrentSpreadRadius(StateSpread);
	}
}

float AEnemyCharacter::CalculateCurrentSpreadStateMultiplier() const
{
	const UDA_SpreadSettings* SpreadSettings = GetSpreadSettings();
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	const bool bIsFalling = MovementComponent && MovementComponent->IsFalling();
	const float HorizontalSpeed = MovementComponent
		? FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size()
		: FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	const bool bIsMoving = HorizontalSpeed > SpreadSettings->MovingSpeedThreshold;
	const bool bIsSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Sprint);
	const bool bIsWalk = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Walk);
	const bool bIsCrouching = bIsCrouched || (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Crouching));

	float StateSpreadMultiplier = 1.f;
	if (bIsFalling)
	{
		StateSpreadMultiplier = SpreadSettings->JumpMultiplier;
	}
	else if (bIsMoving)
	{
		if (bIsSprint)
		{
			StateSpreadMultiplier = SpreadSettings->SprintMultiplier;
		}
		else if (bIsWalk)
		{
			StateSpreadMultiplier = SpreadSettings->WalkMultiplier;
		}
		else
		{
			StateSpreadMultiplier = SpreadSettings->MovingMultiplier;
		}
	}
	else if (bIsCrouching)
	{
		StateSpreadMultiplier = SpreadSettings->CrouchStillMultiplier;
	}

	return FMath::Max(0.f, StateSpreadMultiplier);
}

void AEnemyCharacter::SetEnemyStateTagFromAI(const FGameplayTag& NewStateTag)
{
	SetEnemyStateTag(NewStateTag);
}

void AEnemyCharacter::DebugPrintOwnedTags() const
{
	if (!MGSAbilitySystemComponent)
	{
		return;
	}

	FGameplayTagContainer OwnedTags;
	MGSAbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	const FString TagString = OwnedTags.ToStringSimple();
	UE_LOG(LogTemp, Log, TEXT("[Enemy ASC Tags] %s"), *TagString);
}

void AEnemyCharacter::InitEnemyStartupData()
{
	if (StartupData.IsNull())
	{
		return;
	}

	if (UDA_StartupBase* LoadedData = StartupData.LoadSynchronous())
	{
		if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
		{
			LoadedData->GiveToAbilitySystemComponent(ASC);
		}
	}
}

bool AEnemyCharacter::ApplyEnemyMovementMode(const FGameplayTag& NewMovementStateTag, float NewMaxWalkSpeed)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	// UnCrouch
	SetCrouchState(false);

	// Set speed
	const float ClampedSpeed = FMath::Max(0.0f, NewMaxWalkSpeed);
	MovementComponent->MaxWalkSpeed = ClampedSpeed;

	// Set state tag
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		if (CurrentEnemyMovementStateTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(CurrentEnemyMovementStateTag);
		}

		if (NewMovementStateTag.IsValid())
		{
			ASC->AddLooseGameplayTag(NewMovementStateTag);
		}
	}

	CurrentEnemyMovementStateTag = NewMovementStateTag;
	RequestSpreadRefreshNextTick();
	return true;
}

void AEnemyCharacter::BindHpChangedDelegate()
{
	if (bHasBoundHpChangedDelegate || !MGSAbilitySystemComponent)
	{
		return;
	}

	// Bind Character AttributeSet
	CurrentHpChangedDelegateHandle = MGSAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
		.AddUObject(this, &ThisClass::HandleCurrentHpChanged);
	bHasBoundHpChangedDelegate = true;
}

void AEnemyCharacter::HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (AttributeChangeData.NewValue >= AttributeChangeData.OldValue - KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FString DamageMsg = FString::Printf(TEXT("[EnemyHP][GE] %s HP=%.1f->%.1f / %.1f"),
		*GetName(),
		AttributeChangeData.OldValue,
		AttributeChangeData.NewValue,
		CharacterAttributeSet->GetCurrentHp());
	UE_LOG(LogTemp, Log, TEXT("%s"), *DamageMsg);
	Debug::Print(DamageMsg, FColor::Yellow);

	if (AttributeChangeData.NewValue <= KINDA_SMALL_NUMBER)
	{
		if (MGSAbilitySystemComponent && !MGSAbilitySystemComponent->HasMatchingGameplayTag(MGSGameplayTags::State_Character_Dead))
		{
			FGameplayTagContainer AbilityTags;
			AbilityTags.AddTag(MGSGameplayTags::Ability_Enemy_Death);
			MGSAbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags, true);
		}

		const FString DefeatedMsg = FString::Printf(TEXT("[EnemyHP] %s defeated"), *GetName());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *DefeatedMsg);
		Debug::Print(DefeatedMsg, FColor::Red);
	}
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
