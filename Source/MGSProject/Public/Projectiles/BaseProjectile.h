/*
 * 파일명: BaseProjectile.h
 * 생성자: 장대한
 * 생성일: 2026-03-06
 * 수정자: 장대한
 * 수정일: 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UDamageType;
class UDA_ProjectileDefinition;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
struct FHitResult;

UCLASS()
class MGSPROJECT_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	// 월드 방향 기준 velocity 초기화
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void InitializeProjectile(const FVector& ShootDirection);

	// 데미지 설정
	UFUNCTION(BlueprintCallable, Category = "Projectile|Damage")
	void SetProjectileDamage(float InDamage);

	UFUNCTION(BlueprintPure, Category = "Projectile|Damage")
	float GetProjectileDamage() const { return CurrentDamage; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	USphereComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Components")
	UStaticMeshComponent* GetProjectileMeshComponent() const { return ProjectileMeshComponent; }

	UFUNCTION(BlueprintPure, Category = "Projectile|Data")
	const UDA_ProjectileDefinition* GetProjectileDefinition() const { return ProjectileDefinition; }

protected:
	virtual void BeginPlay() override;

	// Overlap 이벤트 핸들러
	UFUNCTION()
	virtual void HandleProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 데미지 적용
	virtual void ApplyHitDamage(AActor* DirectHitActor, const FHitResult& Hit);

	// 충돌 이벤트
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OnProjectileImpact(const FHitResult& HitResult);
	virtual void OnProjectileImpact_Implementation(const FHitResult& HitResult);

	// 적재된 DA_ProjectileDefinition에 정의된 값을 적용
	void ApplyProjectileDefinition();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMeshComponent;

	// DA_ProjectileDefinition
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Data")
	TObjectPtr<UDA_ProjectileDefinition> ProjectileDefinition;

	UPROPERTY(Transient)
	TSubclassOf<UDamageType> CurrentDamageTypeClass;

	float CurrentDamage = 0.f;
	bool bShouldDestroyOnHit = true;
	bool bShouldIgnoreOwnerOnHit = true;
	bool bHasAppliedDefinition = false;
	bool bHasProcessedImpact = false;
	
};
