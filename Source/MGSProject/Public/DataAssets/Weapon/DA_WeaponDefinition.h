/*
 * 파일명: DA_WeaponDefinition.h
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자: 장대한
 * 수정일: 2026-03-04
 */

#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_WeaponDefinition.generated.h"

UCLASS(BlueprintType)
class MGSPROJECT_API UDA_WeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 탄창 최대 탄약 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	int32 MaxMagazineAmmo = 30;

	// 시작 시 탄창 탄약 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 StartMagazineAmmo = 30;

	// 예비 탄약 최대치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MaxCarriedAmmo = 120;

	// 시작 시 예비 탄약 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 StartCarriedAmmo = 120;

	// 히트스캔 최대 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "100.0"))
	float FireRange = 12000.f;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;

	// 연사 간격(초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.12f;

	// 탄착군 기본 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float BaseSpreadRadius = 0.f;

	// 탄착군 최대 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float MaxSpreadRadius = 120.f;

	// 발사 1회당 탄착군 증가량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire|Spread", meta = (ClampMin = "0.0"))
	float SpreadRadiusIncreasePerShot = 6.f;

	// 조준 시 적용할 카메라 FOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float AimFOV = 65.f;

	// 조준 시 카메라 소켓 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim")
	FVector AimCameraSocketOffset = FVector(0.f, 55.f, 12.f);
};


