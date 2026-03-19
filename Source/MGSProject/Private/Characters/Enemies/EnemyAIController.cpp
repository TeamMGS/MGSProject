/*
* 파일명 : EnemyAIController.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-16
 */


#include "Characters/Enemies/EnemyAIController.h"

#include "Characters/Enemies/EnemyCharacter.h"
#include "GAS/MGSGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"
#include "GameplayStateTreeModule/Public/Components/StateTreeAIComponent.h"
#include "DrawDebugHelpers.h"

AEnemyAIController::AEnemyAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

	// Match AI StateTree expectations: start logic on possess and attach to pawn for EQS.
	bStartAILogicOnPossess = true;
	bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UAIPerceptionComponent* LocalPerceptionComponent = GetPerceptionComponent())
	{
		LocalPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::HandleTargetPerceptionUpdated);
	}

	DetectionValue = 0.0f;
	bIsTargetSensed = false;
	bIsTargetHeard = false;
	bDetectionLocked = false;
	bSuspendDetectionDecrease = false;
	bHasLastSeenLocation = false;
	LastSeenLocation = FVector::ZeroVector;
	bHasLastHeardLocation = false;
	LastHeardLocation = FVector::ZeroVector;
	CurrentTargetActor.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DetectionTimerHandle,
			this,
			&ThisClass::UpdateDetection,
			DetectionTickInterval,
			true);
	}

	if (!StateTreeComponent)
	{
		return;
	}

	StateTreeComponent->StartLogic();
}

void AEnemyAIController::OnUnPossess()
{
	if (UAIPerceptionComponent* LocalPerceptionComponent = GetPerceptionComponent())
	{
		LocalPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &ThisClass::HandleTargetPerceptionUpdated);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetectionTimerHandle);
	}

	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("UnPossess"));
	}

	Super::OnUnPossess();
}

void AEnemyAIController::SetSuspendDetectionDecrease(bool bSuspend)
{
	bSuspendDetectionDecrease = bSuspend;
}

void AEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	if (!IsValidPerceptionTarget(Actor))
	{
		return;
	}

	const UClass* SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	if (SenseClass == UAISense_Sight::StaticClass())
	{
		CurrentTargetActor = Actor;
		bIsTargetSensed = Stimulus.WasSuccessfullySensed();
		if (bIsTargetSensed)
		{
			LastSeenLocation = Stimulus.StimulusLocation;
			bHasLastSeenLocation = true;
		}
		return;
	}

	if (SenseClass == UAISense_Hearing::StaticClass())
	{
		CurrentTargetActor = Actor;
		bIsTargetHeard = Stimulus.WasSuccessfullySensed();
		if (bIsTargetHeard)
		{
			LastHeardLocation = Stimulus.StimulusLocation;
			bHasLastHeardLocation = true;
		}
		return;
	}
}

float AEnemyAIController::GetTargetLightMultiplier_Implementation(const AActor* Target) const
{
	return 1.0f;
}

void AEnemyAIController::UpdateDetection()
{
	const float DeltaSeconds = DetectionTickInterval;
	if (CurrentTargetActor.IsValid() && !IsValidPerceptionTarget(CurrentTargetActor.Get()))
	{
		CurrentTargetActor.Reset();
		bIsTargetSensed = false;
		bIsTargetHeard = false;
		bHasLastHeardLocation = false;
	}
	if (bDetectionLocked)
	{
		DetectionValue = DetectionMaxValue;
		UpdateEnemyStateFromDetection();
		return;
	}

	float NextDetectionValue = DetectionValue;

	if ((bIsTargetSensed || bIsTargetHeard) && CurrentTargetActor.IsValid())
	{
		float GainPerSecond = CalculateDetectionGainForTarget(CurrentTargetActor.Get());
		if (!bIsTargetSensed && bIsTargetHeard)
		{
			GainPerSecond *= HearingGainMultiplier;
		}
		NextDetectionValue = DetectionValue + (GainPerSecond * DeltaSeconds);
	}
	else if (!bSuspendDetectionDecrease)
	{
		NextDetectionValue = DetectionValue - (DetectionDecreaseRate * DeltaSeconds);
	}

	DetectionValue = FMath::Clamp(NextDetectionValue, 0.0f, DetectionMaxValue);
	if (DetectionValue >= DetectionMaxValue - KINDA_SMALL_NUMBER)
	{
		DetectionValue = DetectionMaxValue;
		bDetectionLocked = true;
	}
	UpdateEnemyStateFromDetection();

#if ENABLE_DRAW_DEBUG
	if (bShowDebugDetectionValue && GetPawn())
	{
		FVector TextLocation = GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 100.f);
		FString DebugStr = FString::Printf(TEXT("Detection: %.1f"), DetectionValue);
		DrawDebugString(GetWorld(), TextLocation, DebugStr, nullptr, FColor::Yellow, DetectionTickInterval, true);
	}
#endif
}

float AEnemyAIController::CalculateDetectionGainForTarget(const AActor* Target) const
{
	if (!Target)
	{
		return 0.0f;
	}

	const APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn)
	{
		return 0.0f;
	}

	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	const UCharacterMovementComponent* TargetMovement = TargetCharacter ? TargetCharacter->GetCharacterMovement() : nullptr;
	const bool bIsCrouched = TargetMovement && TargetMovement->IsCrouching();
	const float PostureMultiplier = bIsCrouched ? CrouchMultiplier : StandMultiplier;

	const float RawLightMultiplier = GetTargetLightMultiplier(Target);
	const float LightMultiplier = FMath::Clamp(RawLightMultiplier, LightMultiplierMin, LightMultiplierMax);

	const float Distance = FVector::Distance(OwnerPawn->GetActorLocation(), Target->GetActorLocation());
	float DistanceMultiplier = DistanceNearMultiplier;
	if (DistanceFar > DistanceNear + KINDA_SMALL_NUMBER)
	{
		DistanceMultiplier = FMath::GetMappedRangeValueClamped(
			FVector2D(DistanceNear, DistanceFar),
			FVector2D(DistanceNearMultiplier, DistanceFarMultiplier),
			Distance);
	}

	const float CombinedMultiplier = PostureMultiplier * LightMultiplier * DistanceMultiplier;
	return DetectionIncreaseRate * CombinedMultiplier;
}

void AEnemyAIController::UpdateEnemyStateFromDetection()
{
	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(GetPawn());
	if (!EnemyCharacter)
	{
		return;
	}

	FGameplayTag NewStateTag = MGSGameplayTags::State_Enemy_Clear;
	if (bDetectionLocked)
	{
		NewStateTag = bIsTargetSensed
			? MGSGameplayTags::State_Enemy_Combat
			: MGSGameplayTags::State_Enemy_Investigation;
	}
	else if (DetectionValue >= DetectionSuspiciousThreshold)
	{
		NewStateTag = MGSGameplayTags::State_Enemy_Suspicious;
	}

	EnemyCharacter->SetEnemyStateTagFromAI(NewStateTag);
}

bool AEnemyAIController::IsValidPerceptionTarget(const AActor* Actor) const
{
	const APawn* PawnTarget = Cast<APawn>(Actor);
	return PawnTarget && PawnTarget->IsPlayerControlled();
}

