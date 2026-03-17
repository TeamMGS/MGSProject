/*
 * 파일명 : BaseGun.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "BaseGun.generated.h"

class UDA_WeaponDefinition;
class UCameraShakeBase;
class UWeaponAttributeSet;
class ABaseProjectile;
class UGameplayEffect;
struct FWeaponRuntimeState;

UCLASS()
class MGSPROJECT_API ABaseGun : public ABaseWeapon
{
	GENERATED_BODY()

public:
	// --- Ammo ---
	// 발사 가능 조회
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	bool CanFire() const;

	// 탄약 소모
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun|Ammo")
	bool ConsumeAmmo(int32 AmmoToConsume = 1);

	// 탄약 환급(롤백)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun|Ammo")
	bool RefundAmmo(int32 AmmoToRefund = 1);

	// 장전 가능 조회
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	bool CanReload() const;

	// 장전
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun|Ammo")
	int32 ReloadAmmo();

	// 현재 탄약
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetCurrentMagazineAmmo() const;

	// 최대 탄약
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetMaxMagazineAmmo() const;

	// 현재 휴대 탄약
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Ammo")
	int32 GetCarriedAmmo() const;

	// --- Fire ---
	// 에임 목표점 계산 기준 거리
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetAimReferenceDistance() const;

	// 기본 데미지
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetBaseDamage() const;

	// 발사 데미지 GameplayEffect
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	TSubclassOf<UGameplayEffect> GetDamageGameplayEffectClass() const;

	// 연사 간격
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetFireInterval() const;

	// 기본 탄착군
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetBaseSpreadRadius() const;

	// 최대 탄착군
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetMaxSpreadRadius() const;

	// 탄착군 증가율
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	float GetSpreadRadiusIncreasePerShot() const;

	// --- Aim ---
	// 조준 줌 FOV
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Aim")
	float GetAimFOV() const;

	// 조준 줌 카메라 오프셋
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Aim")
	FVector GetAimCameraSocketOffset() const;

	// --- Recoil ---
	// 1발당 수직 반동(도)
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	float GetRecoilPitchPerShot() const;

	// 1발당 수평 반동 최소값(도)
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	float GetRecoilYawPerShotMin() const;

	// 1발당 수평 반동 최대값(도)
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	float GetRecoilYawPerShotMax() const;

	// ADS 상태 반동 배율
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	float GetRecoilADSScale() const;

	// 발사 카메라 쉐이크 클래스
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	TSubclassOf<UCameraShakeBase> GetFireCameraShakeClass() const;

	// 카메라 쉐이크 강도
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Recoil")
	float GetFireCameraShakeScale() const;

	// --- Projectile ---
	// 발사할 프로젝타일 클래스
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Fire")
	TSubclassOf<ABaseProjectile> GetProjectileClass() const;

	// --- Data ---
	UFUNCTION(BlueprintPure, Category = "Weapon|Gun|Data")
	const UDA_WeaponDefinition* GetWeaponDefinition() const { return WeaponDefinition; }

	// --- Runtime state ---
	// 장착 시 DA 값을 WeaponAttributeSet에 반영합니다.
	bool InitializeWeaponAttributes(UWeaponAttributeSet* WeaponAttributeSet) const;

	// 무기 전환 시 저장한 런타임 탄약 상태를 WeaponAttributeSet에 복원합니다.
	void ApplyRuntimeState(UWeaponAttributeSet* WeaponAttributeSet, const FWeaponRuntimeState& RuntimeState) const;

	// 현재 WeaponAttributeSet 값을 런타임 상태로 추출합니다.
	FWeaponRuntimeState MakeRuntimeState(const UWeaponAttributeSet* WeaponAttributeSet) const;

	// DA 기준 기본 런타임 상태를 생성합니다.
	FWeaponRuntimeState MakeDefaultRuntimeState() const;

protected:
	virtual void BeginPlay() override;

	// DA_WeaponDefinition
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UDA_WeaponDefinition> WeaponDefinition;

private:
	const UDA_WeaponDefinition& GetWeaponDefinitionChecked() const;

	// Owner의 WeaponAttributeSet 조회
	const UWeaponAttributeSet* GetWeaponAttributeSet() const;
	UWeaponAttributeSet* GetWeaponAttributeSetMutable() const;
	
};
