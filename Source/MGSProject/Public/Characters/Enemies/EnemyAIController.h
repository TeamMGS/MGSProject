/*
 * 파일명 : EnemyAIController.h
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-10
 */
#pragma once

#include "CoreMinimal.h"
#include "AI/Core/AICoreAIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "TimerManager.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;
class AActor;

/**
 * 
 */
UCLASS()
class MGSPROJECT_API AEnemyAIController : public AAICoreAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	UFUNCTION(BlueprintPure, Category = "AI|Detection")
	FVector GetLastSeenLocation() const { return LastSeenLocation; }

	UFUNCTION(BlueprintPure, Category = "AI|Detection")
	bool HasLastSeenLocation() const { return bHasLastSeenLocation; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Detection")
	float GetTargetLightMultiplier(const AActor* Target) const;
	virtual float GetTargetLightMultiplier_Implementation(const AActor* Target) const;

private:
	void UpdateDetection();
	float CalculateDetectionGainForTarget(const AActor* Target) const;
	void UpdateEnemyStateFromDetection();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.05"))
	float DetectionTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
	float DetectionIncreaseRate = 22.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
	float DetectionDecreaseRate = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "1.0"))
	float DetectionMaxValue = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
	float DetectionSuspiciousThreshold = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
	float DetectionInvestigationThreshold = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
	float DetectionCombatThreshold = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Posture", meta = (ClampMin = "0.0"))
	float StandMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Posture", meta = (ClampMin = "0.0"))
	float CrouchMultiplier = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Light", meta = (ClampMin = "0.0"))
	float LightMultiplierMin = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Light", meta = (ClampMin = "0.0"))
	float LightMultiplierMax = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance", meta = (ClampMin = "0.0"))
	float DistanceNear = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance", meta = (ClampMin = "0.0"))
	float DistanceFar = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance", meta = (ClampMin = "0.0"))
	float DistanceNearMultiplier = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Detection|Distance", meta = (ClampMin = "0.0"))
	float DistanceFarMultiplier = 0.4f;

	UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
	float DetectionValue = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
	bool bIsTargetSensed = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Detection", meta = (AllowPrivateAccess = true))
	FVector LastSeenLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
	bool bHasLastSeenLocation = false;

	UPROPERTY(VisibleInstanceOnly, Category = "AI|Detection")
	bool bDetectionLocked = false;

	TWeakObjectPtr<AActor> CurrentTargetActor;

	FTimerHandle DetectionTimerHandle;

};
