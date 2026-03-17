/*
 * 파일명 : BaseGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/BaseGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"

UBaseGameplayAbility::UBaseGameplayAbility()
{
	// 캐릭터가 죽음 상태일 때는 GA가 발동하지 않음
	ActivationBlockedTags.AddTag(MGSGameplayTags::State_Character_Dead);
}

void UBaseGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (AbilityActivationPolicy != EBaseAbilityActivationPolicy::OnGiven || Spec.IsActive())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UBaseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AbilityActivationPolicy == EBaseAbilityActivationPolicy::OnGiven && bClearAbilityOnEndWhenGiven && Handle.IsValid())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UPawnCombatComponent* UBaseGameplayAbility::GetPawnCombatComponentFromActorInfo() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return nullptr;
	}

	return AvatarActor->FindComponentByClass<UPawnCombatComponent>();
}

UMGSAbilitySystemComponent* UBaseGameplayAbility::GetMGSAbilitySystemComponentFromActorInfo() const
{
	if (!CurrentActorInfo)
	{
		return nullptr;
	}

	return Cast<UMGSAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
}
