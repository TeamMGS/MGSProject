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
	// Projectile class 유효성 검사
	UClass* ProjectileClassPtr = ProjectileClass.Get();
	if (!ProjectileClassPtr)
	{
		return;
	}

	// 클래스별 버킷 조회
	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClassPtr);
	if (PoolBucket.bHasBeenPrewarmed)
	{
		return;
	}

	// 최초 1회만 기본 크기만큼 예열
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

	// 해당 클래스 버킷을 가져오고, 아직 예열되지 않았다면 즉시 예열
	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClassPtr);
	if (!PoolBucket.bHasBeenPrewarmed)
	{
		PrewarmProjectileClass(ProjectileClassPtr);
	}

	ABaseProjectile* Projectile = nullptr;
	bool bReusedFromPool = false;

	// 대기열에서 유효한 Projectile을 꺼냄
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
		// 풀이 바닥났으면 1개씩이 아니라 배치 단위로 확장
		const int32 ExpansionCount = CalculateExpansionCount(PoolBucket);
		ExpandPool(ProjectileClassPtr, ExpansionCount);

		// 확장 직후 다시 대기열에서 하나를 가져옴
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

	// Projectile을 활성 상태로 전환
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

	// Projectile을 비활성 대기 상태로 전환
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
	// 기본 예열 수량에서 현재 총량을 뺀 만큼만 생성
	const int32 MissingCount = FMath::Max(0, DefaultPoolSize - PoolBucket.AllProjectiles.Num());

	UE_LOG(LogProjectilePool, Log, TEXT("[ProjectilePool][PrewarmStart] Class=%s Missing=%d CurrentTotal=%d"),
		*GetNameSafe(ProjectileClass),
		MissingCount,
		PoolBucket.AllProjectiles.Num());

	// 예열도 공용 확장 함수로 처리
	ExpandPool(ProjectileClass, MissingCount);
}

void UMGSProjectilePoolWorldSubsystem::ExpandPool(UClass* ProjectileClass, int32 SpawnCount)
{
	if (!ProjectileClass || SpawnCount <= 0)
	{
		return;
	}

	FProjectilePoolBucket& PoolBucket = ProjectilePools.FindOrAdd(ProjectileClass);
	int32 SpawnedCount = 0;

	// 요청 수량만큼 Projectile을 생성해서 전체/대기 목록에 적재
	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		ABaseProjectile* Projectile = SpawnPooledProjectile(ProjectileClass);
		if (!Projectile)
		{
			break;
		}

		PoolBucket.AllProjectiles.Add(Projectile);
		PoolBucket.AvailableProjectiles.Add(Projectile);
		++SpawnedCount;
	}

	if (SpawnedCount > 0)
	{
		UE_LOG(LogProjectilePool, Warning, TEXT("[ProjectilePool][Expanded] Class=%s Requested=%d Spawned=%d Available=%d Total=%d"),
			*GetNameSafe(ProjectileClass),
			SpawnCount,
			SpawnedCount,
			PoolBucket.AvailableProjectiles.Num(),
			PoolBucket.AllProjectiles.Num());
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

	// Projectile spawn
	ABaseProjectile* SpawnedProjectile = World->SpawnActor<ABaseProjectile>(
		ProjectileClass,
		FTransform::Identity,
		SpawnParams);
	if (!SpawnedProjectile)
	{
		UE_LOG(LogProjectilePool, Error, TEXT("[ProjectilePool][SpawnFailed] Class=%s"), *GetNameSafe(ProjectileClass));
		return nullptr;
	}

	// 풀 복귀를 위해 Subsystem을 연결하고, 생성 직후에는 대기 상태로 둠
	SpawnedProjectile->SetProjectilePoolSubsystem(this);
	SpawnedProjectile->DeactivateToPool();
	
	return SpawnedProjectile;
}

int32 UMGSProjectilePoolWorldSubsystem::CalculateExpansionCount(const FProjectilePoolBucket& PoolBucket) const
{
	// 현재 총량의 일정 비율만큼 확장하되 최소 배치 크기는 보장
	const int32 CurrentTotal = PoolBucket.AllProjectiles.Num();
	const int32 RatioCount = FMath::CeilToInt(static_cast<float>(CurrentTotal) * ExpansionRatio);
	return FMath::Max(MinExpansionBatchSize, RatioCount);
}
