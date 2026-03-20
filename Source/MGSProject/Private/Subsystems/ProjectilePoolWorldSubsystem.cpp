#include "Subsystems/ProjectilePoolWorldSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Projectiles/BaseProjectile.h"

void UMGSProjectilePoolWorldSubsystem::PrewarmProjectileClass(TSubclassOf<ABaseProjectile> ProjectileClass)
{
	UClass* ProjectileClassPtr = ProjectileClass.Get();
	if (!ProjectileClassPtr)
	{
		return;
	}

	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClassPtr);
	if (PoolBucket.bHasBeenPrewarmed)
	{
		return;
	}

	PrewarmPool(ProjectileClassPtr);
	PoolBucket.bHasBeenPrewarmed = true;
}

ABaseProjectile* UMGSProjectilePoolWorldSubsystem::AcquireProjectile(
	TSubclassOf<ABaseProjectile> ProjectileClass,
	const FTransform& SpawnTransform,
	AActor* NewOwner,
	APawn* NewInstigator)
{
	UClass* ProjectileClassPtr = ProjectileClass.Get();
	if (!ProjectileClassPtr)
	{
		return nullptr;
	}

	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClassPtr);
	if (!PoolBucket.bHasBeenPrewarmed)
	{
		return nullptr;
	}

	ABaseProjectile* Projectile = nullptr;

	while (PoolBucket.AvailableProjectiles.Num() > 0 && !Projectile)
	{
		Projectile = PoolBucket.AvailableProjectiles.Pop(EAllowShrinking::No);
		if (!IsValid(Projectile))
		{
			Projectile = nullptr;
		}
	}

	if (!Projectile)
	{
		const int32 ExpansionCount = CalculateExpansionCount(PoolBucket);
		ExpandPool(ProjectileClassPtr, ExpansionCount);

		while (PoolBucket.AvailableProjectiles.Num() > 0 && !Projectile)
		{
			Projectile = PoolBucket.AvailableProjectiles.Pop(EAllowShrinking::No);
			if (!IsValid(Projectile))
			{
				Projectile = nullptr;
			}
		}

		if (!Projectile)
		{
			return nullptr;
		}
	}

	Projectile->ActivateFromPool(SpawnTransform, NewOwner, NewInstigator);
	return Projectile;
}

void UMGSProjectilePoolWorldSubsystem::ReturnProjectile(ABaseProjectile* Projectile)
{
	if (!IsValid(Projectile))
	{
		return;
	}

	UClass* ProjectileClass = Projectile->GetClass();
	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClass);

	Projectile->DeactivateToPool();

	if (!PoolBucket.AllProjectiles.Contains(Projectile))
	{
		PoolBucket.AllProjectiles.Add(Projectile);
	}

	if (!PoolBucket.AvailableProjectiles.Contains(Projectile))
	{
		PoolBucket.AvailableProjectiles.Add(Projectile);
	}
}

void UMGSProjectilePoolWorldSubsystem::PrewarmPool(UClass* ProjectileClass)
{
	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClass);
	const int32 MissingCount = FMath::Max(0, DefaultPoolSize - PoolBucket.AllProjectiles.Num());
	ExpandPool(ProjectileClass, MissingCount);
}

void UMGSProjectilePoolWorldSubsystem::ExpandPool(UClass* ProjectileClass, int32 SpawnCount)
{
	if (!ProjectileClass || SpawnCount <= 0)
	{
		return;
	}

	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClass);

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		ABaseProjectile* Projectile = SpawnPooledProjectile(ProjectileClass);
		if (!Projectile)
		{
			break;
		}

		PoolBucket.AllProjectiles.Add(Projectile);
		PoolBucket.AvailableProjectiles.Add(Projectile);
	}
}

ABaseProjectile* UMGSProjectilePoolWorldSubsystem::SpawnPooledProjectile(UClass* ProjectileClass)
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABaseProjectile* SpawnedProjectile = World->SpawnActor<ABaseProjectile>(
		ProjectileClass,
		FTransform::Identity,
		SpawnParams);
	if (!SpawnedProjectile)
	{
		return nullptr;
	}

	SpawnedProjectile->SetProjectilePoolSubsystem(this);
	SpawnedProjectile->DeactivateToPool();
	return SpawnedProjectile;
}

int32 UMGSProjectilePoolWorldSubsystem::CalculateExpansionCount(const FProjectilePoolBucket& PoolBucket) const
{
	const int32 CurrentTotal = PoolBucket.AllProjectiles.Num();
	const int32 RatioCount = FMath::CeilToInt(static_cast<float>(CurrentTotal) * ExpansionRatio);
	const int32 ExpansionCount = FMath::Max(MinExpansionBatchSize, RatioCount);
	return FMath::Min(ExpansionCount, DefaultPoolSize);
}
