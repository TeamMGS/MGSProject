/*
 * 파일명 : PlayerGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "GAS/GA/PlayerGameplayAbility.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"

UPlayerGameplayAbility::UPlayerGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

APlayerCharacter* UPlayerGameplayAbility::GetPlayerCharacterFromActorInfo()
{
	if (!CachedPlayerCharacter.IsValid())
	{
		CachedPlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	}

	return CachedPlayerCharacter.Get();
}

AMGSPlayerController* UPlayerGameplayAbility::GetMGSPlayerControllerFromActorInfo()
{
	if (!CachedMGSPlayerController.IsValid())
	{
		if (!CurrentActorInfo)
		{
			return nullptr;
		}

		CachedMGSPlayerController = Cast<AMGSPlayerController>(CurrentActorInfo->PlayerController.Get());
	}

	return CachedMGSPlayerController.Get();
}

UPlayerCombatComponent* UPlayerGameplayAbility::GetPlayerCombatComponentFromActorInfo()
{
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		return PlayerCharacter->GetPlayerCombatComponent();
	}

	return nullptr;
}


