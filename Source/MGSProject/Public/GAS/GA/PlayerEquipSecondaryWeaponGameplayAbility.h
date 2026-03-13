/*
 * 파일명 : PlayerEquipSecondaryWeaponGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "PlayerEquipSecondaryWeaponGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerEquipSecondaryWeaponGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerEquipSecondaryWeaponGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
private:
	// 델리게이트 응답을 위한 함수 선언
	// 애니메이션 이벤트 수신 콜백
	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageFinished();
};
