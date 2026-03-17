/*
 * 파일명 : ProjectilePoolWorldSubsystem.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-10
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolWorldSubsystem.generated.h"

class AActor;
class APawn;
class ABaseProjectile;

USTRUCT()
struct FProjectilePoolBucket
{
	GENERATED_BODY()

	// 대기열 목록
	UPROPERTY(Transient)
	TArray<TObjectPtr<ABaseProjectile>> AvailableProjectiles;

	// 활성, 대기 목록
	UPROPERTY(Transient)
	TArray<TObjectPtr<ABaseProjectile>> AllProjectiles;

	// 예열 확인 플래그 
	UPROPERTY(Transient)
	bool bHasBeenPrewarmed = false;
	
};

UCLASS()
class MGSPROJECT_API UMGSProjectilePoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Getter
	int32 GetDefaultPoolSize() const { return DefaultPoolSize; }

	// Projectile class별 풀을 최초 1회 예열
	void PrewarmProjectileClass(TSubclassOf<ABaseProjectile> ProjectileClass);

	// 대기열에서 Projectile을 꺼내 활성화
	ABaseProjectile* AcquireProjectile(
		TSubclassOf<ABaseProjectile> ProjectileClass,
		const FTransform& SpawnTransform,
		AActor* NewOwner,
		APawn* NewInstigator);

	// Projectile을 비활성화하고 대기 목록으로 반환
	void ReturnProjectile(ABaseProjectile* Projectile);

private:
	// 기본 풀 크기만큼 선생성
	void PrewarmPool(UClass* ProjectileClass);
	// 풀이 고갈됐을 때 배치 단위로 확장
	void ExpandPool(UClass* ProjectileClass, int32 SpawnCount);
	// 실제 Projectile 액터 생성 및 풀 연결
	ABaseProjectile* SpawnPooledProjectile(UClass* ProjectileClass);
	// 현재 총량 기준으로 확장 배치 크기 계산
	int32 CalculateExpansionCount(const FProjectilePoolBucket& PoolBucket) const;

private:
	// 클래스별 Projectile 풀 저장소
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, FProjectilePoolBucket> ProjectilePools;

	// 최초 예열 기본 수량
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Pool", meta = (ClampMin = "1"))
	int32 DefaultPoolSize = 500;

	// 확장 시 최소 배치 크기
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Pool", meta = (ClampMin = "1"))
	int32 MinExpansionBatchSize = 64;

	// 현재 총량 대비 확장 비율
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Pool", meta = (ClampMin = "0.01"))
	float ExpansionRatio = 0.25f;
	
};
