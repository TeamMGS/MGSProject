/*
 * 파일명 : PawnCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-10
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
	FOnEquippedWeaponChangedSignature& GetOnEquippedWeaponChangedDelegate() { return OnEquippedWeaponChanged; }
	
	// 게임 시작 시 최초 무기 등록 (BP권장)
	UFUNCTION(BlueprintCallable, Category = "Player")
	void RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon, const bool bRegisterAsEquippedWeapon = false);

	// 태그(Key)로 소유중인 무기 조회
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCarriedWeaponByTag(const FGameplayTag& WeaponTag) const;

	// 현재 장착 무기 조회
	UFUNCTION(BlueprintCallable, Category = "Player")
	ABaseWeapon* GetCharacterCurrentEquippedWeapon() const;

	// 태그(주무기, 보조무기)에 해당하는 무기 장착
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipWeaponByTag(const FGameplayTag& WeaponTag);

	// 현재 장착 무기 장비 해제 (소켓 홀스터로 이동 및 어빌리티, 입력 제거)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool UnequipCurrentWeapon();

	// 월드에 떨어진 무기를 지정 슬롯 태그로 줍고, 기존 슬롯 무기와 교체합니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PickupDroppedWeaponByTag(const FGameplayTag& WeaponTag, ABaseWeapon* DroppedWeapon);

	// 지정 슬롯 무기를 캐릭터 소유에서 해제하고 월드에 드롭합니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool DropCarriedWeaponByTag(const FGameplayTag& WeaponTag, const FVector& WorldLocation, const FRotator& WorldRotation);

protected:
	// 무기 등록/획득/장착 시 발사체 풀을 미리 예열합니다.
	void PrewarmProjectilePoolForWeapon(ABaseWeapon* Weapon) const;
	// 런타임 무기 정보 적용
	void ApplyWeaponRuntimeState(const FGameplayTag& WeaponTag, ABaseWeapon* Weapon);
	// 무기가 갖고 있는 Ability 부여
	void ApplyWeaponAbilities(ABaseWeapon* Weapon) const;
	// 무기가 갖고 있는 IMC 맵핑
	void AddWeaponInputMappingContext(const ABaseWeapon* Weapon) const;
	// 소켓에 무기 적재
	bool AttachWeaponToSocket(ABaseWeapon* Weapon, const FName& SocketName) const;
	
	// 런타임 무기 정보 저장
	void SaveCurrentWeaponRuntimeState();
	// 무기가 갖고 있는 Ability 제거
	void RemoveWeaponAbilities(ABaseWeapon* Weapon) const;
	// 무기가 갖고 있는 IMC 제거
	void RemoveWeaponInputMappingContext(const ABaseWeapon* Weapon) const;
	
	// 무기 줍기 시 가지고 있던 무기를 떨어뜨릴 전방 위치 (캐릭터 위치 기준)
	static constexpr float PickupSwapDropForwardOffset = 70.0f;
	// 무기 줍기 시 가지고 있던 무기를 떨어뜨릴 높이 (캐릭터 위치 기준)
	static constexpr float PickupSwapDropHeightOffset = 90.0f;

	// 현재 장착 중인 무기 태그
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	FGameplayTag CurrentEquippedWeaponTag;
	
	// 소유 무기 (Key: Tag, Value: Class)
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<ABaseWeapon>> CharacterCarriedWeaponMap;
	// 소유 무기 런타임 정보(탄약, 탄창) (Key: Tag, Value: Struct)
	TMap<FGameplayTag, FWeaponRuntimeState> CharacterCarriedWeaponRuntimeStateMap;
	// 장착 무기 변경 이벤트
	FOnEquippedWeaponChangedSignature OnEquippedWeaponChanged;
	
};
