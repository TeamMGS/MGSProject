/*
 * 파일명: DA_ProjectileDefinition.h
 * 생성자: 장대한
 * 생성일: 2026-03-06
 * 수정자: 장대한
 * 수정일: 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_ProjectileDefinition.generated.h"

class UDamageType;

UCLASS(BlueprintType)
class MGSPROJECT_API UDA_ProjectileDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Collision 반지름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Collision", meta = (ClampMin = "0.0"))
	float CollisionRadius = 6.f;

	// DamageType 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	// 초기 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Move", meta = (ClampMin = "0.0"))
	float InitialSpeed = 4500.f;

	// 최고 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Move", meta = (ClampMin = "0.0"))
	float MaxSpeed = 4500.f;

	// 날아가는 방향으로 회전 플래그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Move")
	bool bRotationFollowsVelocity = true;

	// 중력 스케일
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Move")
	float ProjectileGravityScale = 0.f;

	// 생존 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float ProjectileLifeSpan = 5.f;

	// 충돌 후 제거 플래그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bDestroyOnHit = true;

	// Owner/Instigator 충돌 판정 무시 플래그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bIgnoreOwnerOnHit = true;
};
