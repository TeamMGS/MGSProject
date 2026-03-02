/*
 * 파일명 : PlayerGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "GAS/GA/PlayerGameplayAbility.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"

APlayerCharacter* UPlayerGameplayAbility::GetPlayerCharacterFromActorInfo()
{
	if (!CachedPlayerCharacter.IsValid())
	{
		CachedPlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor);
	}
	
	return CachedPlayerCharacter.IsValid() ? CachedPlayerCharacter.Get() : nullptr;
}

AMGSPlayerController* UPlayerGameplayAbility::GetMGSPlayerControllerFromActorInfo()
{
	if (!CachedMGSPlayerController.IsValid())
	{
		CachedMGSPlayerController = Cast<AMGSPlayerController>(CurrentActorInfo->PlayerController);
	}
	
	return CachedMGSPlayerController.IsValid() ? CachedMGSPlayerController.Get() : nullptr;
}

UPlayerCombatComponent* UPlayerGameplayAbility::GetPlayerCombatComponentFromActorInfo()
{
	return GetPlayerCharacterFromActorInfo()->GetPlayerCombatComponent();
}
