/*
 * 파일명 : PlayerGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
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

	// ActorInfo로 플레이어 캐릭터 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	APlayerCharacter* GetPlayerCharacterFromActorInfo();

	// ActorInfo로 플레이어 컨트롤러 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	AMGSPlayerController* GetMGSPlayerControllerFromActorInfo();

	// ActorInfo로 플레이어 컴뱃 컴포넌트 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	UPlayerCombatComponent* GetPlayerCombatComponentFromActorInfo();

private:
	TWeakObjectPtr<APlayerCharacter> CachedPlayerCharacter; // 플레이어 캐릭터 캐시
	TWeakObjectPtr<AMGSPlayerController> CachedMGSPlayerController; // 플레이어 컨트롤러 캐시
	
};
