/*
 * 파일명: BaseGun.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#pragma once


#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "BaseGun.generated.h"

class UDA_WeaponDefinition;
class UWeaponAttributeSet;
struct FWeaponRuntimeState;

UCLASS()
class MGSPROJECT_API ABaseGun : public ABaseWeapon
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun|Ammo")
	bool ConsumeAmmo(int32 AmmoToConsume = 1);

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun|Ammo")
	int32 ReloadAmmo();

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetCurrentMagazineAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetMaxMagazineAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetCarriedAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetFireRange() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetBaseDamage() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetFireInterval() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetBaseSpreadRadius() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetMaxSpreadRadius() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetSpreadRadiusIncreasePerShot() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Aim")
	float GetAimFOV() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Aim")
	FVector GetAimCameraSocketOffset() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Data")
	const UDA_WeaponDefinition* GetWeaponDefinition() const { return WeaponDefinition; }

	// 장착 시 DA 값을 WeaponAttributeSet에 반영합니다.
	bool InitializeWeaponAttributes(UWeaponAttributeSet* WeaponAttributeSet) const;

	// 무기 전환 시 저장한 런타임 탄약 상태를 WeaponAttributeSet에 복원합니다.
	void ApplyRuntimeState(UWeaponAttributeSet* WeaponAttributeSet, const FWeaponRuntimeState& RuntimeState) const;

	// 현재 WeaponAttributeSet 값을 런타임 상태로 추출합니다.
	FWeaponRuntimeState MakeRuntimeState(const UWeaponAttributeSet* WeaponAttributeSet) const;

	// DA(또는 폴백 값) 기준 기본 런타임 상태를 생성합니다.
	FWeaponRuntimeState MakeDefaultRuntimeState() const;

protected:
	virtual void BeginPlay() override;

private:
	const UWeaponAttributeSet* GetWeaponAttributeSet() const;
	UWeaponAttributeSet* GetWeaponAttributeSetMutable() const;

	int32 GetDefinitionMaxMagazineAmmo() const;
	int32 GetDefinitionStartMagazineAmmo() const;
	int32 GetDefinitionMaxCarriedAmmo() const;
	int32 GetDefinitionStartCarriedAmmo() const;
	float GetDefinitionFireRange() const;
	float GetDefinitionBaseDamage() const;
	float GetDefinitionFireInterval() const;
	float GetDefinitionBaseSpreadRadius() const;
	float GetDefinitionMaxSpreadRadius() const;
	float GetDefinitionSpreadRadiusIncreasePerShot() const;
	float GetDefinitionAimFOV() const;
	FVector GetDefinitionAimCameraSocketOffset() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UDA_WeaponDefinition> WeaponDefinition;

	// 폴백 탄창 최대 탄약 수(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Ammo", meta = (ClampMin = "1"))
	int32 MaxMagazineAmmo = 30;

	// 폴백 시작 탄창 탄약 수(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Gun|Ammo", meta = (ClampMin = "0"))
	int32 CurrentMagazineAmmo = 30;

	// 폴백 예비 탄약 수(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Gun|Ammo", meta = (ClampMin = "0"))
	int32 CarriedAmmo = 120;

	// 폴백 발사 최대 사거리(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire", meta = (ClampMin = "100.0"))
	float FireRange = 12000.f;

	// 폴백 기본 데미지(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;

	// 폴백 연사 간격(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.12f;

	// 폴백 탄착군 기본 반경(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire|Spread", meta = (ClampMin = "0.0"))
	float BaseSpreadRadius = 0.f;

	// 폴백 탄착군 최대 반경(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire|Spread", meta = (ClampMin = "0.0"))
	float MaxSpreadRadius = 120.f;

	// 폴백 발사 1회당 탄착군 증가량(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Fire|Spread", meta = (ClampMin = "0.0"))
	float SpreadRadiusIncreasePerShot = 6.f;

	// 폴백 에임 FOV(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Aim", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float AimFOV = 65.f;

	// 폴백 에임 카메라 오프셋(WeaponDefinition 미할당 시 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Aim")
	FVector AimCameraSocketOffset = FVector(0.f, 55.f, 12.f);
};


