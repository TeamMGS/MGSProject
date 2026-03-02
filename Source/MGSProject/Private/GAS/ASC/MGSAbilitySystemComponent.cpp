/*
 * 파일명 : MGSAbilitySystemComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#include "GAS/ASC/MGSAbilitySystemComponent.h"

#include "GAS/GA/PlayerGameplayAbility.h"
#include "MGSStructType.h"

void UMGSAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	
	// 주입한 스펙으로 입력 실행
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}
		
		TryActivateAbility(Spec.Handle);
	}
}

void UMGSAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
}

void UMGSAbilitySystemComponent::GrantWeaponAbilities(const TArray<FPlayerAbilitySet>& WeaponAbilities, int32 Level,
	TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if (WeaponAbilities.IsEmpty())
	{
		return;
	}
	
	for (const FPlayerAbilitySet& WeaponAbilitySet : WeaponAbilities)
	{
		if (!WeaponAbilitySet.IsValid())
		{
			continue;
		}
		
		FGameplayAbilitySpec Spec(WeaponAbilitySet.AbilityToGrant);
		Spec.SourceObject = GetAvatarActor();
		Spec.Level = Level;
		Spec.GetDynamicSpecSourceTags().AddTag(WeaponAbilitySet.InputTag);
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(Spec));
	}
}

void UMGSAbilitySystemComponent::RemoveGrantedWeaponAbilities(TArray<FGameplayAbilitySpecHandle>& SpecHandlesToRemove)
{
	if (SpecHandlesToRemove.IsEmpty())
	{
		return;
	}
	
	for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandlesToRemove)
	{
		if (SpecHandle.IsValid())
		{
			ClearAbility(SpecHandle);
		}
	}
	
	SpecHandlesToRemove.Empty();
}
