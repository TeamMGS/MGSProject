/*
 * 파일명 : BaseGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "GAS/GA/BaseGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"

void UBaseGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	if (AbilityActivationPolicy == EBaseAbilityActivationPolicy::OnGiven)
	{
		// 액터의 정보를 받아올 수 있거나 해당 어빌리티가 활성화 되지 않았으면
		if (ActorInfo && !Spec.IsActive())
		{
			// 현재 가지고 있는 어빌리티를 발동시킨다.
			ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		}
	}
}

void UBaseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AbilityActivationPolicy == EBaseAbilityActivationPolicy::OnGiven)
	{
		// 어빌리트가 유효한지 체크
		if (ActorInfo)
		{
			// 클리어
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UPawnCombatComponent* UBaseGameplayAbility::GetPawnCombatComponentFromActorInfo() const
{
	return GetAvatarActorFromActorInfo()->FindComponentByClass<UPawnCombatComponent>();
}

UMGSAbilitySystemComponent* UBaseGameplayAbility::GetMGSAbilitySystemComponentFromActorInfo() const
{
	return Cast<UMGSAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
}
