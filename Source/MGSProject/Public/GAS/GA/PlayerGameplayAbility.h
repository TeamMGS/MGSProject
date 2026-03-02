/*
 * 파일명 : PlayerGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/BaseGameplayAbility.h"
#include "PlayerGameplayAbility.generated.h"

class UPlayerCombatComponent;
class AMGSPlayerController;
class APlayerCharacter;

UCLASS()
class MGSPROJECT_API UPlayerGameplayAbility : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Ability")
	APlayerCharacter* GetPlayerCharacterFromActorInfo();
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	AMGSPlayerController* GetMGSPlayerControllerFromActorInfo();
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	UPlayerCombatComponent* GetPlayerCombatComponentFromActorInfo();
	
private:
	// 플레이어 캐릭터의 정보를 가지고 있다면 캐시 TWeakObjectPtr는 공유 참조가 존재하지 않을 때 
	// 객체를 자동으로 해제합니다. (약한 포인터)
	TWeakObjectPtr<APlayerCharacter> CachedPlayerCharacter;
	TWeakObjectPtr<AMGSPlayerController> CachedMGSPlayerController;
	
};
