#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerPickupWeaponGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerPickupWeaponGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerPickupWeaponGameplayAbility();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float PickupDebugForwardOffset = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float PickupDebugRadius = 90.0f;
};
