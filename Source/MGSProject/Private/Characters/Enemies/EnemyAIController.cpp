/*
 * 파일명 : EnemyAIController.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-23
 */

#include "Characters/Enemies/EnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/EnemyCharacter.h"
#include "DrawDebugHelpers.h"
#include "GAS/MGSGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayStateTreeModule/Public/Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"


AEnemyAIController::AEnemyAIController() {
  StateTreeComponent =
      CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

  // Match AI StateTree expectations: start logic on possess and attach to pawn
  // for EQS.
  bStartAILogicOnPossess = true;
  bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  if (UAIPerceptionComponent *LocalPerceptionComponent =
          GetPerceptionComponent()) {
    LocalPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
        this, &ThisClass::HandleTargetPerceptionUpdated);
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

  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().SetTimer(DetectionTimerHandle, this,
                                      &ThisClass::UpdateDetection,
                                      DetectionTickInterval, true);
  }

  if (!StateTreeComponent) {
    return;
  }

  StateTreeComponent->StartLogic();
}

void AEnemyAIController::OnUnPossess() {
  if (UAIPerceptionComponent *LocalPerceptionComponent =
          GetPerceptionComponent()) {
    LocalPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
        this, &ThisClass::HandleTargetPerceptionUpdated);
  }

  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().ClearTimer(DetectionTimerHandle);
  }

  if (StateTreeComponent) {
    StateTreeComponent->StopLogic(TEXT("UnPossess"));
  }

  Super::OnUnPossess();
}

void AEnemyAIController::SetSuspendDetectionDecrease(bool bSuspend) {
  bSuspendDetectionDecrease = bSuspend;
}

void AEnemyAIController::SetDetectionToMax() {
  DetectionValue = DetectionMaxValue;
  bDetectionLocked = true; // 값이 최대로 도달했으므로 상태를 잠금 처리
  UpdateEnemyStateFromDetection();
}

void AEnemyAIController::SetDetectionLocked(bool bLocked) {
  bDetectionLocked = bLocked;
  if (bLocked) {
    DetectionValue = DetectionMaxValue;
  }
  UpdateEnemyStateFromDetection();
}

void AEnemyAIController::HandleTargetPerceptionUpdated(AActor *Actor,
                                                       FAIStimulus Stimulus) {
  if (!Actor) {
    return;
  }

  // 아군(Enemy) 시야 판정: 시야에 들어온 액터가 아군이고 사망 상태에 있다면 발견 처리
  if (AEnemyCharacter* SeenEnemy = Cast<AEnemyCharacter>(Actor)) {
    if (Stimulus.WasSuccessfullySensed()) {
      const UClass* SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
      if (SenseClass == UAISense_Sight::StaticClass()) {
        if (UMGSAbilitySystemComponent* EnemyASC = SeenEnemy->GetMGSAbilitySystemComponent()) {
          // 해당 아군이 사망 태그(State.Character.Dead)를 가졌는지 확인
          if (EnemyASC->HasMatchingGameplayTag(MGSGameplayTags::State_Character_Dead)) {
            if (Stimulus.WasSuccessfullySensed()) {
              // 시체를 발견했으므로 의심도를 즉시 최대로 설정
              SetDetectionToMax();
              
              // 시체가 발견된 장소로 플레이어 수색을 유도하기 위해 위치 정보를 갱신
              LastSeenLocation = Stimulus.StimulusLocation;
              bHasLastSeenLocation = true;

              // 블랙보드 키 "S_SightTargetLastLocation"에 시체 발견 위치 기록 및 "S_DetectFix" 잠금
              if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent()) {
                BlackboardComp->SetValueAsVector(FName("S_SightTargetLastLocation"), Stimulus.StimulusLocation);
                BlackboardComp->SetValueAsBool(FName("S_DetectFix"), true);
              }
            } else {
              // 시체가 시야에서 벗어남
              if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent()) {
                BlackboardComp->SetValueAsBool(FName("S_DetectFix"), false);
              }
            }
            return;
          }
        }
      }
    }
  }

  if (!IsValidPerceptionTarget(Actor)) {
    return;
  }

  const UClass *SenseClass =
      UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
  if (SenseClass == UAISense_Sight::StaticClass()) {
    CurrentTargetActor = Actor;
    bIsTargetSensed = Stimulus.WasSuccessfullySensed();
    if (bIsTargetSensed) {
      LastSeenLocation = Stimulus.StimulusLocation;
      bHasLastSeenLocation = true;
    }
    return;
  }

  if (SenseClass == UAISense_Hearing::StaticClass()) {
    CurrentTargetActor = Actor;
    bIsTargetHeard = Stimulus.WasSuccessfullySensed();
    if (bIsTargetHeard) {
      LastHeardLocation = Stimulus.StimulusLocation;
      bHasLastHeardLocation = true;
    }
    return;
  }
}

float AEnemyAIController::GetTargetLightMultiplier_Implementation(
    const AActor *Target) const {
  return 1.0f;
}

void AEnemyAIController::UpdateDetection() {
  const float DeltaSeconds = DetectionTickInterval;
  if (CurrentTargetActor.IsValid() &&
      !IsValidPerceptionTarget(CurrentTargetActor.Get())) {
    CurrentTargetActor.Reset();
    bIsTargetSensed = false;
    bIsTargetHeard = false;
    bHasLastHeardLocation = false;
  }
  if (bDetectionLocked) {
    DetectionValue = DetectionMaxValue;
    UpdateEnemyStateFromDetection();
    return;
  }

  float NextDetectionValue = DetectionValue;

  if ((bIsTargetSensed || bIsTargetHeard) && CurrentTargetActor.IsValid()) {
    float GainPerSecond =
        CalculateDetectionGainForTarget(CurrentTargetActor.Get());
    if (!bIsTargetSensed && bIsTargetHeard) {
      GainPerSecond *= HearingGainMultiplier;
    }
    NextDetectionValue = DetectionValue + (GainPerSecond * DeltaSeconds);
  } else if (!bSuspendDetectionDecrease) {
    NextDetectionValue =
        DetectionValue - (DetectionDecreaseRate * DeltaSeconds);
  }

  DetectionValue = FMath::Clamp(NextDetectionValue, 0.0f, DetectionMaxValue);
  if (DetectionValue >= DetectionMaxValue - KINDA_SMALL_NUMBER) {
    DetectionValue = DetectionMaxValue;
    bDetectionLocked = true;
  }
  UpdateEnemyStateFromDetection();

#if ENABLE_DRAW_DEBUG
  if (bShowDebugDetectionValue && GetPawn()) {
    FVector TextLocation =
        GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 100.f);
    FString DebugStr = FString::Printf(TEXT("Detection: %.1f"), DetectionValue);
    DrawDebugString(GetWorld(), TextLocation, DebugStr, nullptr, FColor::Yellow,
                    DetectionTickInterval, true);
  }
#endif
}

float AEnemyAIController::CalculateDetectionGainForTarget(
    const AActor *Target) const {
  if (!Target) {
    return 0.0f;
  }

  const APawn *OwnerPawn = GetPawn();
  if (!OwnerPawn) {
    return 0.0f;
  }

  const ACharacter *TargetCharacter = Cast<ACharacter>(Target);
  const UCharacterMovementComponent *TargetMovement =
      TargetCharacter ? TargetCharacter->GetCharacterMovement() : nullptr;
  const bool bIsCrouched = TargetMovement && TargetMovement->IsCrouching();
  const float PostureMultiplier =
      bIsCrouched ? CrouchMultiplier : StandMultiplier;

  const float RawLightMultiplier = GetTargetLightMultiplier(Target);
  const float LightMultiplier =
      FMath::Clamp(RawLightMultiplier, LightMultiplierMin, LightMultiplierMax);

  const float Distance = FVector::Distance(OwnerPawn->GetActorLocation(),
                                           Target->GetActorLocation());
  float DistanceMultiplier = DistanceNearMultiplier;
  if (DistanceFar > DistanceNear + KINDA_SMALL_NUMBER) {
    DistanceMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(DistanceNear, DistanceFar),
        FVector2D(DistanceNearMultiplier, DistanceFarMultiplier), Distance);
  }

  const float CombinedMultiplier =
      PostureMultiplier * LightMultiplier * DistanceMultiplier;
  return DetectionIncreaseRate * CombinedMultiplier;
}

void AEnemyAIController::UpdateEnemyStateFromDetection() {
  AEnemyCharacter *EnemyCharacter = Cast<AEnemyCharacter>(GetPawn());
  if (!EnemyCharacter) {
    return;
  }

  FGameplayTag NewStateTag = MGSGameplayTags::State_Enemy_Clear;
  if (bDetectionLocked) {
    NewStateTag = bIsTargetSensed ? MGSGameplayTags::State_Enemy_Combat
                                  : MGSGameplayTags::State_Enemy_Investigation;
  } else if (DetectionValue >= DetectionSuspiciousThreshold) {
    NewStateTag = MGSGameplayTags::State_Enemy_Suspicious;
  }

  EnemyCharacter->SetEnemyStateTagFromAI(NewStateTag);

  // Behavior Tree 용 블랙보드 업데이트
  if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent()) {
    BlackboardComp->SetValueAsBool(FName("bIsInCombat"), bDetectionLocked);
    BlackboardComp->SetValueAsBool(FName("bCanSeePlayer"), bIsTargetSensed);
  }
}

bool AEnemyAIController::IsValidPerceptionTarget(const AActor *Actor) const {
  const APawn *PawnTarget = Cast<APawn>(Actor);
  return PawnTarget && PawnTarget->IsPlayerControlled();
}
