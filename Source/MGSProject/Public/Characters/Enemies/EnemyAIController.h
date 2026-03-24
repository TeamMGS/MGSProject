/*
 * 파일명 : EnemyAIController.h
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-23
 */
#pragma once

#include "AI/Core/AICoreAIController.h"
#include "CoreMinimal.h"
#include "Perception/AIPerceptionTypes.h"
#include "TimerManager.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;
class AActor;

/**
 *
 */
UCLASS()
class MGSPROJECT_API AEnemyAIController : public AAICoreAIController {
  GENERATED_BODY()

public:
  AEnemyAIController();

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  FVector GetLastSeenLocation() const { return LastSeenLocation; }

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  bool HasLastSeenLocation() const { return bHasLastSeenLocation; }

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  FVector GetLastHeardLocation() const { return LastHeardLocation; }

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  bool HasLastHeardLocation() const { return bHasLastHeardLocation; }

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  AActor *GetCurrentTargetActor() const { return CurrentTargetActor.Get(); }

  UFUNCTION(BlueprintPure, Category = "AI|Detection")
  float GetDetectionValue() const { return DetectionValue; }

  /** Task 등에서 마지막 위치로 이동할 때 의심치 감소를 멈추기 위해 호출합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "AI|Detection")
  void SetSuspendDetectionDecrease(bool bSuspend);

  /** 의심치를 즉시 최대치로 설정하고 상태를 갱신합니다. */
  UFUNCTION(BlueprintCallable, Category = "AI|Detection")
  void SetDetectionToMax();

protected:
  virtual void OnPossess(APawn *InPawn) override;
  virtual void OnUnPossess() override;

  UFUNCTION()
  void HandleTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus);

  UFUNCTION(BlueprintNativeEvent, Category = "AI|Detection")
  float GetTargetLightMultiplier(const AActor *Target) const;
  virtual float
  GetTargetLightMultiplier_Implementation(const AActor *Target) const;

private:
  void UpdateDetection();
  float CalculateDetectionGainForTarget(const AActor *Target) const;
  void UpdateEnemyStateFromDetection();
  bool IsValidPerceptionTarget(const AActor *Actor) const;

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.05"))
  float DetectionTickInterval = 0.2f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float DetectionIncreaseRate = 22.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float DetectionDecreaseRate =
      5.0f; // 기존 18.0f 에서 수정되어 더 천천히 감소합니다.

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "1.0"))
  float DetectionMaxValue = 100.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float DetectionSuspiciousThreshold = 25.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float DetectionInvestigationThreshold = 100.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float DetectionCombatThreshold = 100.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Posture",
            meta = (ClampMin = "0.0"))
  float StandMultiplier = 1.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Posture",
            meta = (ClampMin = "0.0"))
  float CrouchMultiplier = 0.6f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Light",
            meta = (ClampMin = "0.0"))
  float LightMultiplierMin = 0.35f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Light",
            meta = (ClampMin = "0.0"))
  float LightMultiplierMax = 1.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Hearing",
            meta = (ClampMin = "0.0"))
  float HearingGainMultiplier = 0.6f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance",
            meta = (ClampMin = "0.0"))
  float DistanceNear = 250.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance",
            meta = (ClampMin = "0.0"))
  float DistanceFar = 2000.0f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance",
            meta = (ClampMin = "0.0"))
  float DistanceNearMultiplier = 1.2f;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance",
            meta = (ClampMin = "0.0"))
  float DistanceFarMultiplier = 0.4f;

  UPROPERTY(EditAnywhere, Category = "AI|Detection|Debug")
  bool bShowDebugDetectionValue = true;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Detection",
            meta = (AllowPrivateAccess = true))
  float DetectionValue = 0.0f;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  bool bIsTargetSensed = false;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  bool bIsTargetHeard = false;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Detection",
            meta = (AllowPrivateAccess = true))
  FVector LastSeenLocation = FVector::ZeroVector;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  bool bHasLastSeenLocation = false;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Detection",
            meta = (AllowPrivateAccess = true))
  FVector LastHeardLocation = FVector::ZeroVector;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  bool bHasLastHeardLocation = false;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  bool bDetectionLocked = false;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Detection",
            meta = (AllowPrivateAccess = true))
  bool bSuspendDetectionDecrease = false;

  UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
            meta = (ClampMin = "0.0"))
  float MaxCombatDuration = 10.0f;

  UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
  float CurrentCombatTimer = 0.0f;

  TWeakObjectPtr<AActor> CurrentTargetActor;

  FTimerHandle DetectionTimerHandle;
};
