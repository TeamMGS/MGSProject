/*
 * 파일명 : EnemyFireGameplayAbility.cpp
 * 생성자 : Codex
 * 생성일 : 2026-03-13
 */

#include "GAS/GA/Enemy/EnemyFireGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Characters/Enemies/EnemyAIController.h"
#include "Characters/Enemies/EnemyCharacter.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "MGSStructType.h"
#include "Projectiles/BaseProjectile.h"
#include "Subsystems/ProjectilePoolWorldSubsystem.h"
#include "Weapon/BaseGun.h"

UEnemyFireGameplayAbility::UEnemyFireGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Enemy_Fire);
	SetAssetTags(AbilityAssetTags);
	ActivationOwnedTags.AddTag(MGSGameplayTags::State_Enemy_Attacking);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CurrentSpreadRadius = 0.0f;
}

bool UEnemyFireGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AEnemyCharacter* EnemyCharacter = ActorInfo ? Cast<AEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UEnemyCombatComponent* EnemyCombatComponent = EnemyCharacter ? EnemyCharacter->GetEnemyCombatComponent() : nullptr;
	const ABaseGun* EquippedGun = EnemyCombatComponent ? Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		return false;
	}

	const bool bCanFire = EquippedGun->CanFire();
	if (!bCanFire && bEnableAmmoLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAmmo][BLOCKED] %s Current=%d/%d Carried=%d"),
			*GetNameSafe(EnemyCharacter),
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo());
	}

	return bCanFire;
}

void UEnemyFireGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = EnemyCombatComponent ? Cast<ABaseGun>(EnemyCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EnemyCharacter || !EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UWeaponAttributeSet* WeaponAttributeSet = EnemyCharacter->GetWeaponAttributeSet())
	{
		CurrentSpreadRadius = EquippedGun->GetMaxSpreadRadius();
		WeaponAttributeSet->SetCurrentSpreadRadius(CurrentSpreadRadius);
	}
	else
	{
		CurrentSpreadRadius = EquippedGun->GetMaxSpreadRadius();
	}

	if (!FireSingleShot(EnemyCharacter, EquippedGun))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UEnemyFireGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo())
	{
		EnemyCharacter->RequestSpreadRefreshNextTick();
	}

	CurrentSpreadRadius = 0.0f;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UEnemyFireGameplayAbility::FireSingleShot(AEnemyCharacter* EnemyCharacter, ABaseGun* EquippedGun)
{
	if (!EnemyCharacter || !EquippedGun)
	{
		return false;
	}

	UWeaponAttributeSet* WeaponAttributeSet = EnemyCharacter->GetWeaponAttributeSet();
	if (!WeaponAttributeSet)
	{
		return false;
	}

	if (UAnimMontage* MontageToPlay = EquippedGun->GetWeaponData().FireMontage)
	{
		EnemyCharacter->PlayAnimMontage(MontageToPlay);
	}

	CurrentSpreadRadius = EquippedGun->GetMaxSpreadRadius();
	WeaponAttributeSet->SetCurrentSpreadRadius(CurrentSpreadRadius);
	const float AimReferenceDistance = EquippedGun->GetAimReferenceDistance();

	if (!EquippedGun->ConsumeAmmo(1))
	{
		if (bEnableAmmoLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAmmo][FAILED_CONSUME] %s Current=%d/%d Carried=%d"),
				*GetNameSafe(EnemyCharacter),
				EquippedGun->GetCurrentMagazineAmmo(),
				EquippedGun->GetMaxMagazineAmmo(),
				EquippedGun->GetCarriedAmmo());
		}

		return false;
	}

	if (!SpawnProjectileShot(EnemyCharacter, EquippedGun, AimReferenceDistance, CurrentSpreadRadius))
	{
		const bool bRefunded = EquippedGun->RefundAmmo(1);
		if (bEnableAmmoLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAmmo][ROLLBACK] %s Refunded=%s Current=%d/%d Carried=%d"),
				*GetNameSafe(EnemyCharacter),
				bRefunded ? TEXT("true") : TEXT("false"),
				EquippedGun->GetCurrentMagazineAmmo(),
				EquippedGun->GetMaxMagazineAmmo(),
				EquippedGun->GetCarriedAmmo());
		}
		return false;
	}

	CurrentSpreadRadius = EquippedGun->GetMaxSpreadRadius();
	WeaponAttributeSet->SetCurrentSpreadRadius(CurrentSpreadRadius);

	if (bEnableAmmoLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemyAmmo][FIRED] %s Current=%d/%d Carried=%d SpreadRadius=%.1f"),
			*GetNameSafe(EnemyCharacter),
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo(),
			CurrentSpreadRadius);
	}

	return true;
}

bool UEnemyFireGameplayAbility::SpawnProjectileShot(AEnemyCharacter* EnemyCharacter, ABaseGun* EquippedGun,
	float AimReferenceDistance, float SpreadRadius) const
{
	if (!EnemyCharacter || !EquippedGun)
	{
		return false;
	}

	const TSubclassOf<ABaseProjectile> ProjectileClassToSpawn = EquippedGun->GetProjectileClass();
	if (!ProjectileClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyFireProjectile][FAILED] ProjectileClass missing. Weapon=%s"),
			*GetNameSafe(EquippedGun));
		return false;
	}

	UWorld* World = EnemyCharacter->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector MuzzleTraceStart = EquippedGun->GetActorLocation();
	FVector MuzzleForwardDirection = EquippedGun->GetActorForwardVector();
	if (USceneComponent* WeaponRootComponent = EquippedGun->GetRootComponent())
	{
		if (MuzzleSocketName.IsNone())
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
			MuzzleForwardDirection = WeaponRootComponent->GetComponentRotation().Vector();
		}
		else if (WeaponRootComponent->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleTraceStart = WeaponRootComponent->GetSocketLocation(MuzzleSocketName);
			MuzzleForwardDirection = WeaponRootComponent->GetSocketRotation(MuzzleSocketName).Vector();
		}
		else
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
			MuzzleForwardDirection = WeaponRootComponent->GetComponentRotation().Vector();
		}
	}

	FVector AimTargetPoint = MuzzleTraceStart + (MuzzleForwardDirection * FMath::Max(1.f, AimReferenceDistance));
	if (const AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(EnemyCharacter->GetController()))
	{
		if (const AActor* TargetActor = EnemyAIController->GetCurrentTargetActor())
		{
			if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
			{
				AimTargetPoint = TargetPawn->GetPawnViewLocation();
			}
			else
			{
				AimTargetPoint = TargetActor->GetActorLocation();
			}
		}
	}

	FVector AimDirection = (AimTargetPoint - MuzzleTraceStart).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = MuzzleForwardDirection.GetSafeNormal();
	}

	const float AimDistance = FMath::Max(1.f, FVector::Distance(MuzzleTraceStart, AimTargetPoint));
	const float SpreadHalfAngleRad = FMath::Atan(SpreadRadius / AimDistance);
	const float SpreadHalfAngleDeg = FMath::RadiansToDegrees(SpreadHalfAngleRad);
	FVector MuzzleTraceDirection = AimDirection;
	if (MuzzleTraceDirection.IsNearlyZero())
	{
		MuzzleTraceDirection = EnemyCharacter->GetActorForwardVector();
	}
	if (SpreadHalfAngleRad > KINDA_SMALL_NUMBER)
	{
		MuzzleTraceDirection = FMath::VRandCone(MuzzleTraceDirection, SpreadHalfAngleRad);
	}

	constexpr float ProjectileSpawnForwardOffset = 20.f;
	const FVector ProjectileSpawnLocation = MuzzleTraceStart + (MuzzleTraceDirection * ProjectileSpawnForwardOffset);

	if (bEnableFireTraceDebug)
	{
		const FVector DebugEnd = MuzzleTraceStart + (MuzzleTraceDirection * AimReferenceDistance);
		DrawDebugLine(World, MuzzleTraceStart, DebugEnd, FColor::Red, false, DebugTraceDuration, 0, 1.2f);
	}

	AActor* DamageCauser = EnemyCharacter;
	APawn* InstigatorPawn = EnemyCharacter;
	UMGSProjectilePoolWorldSubsystem* ProjectilePoolSubsystem = World->GetSubsystem<UMGSProjectilePoolWorldSubsystem>();
	ABaseProjectile* SpawnedProjectile = ProjectilePoolSubsystem
		? ProjectilePoolSubsystem->AcquireProjectile(
			ProjectileClassToSpawn,
			FTransform(MuzzleTraceDirection.Rotation(), ProjectileSpawnLocation),
			DamageCauser,
			InstigatorPawn)
		: nullptr;

	if (!SpawnedProjectile)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = DamageCauser;
		SpawnParams.Instigator = InstigatorPawn;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedProjectile = World->SpawnActor<ABaseProjectile>(
			ProjectileClassToSpawn,
			ProjectileSpawnLocation,
			MuzzleTraceDirection.Rotation(),
			SpawnParams);
		if (SpawnedProjectile && ProjectilePoolSubsystem)
		{
			SpawnedProjectile->SetProjectilePoolSubsystem(ProjectilePoolSubsystem);
		}
	}

	if (!SpawnedProjectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyFireProjectile][FAILED] Spawn failed. Class=%s Muzzle=%s"),
			*GetNameSafe(ProjectileClassToSpawn.Get()),
			*ProjectileSpawnLocation.ToString());
		return false;
	}

	if (USphereComponent* ProjectileCollision = SpawnedProjectile->GetCollisionComponent())
	{
		ProjectileCollision->IgnoreActorWhenMoving(EnemyCharacter, true);
		ProjectileCollision->IgnoreActorWhenMoving(EquippedGun, true);
	}

	FMGSProjectileAttackPayload AttackPayload;
	AttackPayload.SourceActor = DamageCauser;
	AttackPayload.SourceObject = EquippedGun;
	AttackPayload.SourceASC = GetAbilitySystemComponentFromActorInfo();
	AttackPayload.DamageGameplayEffectClass = EquippedGun->GetDamageGameplayEffectClass();
	AttackPayload.BaseDamage = FMath::Max(0.f, EquippedGun->GetBaseDamage());
	SpawnedProjectile->SetAttackPayload(AttackPayload);
	SpawnedProjectile->InitializeProjectile(EquippedGun->GetMuzzleLocation(), MuzzleTraceDirection);

	if (bEnableFireTraceLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemyFireProjectile][SPAWN] Enemy=%s Class=%s SpreadHalfAngleDeg=%.3f Start=%s Direction=%s"),
			*GetNameSafe(EnemyCharacter),
			*GetNameSafe(ProjectileClassToSpawn.Get()),
			SpreadHalfAngleDeg,
			*ProjectileSpawnLocation.ToString(),
			*MuzzleTraceDirection.ToString());
	}

	return true;
}
