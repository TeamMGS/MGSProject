/*
 * 파일명 : BaseProjectile.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-06
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "Projectiles/BaseProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/Projectile/DA_ProjectileDefinition.h"
#include "GAS/GE/MGSDamageGameplayEffect.h"
#include "GAS/MGSGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Weapon/BaseGun.h"

ABaseProjectile::ABaseProjectile()
{
	// 틱은 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 콜리전 컴포넌트 설정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(6.0f);
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

	// 변수 초기화
	CurrentDamageGameplayEffectClass = UMGSDamageGameplayEffect::StaticClass(); // 데미지 GE 클래스
	CachedWeaponDamage = 0.0f; // 발사 시점 데미지
	bShouldDestroyOnHit = true; // 히트 시 파괴 플래그
	bShouldIgnoreOwnerOnHit = true; // 소유자 무시 플래그 
	InitialLifeSpan = 5.0f; // 생존 시간
}

const UDA_ProjectileDefinition& ABaseProjectile::GetProjectileDefinitionChecked() const
{
	checkf(ProjectileDefinition, TEXT("%s has no ProjectileDefinition assigned."), *GetName());
	return *ProjectileDefinition;
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
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);

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

void ABaseProjectile::CacheDamageFromWeapon(const ABaseGun* InSourceWeapon)
{
	CachedWeaponDamage = InSourceWeapon ? FMath::Max(0.f, InSourceWeapon->GetBaseDamage()) : 0.f;
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

	// 피격 데미지 적용
	ApplyHitDamage(OtherActor, EffectiveHit);
	// 충돌 이벤트 브로드캐스트
	OnProjectileImpact(EffectiveHit);

	if (bShouldDestroyOnHit)
	{
		Destroy();
	}
}

void ABaseProjectile::ApplyHitDamage(AActor* DirectHitActor, const FHitResult& Hit)
{
	AActor* HitActor = DirectHitActor ? DirectHitActor : Hit.GetActor();
	const float DamageToApply = FMath::Max(0.f, CachedWeaponDamage);
	if (!HitActor || DamageToApply <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC)
	{
		// GE 전용 데미지 파이프라인: ASC 없는 대상은 데미지를 적용하지 않습니다.
		return;
	}

	AActor* SourceActor = GetOwner() ? GetOwner() : this;
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	const TSubclassOf<UGameplayEffect> DamageGEClass = CurrentDamageGameplayEffectClass
		? CurrentDamageGameplayEffectClass
		: TSubclassOf<UGameplayEffect>(UMGSDamageGameplayEffect::StaticClass());

	FGameplayEffectContextHandle EffectContext = SourceASC
		? SourceASC->MakeEffectContext()
		: TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(GetInstigator(), SourceActor);
	EffectContext.AddHitResult(Hit, true);

	FGameplayEffectSpecHandle DamageSpecHandle = SourceASC
		? SourceASC->MakeOutgoingSpec(DamageGEClass, 1.f, EffectContext)
		: TargetASC->MakeOutgoingSpec(DamageGEClass, 1.f, EffectContext);
	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		return;
	}

	// CurrentHp에 Additive 적용이므로 음수 값을 넣어 체력을 감소시킵니다.
	DamageSpecHandle.Data->SetSetByCallerMagnitude(MGSGameplayTags::Data_Damage, -FMath::Abs(DamageToApply));
	TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
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

	CurrentDamageGameplayEffectClass = Definition.DamageGameplayEffectClass
		? Definition.DamageGameplayEffectClass
		: TSubclassOf<UGameplayEffect>(UMGSDamageGameplayEffect::StaticClass());
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
