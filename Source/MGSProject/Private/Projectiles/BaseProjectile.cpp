/*
 * ?뚯씪紐? BaseProjectile.cpp
 * ?앹꽦?? ?λ??? * ?앹꽦?? 2026-03-06
 * ?섏젙?? ?λ??? * ?섏젙?? 2026-03-06
 */

#include "Projectiles/BaseProjectile.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/Projectile/DA_ProjectileDefinition.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float DefaultCollisionRadius = 6.f;
	constexpr float DefaultVisualScale = 0.06f;
	constexpr float DefaultInitialSpeed = 4500.f;
	constexpr float DefaultMaxSpeed = 4500.f;
	constexpr float DefaultLifeSpan = 5.f;
	constexpr bool DefaultDestroyOnHit = true;
	constexpr bool DefaultIgnoreOwnerOnHit = true;
	constexpr bool DefaultRotationFollowsVelocity = true;
	constexpr float DefaultProjectileGravityScale = 0.f;
}

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 肄쒕━???ㅼ젙
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(DefaultCollisionRadius);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetNotifyRigidBodyCollision(false);
	CollisionComponent->SetCanEverAffectNavigation(false);

	// 硫붿떆 而댄룷?뚰듃 ?ㅼ젙
	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
	ProjectileMeshComponent->SetupAttachment(CollisionComponent);
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMeshComponent->SetCanEverAffectNavigation(false);
	ProjectileMeshComponent->SetRelativeScale3D(FVector(DefaultVisualScale));

	// 湲곕낯 ?쒓컖?붾? ?꾪빐 ?붿쭊 湲곕낯 Sphere 硫붿돩瑜??좊떦(?꾩슂 ??BP?먯꽌 援먯껜)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultProjectileMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultProjectileMesh.Succeeded())
	{
		ProjectileMeshComponent->SetStaticMesh(DefaultProjectileMesh.Object);
	}

	// 臾대툕癒쇳듃 而댄룷?뚰듃 ?ㅼ젙
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = DefaultInitialSpeed;
	ProjectileMovementComponent->MaxSpeed = DefaultMaxSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = DefaultRotationFollowsVelocity;
	ProjectileMovementComponent->ProjectileGravityScale = DefaultProjectileGravityScale;

	// ?곕?吏 ????대옒???ㅼ젙
	CurrentDamageTypeClass = UDamageType::StaticClass();
	CurrentDamage = 0.f;
	bShouldDestroyOnHit = DefaultDestroyOnHit;
	bShouldIgnoreOwnerOnHit = DefaultIgnoreOwnerOnHit;
	InitialLifeSpan = DefaultLifeSpan;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// ?곸옱??DA瑜?湲곗??쇰줈 媛??ㅼ젙
	ApplyProjectileDefinition();

	// 諛쒖궗泥?異⑸룎? CollisionComponent濡쒕쭔 泥섎━?⑸땲??
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
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);

		// 肄쒕━??Overlap ?대깽?몄뿉 ?몃뱾???곸슜
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleProjectileOverlap);

		// 肄쒕━?꾩뿉 Owner, Instigator 臾댁떆 ?ㅼ젙
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
}

void ABaseProjectile::InitializeProjectile(const FVector& ShootDirection)
{
	if (!bHasAppliedDefinition)
	{
		ApplyProjectileDefinition();
	}

	if (!ProjectileMovementComponent)
	{
		return;
	}

	FVector SafeDirection = ShootDirection.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		SafeDirection = GetActorForwardVector();
	}

	ProjectileMovementComponent->Velocity = SafeDirection * ProjectileMovementComponent->InitialSpeed;
	SetActorRotation(SafeDirection.Rotation());
}

void ABaseProjectile::SetProjectileDamage(float InDamage)
{
	CurrentDamage = FMath::Max(0.f, InDamage);
}

void ABaseProjectile::HandleProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherBodyIndex;

	if (bShouldDestroyOnHit && bHasProcessedImpact)
	{
		return;
	}

	if (!OtherActor || OtherActor == this || !OtherComp)
	{
		return;
	}

	if (bShouldIgnoreOwnerOnHit && (OtherActor == GetOwner() || OtherActor == GetInstigator()))
	{
		return;
	}

	const FHitResult EffectiveHit = bFromSweep ? SweepResult : FHitResult();
	bHasProcessedImpact = true;

	// ?곕?吏 ?곸슜
	ApplyHitDamage(OtherActor, EffectiveHit);
	// 異⑸룎 ?대깽??	OnProjectileImpact(EffectiveHit);

	if (bShouldDestroyOnHit)
	{
		Destroy();
	}
}

void ABaseProjectile::ApplyHitDamage(AActor* DirectHitActor, const FHitResult& Hit)
{
	AActor* HitActor = DirectHitActor ? DirectHitActor : Hit.GetActor();
	if (!HitActor || CurrentDamage <= 0.f)
	{
		return;
	}

	AController* InstigatorController = nullptr;
	if (const APawn* InstigatorPawn = GetInstigator())
	{
		InstigatorController = InstigatorPawn->GetController();
	}

	AActor* DamageCauser = GetOwner() ? GetOwner() : this;
	const FVector ShotDirection = ProjectileMovementComponent
		? ProjectileMovementComponent->Velocity.GetSafeNormal()
		: GetActorForwardVector();
	const TSubclassOf<UDamageType> ResolvedDamageTypeClass = CurrentDamageTypeClass
		? CurrentDamageTypeClass
		: TSubclassOf<UDamageType>(UDamageType::StaticClass());

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		CurrentDamage,
		ShotDirection,
		Hit,
		InstigatorController,
		DamageCauser,
		ResolvedDamageTypeClass);
}

void ABaseProjectile::ApplyProjectileDefinition()
{
	if (bHasAppliedDefinition)
	{
		return;
	}

	const UDA_ProjectileDefinition* Definition = ProjectileDefinition.Get();
	if (!Definition)
	{
		ensureMsgf(false, TEXT("%s has no ProjectileDefinition. Using fallback defaults."), *GetName());
	}

	const float CollisionRadius = Definition ? FMath::Max(0.f, Definition->CollisionRadius) : DefaultCollisionRadius;
	const float DefinitionInitialSpeed = Definition ? FMath::Max(0.f, Definition->InitialSpeed) : DefaultInitialSpeed;
	const float DefinitionMaxSpeed = Definition ? FMath::Max(DefinitionInitialSpeed, Definition->MaxSpeed) : DefaultMaxSpeed;
	const float DefinitionLifeSpan = Definition ? FMath::Max(0.f, Definition->ProjectileLifeSpan) : DefaultLifeSpan;
	const bool bDefinitionDestroyOnHit = Definition ? Definition->bDestroyOnHit : DefaultDestroyOnHit;
	const bool bDefinitionIgnoreOwnerOnHit = Definition ? Definition->bIgnoreOwnerOnHit : DefaultIgnoreOwnerOnHit;
	const bool bDefinitionRotationFollowsVelocity = Definition ? Definition->bRotationFollowsVelocity : DefaultRotationFollowsVelocity;
	const float DefinitionProjectileGravityScale = Definition
		? Definition->ProjectileGravityScale
		: DefaultProjectileGravityScale;

	CurrentDamageTypeClass = (Definition && Definition->DamageTypeClass)
		? Definition->DamageTypeClass
		: TSubclassOf<UDamageType>(UDamageType::StaticClass());
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

	InitialLifeSpan = DefinitionLifeSpan;
	SetLifeSpan(DefinitionLifeSpan);
	bHasAppliedDefinition = true;
}

void ABaseProjectile::OnProjectileImpact_Implementation(const FHitResult& HitResult)
{
}

