/*
 * 파일명 : DA_WeaponDefinition.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_WeaponDefinition.generated.h"

class UCameraShakeBase;
class ABaseProjectile;
class UGameplayEffect;

UCLASS(BlueprintType)
class MGSPROJECT_API UDA_WeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UDA_WeaponDefinition();

	// 최대 탄약 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	int32 MaxMagazineAmmo = 30;

	// 시작 탄약 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 StartMagazineAmmo = 30;

	// 최대 탄창 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MaxCarriedAmmo = 120;

	// 시작 탄창 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 StartCarriedAmmo = 120;

	// 에임 목표점 계산 기준 거리(스프레드 각도 환산 기준 거리)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "100.0"))
	float AimReferenceDistance = 12000.0f;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.0f;

	// 발사 시 적용할 기본 데미지 GameplayEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	// 연사 간격(초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.12f;

	// 발사할 프로젝타일 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
	TSubclassOf<ABaseProjectile> ProjectileClass;

	// 스프레드 기본 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float BaseSpreadRadius = 0.0f;

	// 스프레드 최대 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float MaxSpreadRadius = 120.0f;

	// 발사 1회당 스프레드 증가량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float SpreadRadiusIncreasePerShot = 6.0f;

	// 조준 시 적용할 카메라 FOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float AimFOV = 65.0f;

	// 조준 시 카메라 소켓 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim")
	FVector AimCameraSocketOffset = FVector(0.0f, 55.0f, 12.0f);

	// 1발당 수직 반동(도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil", meta = (ClampMin = "0.0"))
	float RecoilPitchPerShot = 0.8f;

	// 1발당 수평 반동 최소값(도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	float RecoilYawPerShotMin = -0.25f;

	// 1발당 수평 반동 최대값(도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	float RecoilYawPerShotMax = 0.25f;

	// ADS(Aim Down Sights, 조준) 상태 반동 배율(1.0 미만이면 감소)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil", meta = (ClampMin = "0.0"))
	float RecoilADSScale = 0.7f;

	// 발사 시 재생할 카메라 셰이크 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	TSubclassOf<UCameraShakeBase> FireCameraShakeClass;

	// 카메라 셰이크 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil", meta = (ClampMin = "0.0"))
	float FireCameraShakeScale = 1.0f;
	
};
