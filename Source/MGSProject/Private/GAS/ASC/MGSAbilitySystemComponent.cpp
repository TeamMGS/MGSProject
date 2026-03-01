/*
 * 파일명 : MGSAbilitySystemComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#include "GAS/ASC/MGSAbilitySystemComponent.h"

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
