/*
 * 파일명 : BaseProjectile.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-06
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#include "Projectiles/BaseProjectile.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/Projectile/DA_ProjectileDefinition.h"
#include "GAS/Statics/MGSDamageStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Subsystems/ProjectilePoolWorldSubsystem.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ABaseProjectile::ABaseProjectile()
{
	// 틱은 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;

	// 콜리전 컴포넌트 설정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(6.0f);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetNotifyRigidBodyCollision(false);
	CollisionComponent->SetCanEverAffectNavigation(false);

	// 메시 컴포넌트 설정
	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
	ProjectileMeshComponent->SetupAttachment(CollisionComponent);
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMeshComponent->SetCanEverAffectNavigation(false);
	ProjectileMeshComponent->SetRelativeScale3D(FVector(0.06f));

	// 기본 시각화를 위한 기본 Sphere 메시 할당(필요 시 BP에서 교체)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultProjectileMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultProjectileMesh.Succeeded())
	{
		ProjectileMeshComponent->SetStaticMesh(DefaultProjectileMesh.Object);
	}

	// 무브먼트 컴포넌트 설정
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = 4500.0f;
	ProjectileMovementComponent->MaxSpeed = 4500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bSweepCollision = true;
	ProjectileMovementComponent->bForceSubStepping = true;

	// 변수 초기화
	// 히트 시 파괴 플래그
	bShouldDestroyOnHit = true;
	// 소유자 무시 플래그
	bShouldIgnoreOwnerOnHit = true;
	// 생존 시간
	InitialLifeSpan = 5.0f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	checkf(ProjectileDefinition, TEXT("%s has no ProjectileDefinition assigned."), *GetName());

	// 데이터 에셋 설정 적용
	ApplyProjectileDefinition();

	// 충돌 처리는 CollisionComponent로만 수행
	if (ProjectileMeshComponent)
	{
		ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	if (ensureMsgf(CollisionComponent, TEXT("%s has no collision component."), *GetName()))
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetSimulatePhysics(false);
		CollisionComponent->SetGenerateOverlapEvents(true);
		CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);

		// Overlap 이벤트 바인딩
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleProjectileOverlap);

		// 피격 시 Owner/Instigator 무시
		if (bShouldIgnoreOwnerOnHit)
		{
			if (AActor* OwnerActor = GetOwner())
			{
				CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
			}

			if (APawn* InstigatorPawn = GetInstigator())
			{
				CollisionComponent->IgnoreActorWhenMoving(InstigatorPawn, true);
			}
		}
	}

	if (ensureMsgf(ProjectileMovementComponent, TEXT("%s has no projectile movement component."), *GetName()))
	{
		// Projectile 정지 이벤트 바인딩
		ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &ThisClass::HandleProjectileStop);
	}
}

void ABaseProjectile::SetProjectilePoolSubsystem(UMGSProjectilePoolWorldSubsystem* InProjectilePoolSubsystem)
{
	ProjectilePoolSubsystem = InProjectilePoolSubsystem;
}

void ABaseProjectile::SetAttackPayload(const FMGSProjectileAttackPayload& InAttackPayload)
{
	AttackPayload = InAttackPayload;
}

void ABaseProjectile::ActivateFromPool(const FTransform& SpawnTransform, AActor* NewOwner, APawn* NewInstigator)
{
	// Owner, Instigator, 위치 설정
	SetOwner(IsValid(NewOwner) ? NewOwner : nullptr);
	SetInstigator(IsValid(NewInstigator) ? NewInstigator : nullptr);
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// 이번 발사에서 충돌 했는지 플래그 초기화
	bHasProcessedImpact = false;
	// 현재 실제로 발사되어 활동중인지 플래그 초기화
	bIsActiveInPool = false;

	// 콜리전 설정
	if (CollisionComponent)
	{
		CollisionComponent->ClearMoveIgnoreActors();
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetGenerateOverlapEvents(true);
	}

	// 무브먼트 설정
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		ProjectileMovementComponent->UpdateComponentVelocity();
		ProjectileMovementComponent->Activate(true);
	}

	// 게임에서 숨김
	SetActorHiddenInGame(false);
	// 콜리전 활성화
	SetActorEnableCollision(true);
}

void ABaseProjectile::DeactivateToPool()
{
	if (UWorld* World = GetWorld())
	{
		// 생존 타이머 해제
		World->GetTimerManager().ClearTimer(ProjectileLifeSpanTimerHandle);
	}

	bHasProcessedImpact = false;
	bIsActiveInPool = false;
	AttackPayload.Reset();

	// 이동 정지 및 비활성화
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		ProjectileMovementComponent->UpdateComponentVelocity();
		ProjectileMovementComponent->Deactivate();
	}

	// 충돌 비활성화 및 무기 목록 제거
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComponent->SetGenerateOverlapEvents(false);
		CollisionComponent->ClearMoveIgnoreActors();
	}

	// Collision 비활성화
	SetActorEnableCollision(false);
	// 게임에서 숨기기
	SetActorHiddenInGame(true);
	// Owner/Instigator clear
	SetOwner(nullptr);
	SetInstigator(nullptr);
}

void ABaseProjectile::InitializeProjectile(const FVector& ShootDirection)
{
	if (!bHasAppliedDefinition)
	{
		// DA_ProjectileDefinition에 설정된 값으로 설정
		ApplyProjectileDefinition();
	}

	if (!ProjectileMovementComponent)
	{
		return;
	}
	
	// Movement component 설정
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	
	// Direction 설정
	FVector SafeDirection = ShootDirection.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		SafeDirection = GetActorForwardVector();
	}

	// Velocity 설정
	ProjectileMovementComponent->Velocity = SafeDirection * ProjectileMovementComponent->InitialSpeed;
	ProjectileMovementComponent->UpdateComponentVelocity();
	// Rotation 설정
	SetActorRotation(SafeDirection.Rotation());
	// 총알 활동 플래그 참으로 설정
	bIsActiveInPool = true;
	// 수명 타이머 시작
	StartProjectileLifeSpanTimer();
}

void ABaseProjectile::HandleProjectileStop(const FHitResult& ImpactResult)
{
	if (!bIsActiveInPool)
	{
		return;
	}

	// Projectile 충돌 처리
	ProcessProjectileImpact(ImpactResult.GetActor(), ImpactResult);
}

void ABaseProjectile::HandleProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	if (!bIsActiveInPool)
	{
		return;
	}

	// Projectile 충돌 처리
	const FHitResult EffectiveHit = bFromSweep ? SweepResult : FHitResult();
	ProcessProjectileImpact(OtherActor, EffectiveHit);
}

void ABaseProjectile::ProcessProjectileImpact(AActor* HitActor, const FHitResult& Hit)
{
	if (!bIsActiveInPool)
	{
		return;
	}

	if (bShouldDestroyOnHit && bHasProcessedImpact)
	{
		return;
	}

	AActor* EffectiveHitActor = HitActor ? HitActor : Hit.GetActor();
	if (EffectiveHitActor == this)
	{
		return;
	}

	if (bShouldIgnoreOwnerOnHit && EffectiveHitActor && (EffectiveHitActor == GetOwner() || EffectiveHitActor == GetInstigator()))
	{
		return;
	}

	if (bShouldDestroyOnHit)
	{
		bHasProcessedImpact = true;
	}

	// 피격 데미지 적용
	ApplyHitDamage(EffectiveHitActor, Hit);

	if (bShouldDestroyOnHit)
	{
		ReleaseToPoolOrDestroy();
	}
}

void ABaseProjectile::ApplyHitDamage(AActor* DirectHitActor, const FHitResult& Hit)
{
	AActor* HitActor = DirectHitActor ? DirectHitActor : Hit.GetActor();
	if (!HitActor || !AttackPayload.HasDamageData())
	{
		return;
	}

	// 실제 데미지 치러
	FMGSDamageStatics::ApplyProjectileDamage(AttackPayload, this, GetInstigator(), HitActor, Hit);
}

void ABaseProjectile::StartProjectileLifeSpanTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 생존 타이머 시작
	World->GetTimerManager().ClearTimer(ProjectileLifeSpanTimerHandle);
	if (ActiveProjectileLifeSpan > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			ProjectileLifeSpanTimerHandle,
			this,
			&ThisClass::HandleLifeSpanExpired,
			ActiveProjectileLifeSpan,
			false);
	}
}

void ABaseProjectile::HandleLifeSpanExpired()
{
	if (!bIsActiveInPool)
	{
		return;
	}

	ReleaseToPoolOrDestroy();
}

void ABaseProjectile::ReleaseToPoolOrDestroy()
{
	// 풀링 시스템이 유효하면
	if (ProjectilePoolSubsystem.IsValid())
	{
		ProjectilePoolSubsystem->ReturnProjectile(this);
		return;
	}

	// 풀링 시스템이 유효하지 않으면 제거
	Destroy();
}

void ABaseProjectile::ApplyProjectileDefinition()
{
	if (bHasAppliedDefinition)
	{
		return;
	}

	const UDA_ProjectileDefinition& Definition = GetProjectileDefinitionChecked();

	const float CollisionRadius = FMath::Max(0.f, Definition.CollisionRadius);
	const float DefinitionInitialSpeed = FMath::Max(0.f, Definition.InitialSpeed);
	const float DefinitionMaxSpeed = FMath::Max(DefinitionInitialSpeed, Definition.MaxSpeed);
	const float DefinitionLifeSpan = FMath::Max(0.f, Definition.ProjectileLifeSpan);
	const bool bDefinitionDestroyOnHit = Definition.bDestroyOnHit;
	const bool bDefinitionIgnoreOwnerOnHit = Definition.bIgnoreOwnerOnHit;
	const bool bDefinitionRotationFollowsVelocity = Definition.bRotationFollowsVelocity;
	const float DefinitionProjectileGravityScale = Definition.ProjectileGravityScale;

	bShouldDestroyOnHit = bDefinitionDestroyOnHit;
	bShouldIgnoreOwnerOnHit = bDefinitionIgnoreOwnerOnHit;

	if (CollisionComponent)
	{
		CollisionComponent->SetSphereRadius(CollisionRadius);
	}

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = DefinitionInitialSpeed;
		ProjectileMovementComponent->MaxSpeed = DefinitionMaxSpeed;
		ProjectileMovementComponent->bRotationFollowsVelocity = bDefinitionRotationFollowsVelocity;
		ProjectileMovementComponent->ProjectileGravityScale = DefinitionProjectileGravityScale;
	}

	ActiveProjectileLifeSpan = DefinitionLifeSpan;
	InitialLifeSpan = 0.0f;
	// DA_ProjectileDefinition 플래그 참처리
	bHasAppliedDefinition = true;
}

const UDA_ProjectileDefinition& ABaseProjectile::GetProjectileDefinitionChecked() const
{
	checkf(ProjectileDefinition, TEXT("%s has no ProjectileDefinition assigned."), *GetName());
	
	return *ProjectileDefinition;
}
