/*
 * 파일명 : PlayerCombatComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-10
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "PlayerCombatComponent.generated.h"

class ABaseWeapon;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNearbyDroppedWeaponChangedSignature, const ABaseWeapon* /*NearbyDroppedWeapon*/);

UCLASS()
class MGSPROJECT_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool EquipSecondaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PickupDroppedWeapon(ABaseWeapon* DroppedWeapon);

	void NotifyDroppedWeaponOverlapBegin(ABaseWeapon* DroppedWeapon);
	void NotifyDroppedWeaponOverlapEnd(ABaseWeapon* DroppedWeapon);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasNearbyDroppedWeapon() const;

	ABaseWeapon* GetNearbyDroppedWeapon() const;
	void RefreshNearbyDroppedWeaponCandidate();

	FOnNearbyDroppedWeaponChangedSignature& GetOnNearbyDroppedWeaponChangedDelegate()
	{
		return OnNearbyDroppedWeaponChanged;
	}

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetPrimaryWeaponTag() const { return PrimaryWeaponTag; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetSecondaryWeaponTag() const { return SecondaryWeaponTag; }

private:
	// 1번 키 장착 대상(주무기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag PrimaryWeaponTag;

	// 2번 키 장착 대상(보조무기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Loadout", meta = (AllowPrivateAccess = "true", Categories = "Weapon"))
	FGameplayTag SecondaryWeaponTag;

	void BroadcastNearbyDroppedWeaponChangedIfNeeded();
	ABaseWeapon* ResolveBestNearbyDroppedWeapon() const;
	ABaseWeapon* FindNearbyDroppedWeaponBySweep() const;

	TWeakObjectPtr<ABaseWeapon> LastNearbyDroppedWeapon;
	TSet<TWeakObjectPtr<ABaseWeapon>> NearbyDroppedWeapons;
	FOnNearbyDroppedWeaponChangedSignature OnNearbyDroppedWeaponChanged;

	static constexpr float PickupSweepForwardOffset = 60.0f;
	static constexpr float PickupSweepRadius = 90.0f;
	
};
