/*
 * 파일명 : BaseGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BaseGameplayAbility.generated.h"

class UPawnCombatComponent;
class UMGSAbilitySystemComponent;

// GA 정책
UENUM(BlueprintType)
enum class EBaseAbilityActivationPolicy : uint8
{
	OnTriggered, // 수동/입력 기반 실행
	OnGiven // 지급 즉시 1회성/자동 실행
};

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBaseGameplayAbility();

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// ActorInfo로 Pawn CombatComponent를 가져온다.
	UFUNCTION(BlueprintPure, Category = "Ability")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;

	// ActorInfo로 MGSASC(MGSAbilitySystemComponent)를 가져온다. 
	UFUNCTION(BlueprintPure, Category = "Ability")
	UMGSAbilitySystemComponent* GetMGSAbilitySystemComponentFromActorInfo() const;

protected:
	// GA 정책
	UPROPERTY(EditDefaultsOnly, Category = "BaseAibility")
	EBaseAbilityActivationPolicy AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;

	// OnGiven GA를 EndAbility 시 자동으로 제거할 지 결정하는 플래그
	UPROPERTY(EditDefaultsOnly, Category = "BaseAibility")
	bool bClearAbilityOnEndWhenGiven = true;
	
};
