/*
 * 파일명 : PlayerFlashGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-19
 * 수정자 : 장대한
 * 수정일 : 2026-03-19
 */

#include "GAS/GA/Player/PlayerFlashGameplayAbility.h"

#include "Characters/Player/PlayerCharacter.h"
#include "GAS/MGSGameplayTags.h"

UPlayerFlashGameplayAbility::UPlayerFlashGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Flash);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

void UPlayerFlashGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	PlayerCharacter->OnFlash();
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
