/*
 * 파일명 : PlayerGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/BaseGameplayAbility.h"
#include "PlayerGameplayAbility.generated.h"

class UPlayerCombatComponent;
class AMGSPlayerController;
class APlayerCharacter;

UCLASS(Blueprintable, BlueprintType)
class MGSPROJECT_API UPlayerGameplayAbility : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerGameplayAbility();

	UFUNCTION(BlueprintPure, Category = "Ability")
	APlayerCharacter* GetPlayerCharacterFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Ability")
	AMGSPlayerController* GetMGSPlayerControllerFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Ability")
	UPlayerCombatComponent* GetPlayerCombatComponentFromActorInfo();

private:
	TWeakObjectPtr<APlayerCharacter> CachedPlayerCharacter;
	TWeakObjectPtr<AMGSPlayerController> CachedMGSPlayerController;
};
