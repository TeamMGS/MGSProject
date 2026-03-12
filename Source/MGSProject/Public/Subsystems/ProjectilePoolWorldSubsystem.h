/*
 * 파일명 : ProjectilePoolWorldSubsystem.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-10
 * 수정자 : 장대한 
 * 수정일 : 2026-03-10
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolWorldSubsystem.generated.h"

class AActor;
class APawn;
class ABaseProjectile;

UCLASS()
class MGSPROJECT_API UMGSProjectilePoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ABaseProjectile* AcquireProjectile(
		TSubclassOf<ABaseProjectile> ProjectileClass,
		const FTransform& SpawnTransform,
		AActor* NewOwner,
		APawn* NewInstigator);

	void ReturnProjectile(ABaseProjectile* Projectile);

	int32 GetDefaultPoolSize() const { return DefaultPoolSize; }

private:
	struct FProjectilePoolBucket
	{
		TArray<TObjectPtr<ABaseProjectile>> AvailableProjectiles;
		TArray<TObjectPtr<ABaseProjectile>> AllProjectiles;
		bool bHasBeenPrewarmed = false;
	};

	void PrewarmPool(UClass* ProjectileClass);
	ABaseProjectile* SpawnPooledProjectile(UClass* ProjectileClass);

private:
	TMap<UClass*, FProjectilePoolBucket> ProjectilePools;

	int32 DefaultPoolSize = 500;
};
