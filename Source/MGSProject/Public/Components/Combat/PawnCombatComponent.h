/*
 * 파일명 : PawnCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponent.h"
#include "MGSStructType.h"
#include "PawnCombatComponent.generated.h"

class ABaseWeapon;

UCLASS()
class MGSPROJECT_API UPawnCombatComponent : public UPawnExtensionComponent
{
	GENERATED_BODY()

public:
	// 생성한 무기를 캐릭터 소유 목록에 등록합니다.
	UFUNCTION(BlueprintCallable, Category = "Player")
	void RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon, bool bRegisterAsEquippedWeapon = false);

	// 전달한 태그로 소유 중인 무기를 조회합니다.
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCarriedWeaponByTag(FGameplayTag WeaponTag) const;

	// 현재 장착 중인 무기 태그
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	FGameplayTag CurrentEquippedWeaponTag;

	// 현재 장착 무기 조회
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCurrentEquippedWeapon() const;

	// 태그로 들고 있는 장비 조회
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ABaseWeapon* GetPlayerCarriedWeaponByTag(FGameplayTag Tag) const;

	// 전달받은 태그의 무기를 장착합니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipWeaponByTag(FGameplayTag WeaponTag);

	// 현재 장착 무기를 해제(홀스터 이동/어빌리티 제거)합니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool UnequipCurrentWeapon();

private:
	bool AttachWeaponToSocket(ABaseWeapon* Weapon, FName SocketName) const;
	void AddWeaponInputMappingContext(ABaseWeapon* Weapon) const;
	void RemoveWeaponInputMappingContext(ABaseWeapon* Weapon) const;
	void ApplyWeaponAbilities(ABaseWeapon* Weapon) const;
	void RemoveWeaponAbilities(ABaseWeapon* Weapon) const;
	void SaveCurrentWeaponRuntimeState();
	void ApplyWeaponRuntimeState(FGameplayTag WeaponTag, ABaseWeapon* Weapon);

	TMap<FGameplayTag, ABaseWeapon*> CharacterCarriedWeaponMap;
	TMap<FGameplayTag, FWeaponRuntimeState> CharacterCarriedWeaponRuntimeStateMap;
};
