/*
 * 파일명 : MGSAbilitySystemComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "GAS/ASC/MGSAbilitySystemComponent.h"

#include "GAS/GA/PlayerGameplayAbility.h"
#include "GAS/MGSGameplayTags.h"
#include "MGSStructType.h"

void UMGSAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	PressedAbilityInputTags.AddTag(InputTag);

	if (InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Crouch))
	{
		bool bHasActiveCrouchAbility = false;

		for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (!Spec.IsActive() || !Spec.Ability)
			{
				continue;
			}

			if (Spec.Ability->AbilityTags.HasTagExact(MGSGameplayTags::Ability_Player_Crouch))
			{
				bHasActiveCrouchAbility = true;
				break;
			}
		}

		if (bHasActiveCrouchAbility)
		{
			FGameplayTagContainer CrouchAbilityTags;
			CrouchAbilityTags.AddTag(MGSGameplayTags::Ability_Player_Crouch);
			CancelAbilities(&CrouchAbilityTags);
			return;
		}
	}

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputPressed(Spec);

		if (Spec.IsActive())
		{
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
			continue;
		}

		TryActivateAbility(Spec.Handle);
	}
}

void UMGSAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	PressedAbilityInputTags.RemoveTag(InputTag);

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputReleased(Spec);

		if (Spec.IsActive())
		{
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
		}
	}
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

bool UMGSAbilitySystemComponent::IsAbilityInputTagPressed(const FGameplayTag& InputTag) const
{
	return InputTag.IsValid() && PressedAbilityInputTags.HasTagExact(InputTag);
}
