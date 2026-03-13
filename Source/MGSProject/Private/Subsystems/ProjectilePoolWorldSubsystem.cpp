/*
 * 파일명 : ProjectilePoolWorldSubsystem.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-10
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Subsystems/ProjectilePoolWorldSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Projectiles/BaseProjectile.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectilePool, Log, All);

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
	UE_LOG(LogProjectilePool, Log, TEXT("[ProjectilePool][PrewarmComplete] Class=%s Available=%d Total=%d"),
		*GetNameSafe(ProjectileClassPtr),
		PoolBucket.AvailableProjectiles.Num(),
		PoolBucket.AllProjectiles.Num());
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
		PrewarmProjectileClass(ProjectileClassPtr);
	}

	ABaseProjectile* Projectile = nullptr;
	bool bReusedFromPool = false;
	while (PoolBucket.AvailableProjectiles.Num() > 0 && !Projectile)
	{
		Projectile = PoolBucket.AvailableProjectiles.Pop(EAllowShrinking::No);
		if (!IsValid(Projectile))
		{
			Projectile = nullptr;
		}
		else
		{
			bReusedFromPool = true;
		}
	}

	if (!Projectile)
	{
		Projectile = SpawnPooledProjectile(ProjectileClassPtr);
		if (!Projectile)
		{
			return nullptr;
		}

		PoolBucket.AllProjectiles.Add(Projectile);
		UE_LOG(LogProjectilePool, Warning, TEXT("[ProjectilePool][Expanded] Class=%s Available=%d Total=%d"),
			*GetNameSafe(ProjectileClassPtr),
			PoolBucket.AvailableProjectiles.Num(),
			PoolBucket.AllProjectiles.Num());
	}

	Projectile->ActivateFromPool(SpawnTransform, NewOwner, NewInstigator);
	UE_LOG(LogProjectilePool, Log, TEXT("[ProjectilePool][Acquire] Class=%s Projectile=%s Source=%s Available=%d Total=%d"),
		*GetNameSafe(ProjectileClassPtr),
		*GetNameSafe(Projectile),
		bReusedFromPool ? TEXT("Pool") : TEXT("Expanded"),
		PoolBucket.AvailableProjectiles.Num(),
		PoolBucket.AllProjectiles.Num());
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

	UE_LOG(LogProjectilePool, Log, TEXT("[ProjectilePool][Return] Class=%s Projectile=%s Available=%d Total=%d"),
		*GetNameSafe(ProjectileClass),
		*GetNameSafe(Projectile),
		PoolBucket.AvailableProjectiles.Num(),
		PoolBucket.AllProjectiles.Num());
}

void UMGSProjectilePoolWorldSubsystem::PrewarmPool(UClass* ProjectileClass)
{
	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClass);
	const int32 MissingCount = FMath::Max(0, DefaultPoolSize - PoolBucket.AllProjectiles.Num());
	UE_LOG(LogProjectilePool, Log, TEXT("[ProjectilePool][PrewarmStart] Class=%s Missing=%d CurrentTotal=%d"),
		*GetNameSafe(ProjectileClass),
		MissingCount,
		PoolBucket.AllProjectiles.Num());

	for (int32 Index = 0; Index < MissingCount; ++Index)
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
		UE_LOG(LogProjectilePool, Error, TEXT("[ProjectilePool][SpawnFailed] Class=%s"), *GetNameSafe(ProjectileClass));
		return nullptr;
	}

	SpawnedProjectile->SetProjectilePoolSubsystem(this);
	SpawnedProjectile->DeactivateToPool();
	return SpawnedProjectile;
}
