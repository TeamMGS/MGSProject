/*
 * 파일명 : BaseGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BaseGameplayAbility.generated.h"

class UPawnCombatComponent;
class UMGSAbilitySystemComponent;

UENUM(BlueprintType)
enum class EBaseAbilityActivationPolicy : uint8
{
	OnTriggered,
	OnGiven
};

UCLASS()
class MGSPROJECT_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
		
	UFUNCTION(BlueprintPure, Category = "Ability")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	UMGSAbilitySystemComponent* GetMGSAbilitySystemComponentFromActorInfo() const;
		
protected:
	UPROPERTY(EditDefaultsOnly, Category = "BaseAibility")
	EBaseAbilityActivationPolicy AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	
};
