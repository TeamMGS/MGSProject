/*
 * 파일명 : MGSAbilitySystemComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/ASC/MGSAbilitySystemComponent.h"

#include "Components/Combat/PawnCombatComponent.h"
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

	// GetDynamicSpecSourceTags에 현재 InputTag가 있는 스펙 핸들 수집
	TArray<FGameplayAbilitySpecHandle> MatchingSpecHandles;
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			MatchingSpecHandles.Add(Spec.Handle);
		}
	}

	TArray<FGameplayAbilitySpecHandle> PendingActivateHandles;
	for (const FGameplayAbilitySpecHandle& SpecHandle : MatchingSpecHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle);
		if (!Spec)
		{
			continue;
		}

		const bool bWasActive = Spec->IsActive();

		// 수집된 각 스펙에 대해 AbilitySpecInputPressed 호출(로컬 ASC 내부 처리)
		AbilitySpecInputPressed(*Spec);

		// 입력 시점에 이미 활성 상태였다면 재활성화 후보에 넣지 않습니다.
		// (InputPressed 내부에서 End/Cancel된 경우 같은 프레임 재활성화 방지)
		if (bWasActive)
		{
			if (Spec->IsActive())
			{
				const TArray<UGameplayAbility*> Instances = Spec->GetAbilityInstances();
				const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfo();
				// 여전히 활성 상태인 경우에만 InputPressed 복제 이벤트를 전파합니다.
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec->Handle, ActivationInfo.GetActivationPredictionKey());
			}

			continue;
		}

		// 비활성 스펙이면 PendingActivateHandles에 담아 이후 호출
		// 순회 중 변이(뮤테이션) 방지 : TryActivateAbility는 내부적으로 Ability 상태/태그/스펙 상태를 바꿀 수 있음 
		// 핸들 수집 -> 입력 눌림 처리 -> 활성화
		PendingActivateHandles.Add(Spec->Handle);
	}

	for (const FGameplayAbilitySpecHandle& PendingHandle : PendingActivateHandles)
	{
		TryActivateAbility(PendingHandle);
	}
}

void UMGSAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	PressedAbilityInputTags.RemoveTag(InputTag);

	TArray<FGameplayAbilitySpecHandle> MatchingSpecHandles;
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			MatchingSpecHandles.Add(Spec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : MatchingSpecHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle);
		if (!Spec)
		{
			continue;
		}

		AbilitySpecInputReleased(*Spec);

		if (Spec->IsActive())
		{
			const TArray<UGameplayAbility*> Instances = Spec->GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfo();
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec->Handle, ActivationInfo.GetActivationPredictionKey());
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

		// 유효한 항목 FGameplayAbilitySpec 생성
		FGameplayAbilitySpec Spec(WeaponAbilitySet.AbilityToGrant);
		Spec.SourceObject = GetAvatarActor();
		Spec.Level = Level;
		Spec.GetDynamicSpecSourceTags().AddTag(WeaponAbilitySet.InputTag);
		// 반환 핸들 저장 : 무기 교체 시 그 무기가 부여한 Ability만 정확히 제거
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(Spec));
	}
}

void UMGSAbilitySystemComponent::RemoveGrantedWeaponAbilities(const TArray<FGameplayAbilitySpecHandle>& SpecHandlesToRemove)
{
	if (SpecHandlesToRemove.IsEmpty())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : SpecHandlesToRemove)
	{
		if (SpecHandle.IsValid())
		{
			ClearAbility(SpecHandle);
		}
	}
}

bool UMGSAbilitySystemComponent::IsAbilityInputTagPressed(const FGameplayTag& InputTag) const
{
	return InputTag.IsValid() && PressedAbilityInputTags.HasTagExact(InputTag);
}
