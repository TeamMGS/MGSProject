/*
 * 파일명 : PlayerCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Weapon/BaseWeapon.h"
#include "PlayerCombatComponent.generated.h"

class ABaseWeapon;

UCLASS()
class MGSPROJECT_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
	// 생성된 무기 등록
	UFUNCTION(BlueprintCallable, Category = "Player")
	void RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon, bool bRegisterAsEquippedWeapon = false);
	
	// 캐릭터가 휴대하는 무기
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCarriedWeaponByTag(FGameplayTag WeaponTag) const;
	
	// 캐릭터가 장착중인 무기의 태그
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	FGameplayTag CurrentEquippedWeaponTag;
	
	// 캐릭터가 장착한 무기
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCurrentEquippedWeapon() const;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ABaseWeapon* GetPlayerCarriedWeaponByTag(FGameplayTag Tag) const;
	
private:
	TMap<FGameplayTag, ABaseWeapon*> CharacterCarriedWeaponMap;
};
