/*
 * 파일명 : EnemyGameplayAbility.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/BaseGameplayAbility.h"
#include "EnemyGameplayAbility.generated.h"

class UEnemyCombatComponent;
class AEnemyCharacter;
struct FGameplayTag;

UCLASS()
class MGSPROJECT_API UEnemyGameplayAbility : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	// ActorInfo로 적 캐릭터 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	AEnemyCharacter* GetEnemyCharacterFromActorInfo();
	
	// ActorInfo로 적 컴뱃 컴포넌트 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo();

protected:
	void ExecuteEnemyGameplayCue(const FGameplayTag& CueTag, const FVector& CueLocation, UObject* SourceObject = nullptr);
	
private:
	TWeakObjectPtr<AEnemyCharacter> CachedEnemyCharacter; // 적 캐릭터 캐싱
	
};
