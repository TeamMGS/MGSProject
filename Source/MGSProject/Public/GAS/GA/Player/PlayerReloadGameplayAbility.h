/*
 * 파일명 : PlayerReloadGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Player/PlayerGameplayAbility.h"
#include "PlayerReloadGameplayAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerReloadGameplayAbility : public UPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerReloadGameplayAbility();

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
	// 재장전 처리 시 탄약 정보를 로그로 출력할지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bEnableReloadLog = true;
	
	// 이벤트 수신시 발생할 장전 함수 실행
	UFUNCTION()
	void OnAmmoRefillEventReceived(FGameplayEventData Payload);
};
