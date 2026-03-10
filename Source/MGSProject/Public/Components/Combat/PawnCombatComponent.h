/*
 * 파일명 : PawnCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponent.h"
#include "MGSStructType.h"
#include "PawnCombatComponent.generated.h"

class ABaseWeapon;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEquippedWeaponChangedSignature, FGameplayTag /*PreviousWeaponTag*/, FGameplayTag /*CurrentWeaponTag*/);

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

	FOnEquippedWeaponChangedSignature& GetOnEquippedWeaponChangedDelegate() { return OnEquippedWeaponChanged; }

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
	bool AttachWeaponToSocket(ABaseWeapon* Weapon, FName SocketName) const; // 소켓에 무기 장착
	void AddWeaponInputMappingContext(ABaseWeapon* Weapon) const; // 무기 입력 매핑
	void RemoveWeaponInputMappingContext(ABaseWeapon* Weapon) const; // 무기 입력 매핑 제거
	void ApplyWeaponAbilities(ABaseWeapon* Weapon) const; // 무기 능력 제거
	void RemoveWeaponAbilities(ABaseWeapon* Weapon) const; // 무기 능력 추가
	void SaveCurrentWeaponRuntimeState(); // 무기 런타임 탄약값 저장
	void ApplyWeaponRuntimeState(FGameplayTag WeaponTag, ABaseWeapon* Weapon); // 무기 런타임 탄약값 적용

	TMap<FGameplayTag, ABaseWeapon*> CharacterCarriedWeaponMap; // 소유 무기 맵
	TMap<FGameplayTag, FWeaponRuntimeState> CharacterCarriedWeaponRuntimeStateMap; // 무기별 런타임 탄약 상태 맵 
	FOnEquippedWeaponChangedSignature OnEquippedWeaponChanged; // 장착 변경 델리게이트
	
};
