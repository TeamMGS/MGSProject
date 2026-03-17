/*
 * 파일명 : PlayerTraversalGameplayAbility.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-16
 * 수정자 : 김동석
 * 수정일 : 2026-03-16
 */
#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "Components/TraversalComponent/MGSTraversalComponent.h"
#include "PlayerTraversalGameplayAbility.generated.h"

class UChooserTable;

/**
 * 
 */
UCLASS()
class MGSPROJECT_API UPlayerTraversalGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPlayerTraversalGameplayAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle SpecHandle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 에디터에서 할당할 Chooser Table
	UPROPERTY(EditDefaultsOnly, Category = "MGS|Traversal")
	TObjectPtr<UChooserTable> TraversalChooserTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "MGS|Traversal|Debug")
	TObjectPtr<UAnimMontage> DebugMontage;
	
private:
	// Motion Warping 타겟 설정
	void SetMotionWarpingTargets(const FMGSTraversalChooserInputs& Inputs);

	// 애니메이션 종료 콜백
	UFUNCTION()
	void OnTraversalFinished();
	
	UFUNCTION()
	void OnTraversalStopEventReceived(FGameplayEventData Payload);
	
	// 헬퍼함수
	void RestoreMovementState(APlayerCharacter* Character, EMovementMode OriginalMode);
};
