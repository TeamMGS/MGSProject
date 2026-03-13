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

	UPROPERTY(Transient)
	TArray<TObjectPtr<ABaseProjectile>> AvailableProjectiles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ABaseProjectile>> AllProjectiles;

	UPROPERTY(Transient)
	bool bHasBeenPrewarmed = false;
};

UCLASS()
class MGSPROJECT_API UMGSProjectilePoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void PrewarmProjectileClass(TSubclassOf<ABaseProjectile> ProjectileClass);

	ABaseProjectile* AcquireProjectile(
		TSubclassOf<ABaseProjectile> ProjectileClass,
		const FTransform& SpawnTransform,
		AActor* NewOwner,
		APawn* NewInstigator);

	void ReturnProjectile(ABaseProjectile* Projectile);

	int32 GetDefaultPoolSize() const { return DefaultPoolSize; }

private:
	void PrewarmPool(UClass* ProjectileClass);
	ABaseProjectile* SpawnPooledProjectile(UClass* ProjectileClass);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, FProjectilePoolBucket> ProjectilePools;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Pool")
	int32 DefaultPoolSize = 500;
	
};
