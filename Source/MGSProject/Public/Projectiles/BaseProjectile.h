/*
 * 파일명 : BaseProjectile.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-06
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UDA_ProjectileDefinition;
class ABaseGun;
class UGameplayEffect;
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

	// 프로젝타일 초기 설정 (DA 적용, Velocity 및 각도 설정)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void InitializeProjectile(const FVector& ShootDirection);

	// 무기에서 데미지 값을 읽어 발사 시점 데미지로 캐시
	void CacheDamageFromWeapon(const ABaseGun* InSourceWeapon);

	float GetCachedWeaponDamage() const { return CachedWeaponDamage; }

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

	// 데미지 GameplayEffect 클래스
	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> CurrentDamageGameplayEffectClass;

	// 무기에서 전달받은 발사 시점 데미지
	float CachedWeaponDamage = 0.0f;
	bool bShouldDestroyOnHit = true;
	bool bShouldIgnoreOwnerOnHit = true;
	bool bHasAppliedDefinition = false;
	bool bHasProcessedImpact = false;
	
};
