/*
 * 파일명 : PlayerPickupWeaponGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-10
 * 수정자 : 장대한
 * 수정일 ; 2026-03-10
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "PlayerPickupWeaponGameplayAbility.generated.h"

class ABaseWeapon;
class APlayerCharacter;

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
	// 무기 탐색
	void DrawPickupDebugSphere(const APlayerCharacter* PlayerCharacter, const ABaseWeapon* PickupCandidate) const;

	// 탐색 중심점을 캐릭터 전방으로 약간 이동시켜 등 뒤 무기 선택을 줄입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float PickupDebugForwardOffset = 60.0f;

	// 탐색 범위
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float PickupDebugRadius = 90.0f;
	
};
