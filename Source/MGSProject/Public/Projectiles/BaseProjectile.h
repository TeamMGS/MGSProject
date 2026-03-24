/*
 * 파일명 : BaseProjectile.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-06
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSStructType.h"
#include "BaseProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
struct FHitResult;
struct FTimerHandle;
class APawn;
class UDA_ProjectileDefinition;
class UMGSProjectilePoolWorldSubsystem;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

USTRUCT()
struct FManagedProjectileNiagaraEntry
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<UNiagaraComponent> Component;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> SystemAsset;

	UPROPERTY(Transient)
	TWeakObjectPtr<USceneComponent> AttachParent;

	UPROPERTY(Transient)
	FName AttachPointName = NAME_None;

	UPROPERTY(Transient)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FVector RelativeScale = FVector::OneVector;
};

UCLASS()
class MGSPROJECT_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	// Getter
	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	USphereComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	UStaticMeshComponent* GetProjectileMeshComponent() const { return ProjectileMeshComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Data")
	const UDA_ProjectileDefinition* GetProjectileDefinition() const { return ProjectileDefinition; }

	UFUNCTION(BlueprintCallable, Category = "Projectile|FX")
	void RegisterManagedNiagaraComponent(UNiagaraComponent* NiagaraComponent);

	UFUNCTION(BlueprintCallable, Category = "Projectile|FX")
	void ClearManagedNiagaraComponents();

	// Setter
	// Pooling world subsystem 적재
	void SetProjectilePoolSubsystem(UMGSProjectilePoolWorldSubsystem* InProjectilePoolSubsystem);
	// 발사 Ability가 생성한 공격 payload를 적재
	void SetAttackPayload(const FMGSProjectileAttackPayload& InAttackPayload);
	
	// Pooling
	// 대기 상태 전환
	void DeactivateToPool();
	// Activate projectile (Pooling world subsystem에서 사용): Owner, Instigator, Transform, 충돌, 이동 컴포넌트 재설정
	void ActivateFromPool(const FTransform& SpawnTransform, AActor* NewOwner, APawn* NewInstigator);
	
	// 프로젝타일 초기 설정 (DA 적용, Velocity 및 각도 설정)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void InitializeProjectile(const FVector& ShootLocation, const FVector& ShootDirection);
	
protected:
	virtual void BeginPlay() override;
	
	// ProjectileMovement 정지 핸들러 (Block 충돌 시)
	UFUNCTION()
	virtual void HandleProjectileStop(const FHitResult& ImpactResult);
	
	// Projectile 충돌 처리
	void ProcessProjectileImpact(AActor* HitActor, const FHitResult& Hit);
	// 데미지 적용
	virtual void ApplyHitDamage(AActor* DirectHitActor, const FHitResult& Hit);
	// BP에 붙은 Niagara trail을 충돌/풀링 시 정리
	void DeactivateProjectileNiagaraComponents();
	// 풀에서 다시 꺼낼 때 Niagara 상태 초기화
	void ResetProjectileNiagaraComponents();
	// 
	void ReleaseToPoolOrDestroy();
	// 
	void StartProjectileLifeSpanTimer();
	// 
	void HandleLifeSpanExpired();
	
	// DA_ProjectileDefinition
	// 적재된 DA_ProjectileDefinition에 정의된 값을 적용
	void ApplyProjectileDefinition();
	// ProjectileDefinition이 반드시 설정되어 있다고 가정하고 참조 반환
	const UDA_ProjectileDefinition& GetProjectileDefinitionChecked() const;

protected:
	// 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	// 프로젝타일 무브먼트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	// 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMeshComponent;

	// DA_ProjectileDefinition
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Data")
	TObjectPtr<UDA_ProjectileDefinition> ProjectileDefinition;

	// Pooling world subsystem
	TWeakObjectPtr<UMGSProjectilePoolWorldSubsystem> ProjectilePoolSubsystem;

	// 발사 Ability가 만든 공격 정보
	FMGSProjectileAttackPayload AttackPayload;

	// DA_ProjectileDefinition이 적용되었는지 여부
	bool bHasAppliedDefinition = false;
	// Projectile 생존 시간
	float ActiveProjectileLifeSpan = 0.0f;
	// 부딪힌 후 생존 플래그
	bool bShouldDestroyOnHit = true;
	// Owner 무시 플래그
	bool bShouldIgnoreOwnerOnHit = true;
	// 이번 발사에서 충돌 처리를 이미 했는지 여부
	bool bHasProcessedImpact = false;
	// 현재 실제로 발사되어 활동 중인지
	bool bIsActiveInPool = false;

	// Life span timer
	FTimerHandle ProjectileLifeSpanTimerHandle;

	// BP에서 SpawnSystemAttached로 만든 Niagara trail을 명시적으로 관리
	UPROPERTY(Transient)
	TArray<FManagedProjectileNiagaraEntry> ManagedNiagaraComponents;
	
};
