/*
 * 파일명 : PlayerFireGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "GAS/GA/PlayerFireGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "DataAssets/Spread/DA_SpreadSettings.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Projectiles/BaseProjectile.h"
#include "Subsystems/ProjectilePoolWorldSubsystem.h"
#include "TimerManager.h"
#include "Weapon/BaseGun.h"

UPlayerFireGameplayAbility::UPlayerFireGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Fire);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
	CurrentSpreadRadius = 0.0f;
	CurrentFireInterval = 0.12f;
}

bool UPlayerFireGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// ActorInfo의 아바타 액터를 플레이어 캐릭터로 캐스팅
	const APlayerCharacter* PlayerCharacter = ActorInfo ? Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	// 플레이어 캐릭터에서 플레이어 컴뱃 컴포넌트 가져옴
	const UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter ? PlayerCharacter->GetPlayerCombatComponent() : nullptr;
	if (!PlayerCombatComponent)
	{
		return false;
	}

	// 컴뱃 컴포넌트에서 장착한 총기를 가져옴
	const ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	if (!EquippedGun)
	{
		return false;
	}

	// 장착한 총기가 발사 가능한 상태인지 확인
	const bool bCanFire = EquippedGun->CanFire();
	if (!bCanFire && bEnableAmmoLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Ammo][BLOCKED] Current=%d/%d Carried=%d"),
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo());
	}

	return bCanFire;
}

void UPlayerFireGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	// 컴뱃 컴포넌트에서 장착 총기 얻기
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스프레드 초기화
	CurrentSpreadRadius = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, EquippedGun->GetMaxSpreadRadius());
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (UWeaponAttributeSet* WeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet())
		{
			// 스프레드 보정값 계산
			const float StateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
			// 최종 스프레드 = 총기 스프레드 초기값 * 스프레드 보정값
			const float EffectiveSpreadRadius = FMath::Clamp(
				CurrentSpreadRadius * StateSpreadMultiplier,
				0.f,
				EquippedGun->GetMaxSpreadRadius());
			WeaponAttributeSet->SetCurrentSpreadRadius(EffectiveSpreadRadius);
		}
	}

	// 총기 연사 간격값으로 연사 간격 설정
	CurrentFireInterval = FMath::Max(0.01f, EquippedGun->GetFireInterval());

	// 실제 한 발 처리
	if (!FireSingleShot())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 타이머로 자동 연사 시작
	World->GetTimerManager().SetTimer(
		AutoFireTimerHandle,
		this,
		&ThisClass::HandleAutomaticFire,
		CurrentFireInterval,
		true);
}

void UPlayerFireGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPlayerFireGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (UWorld* World = AvatarActor->GetWorld())
		{
			// 자동 연사 종료
			World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		}
	}

	// 회전 모드 복원은 PlayerCharacter에서 상시 처리

	// 탄착군 초기화
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (UWeaponAttributeSet* WeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet())
		{
			CurrentSpreadRadius = FMath::Max(0.f, WeaponAttributeSet->GetBaseSpreadRadius());
			WeaponAttributeSet->SetCurrentSpreadRadius(CurrentSpreadRadius);
		}
		else
		{
			CurrentSpreadRadius = 0.f;
		}
	}
	else
	{
		CurrentSpreadRadius = 0.f;
	}

	// 연사 간격 초기화
	CurrentFireInterval = 0.12f;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPlayerFireGameplayAbility::HandleAutomaticFire()
{
	if (!CurrentActorInfo || !CurrentSpecHandle.IsValid())
	{
		return;
	}

	if (!FireSingleShot())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

bool UPlayerFireGameplayAbility::FireSingleShot()
{
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	AMGSPlayerController* PlayerController = GetMGSPlayerControllerFromActorInfo();
	if (!EquippedGun || !PlayerCharacter)
	{
		return false;
	}

	// 스프레드 보정값 계산
	const float StateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
	// 최종 스프레드 = 총기 스프레드 초기값 * 스프레드 보정값
	const float EffectiveSpreadRadius = FMath::Clamp(
		CurrentSpreadRadius * StateSpreadMultiplier,
		0.f,
		EquippedGun->GetMaxSpreadRadius());
	// 장착 총기 사거리값 가져옴
	const float AimReferenceDistance = EquippedGun->GetAimReferenceDistance();

	// 총알 한 발 소비
	if (!EquippedGun->ConsumeAmmo(1))
	{
		if (bEnableAmmoLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Ammo][FAILED_CONSUME] Current=%d/%d Carried=%d"),
				EquippedGun->GetCurrentMagazineAmmo(),
				EquippedGun->GetMaxMagazineAmmo(),
				EquippedGun->GetCarriedAmmo());
		}

		return false;
	}

	if (bEnableAmmoLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[Ammo][FIRED] Current=%d/%d Carried=%d SpreadRadius=%.1f (Base=%.1f, Mult=%.2f)"),
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo(),
			EffectiveSpreadRadius,
			CurrentSpreadRadius,
			StateSpreadMultiplier);
	}

	// 발사체 스폰
	if (!SpawnProjectileShot(
		PlayerCharacter,
		PlayerController,
		EquippedGun,
		AimReferenceDistance,
		EffectiveSpreadRadius))
	{
		const bool bRefunded = EquippedGun->RefundAmmo(1);
		if (bEnableAmmoLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Ammo][ROLLBACK] SpawnFailed Refunded=%s Current=%d/%d Carried=%d"),
				bRefunded ? TEXT("true") : TEXT("false"),
				EquippedGun->GetCurrentMagazineAmmo(),
				EquippedGun->GetMaxMagazineAmmo(),
				EquippedGun->GetCarriedAmmo());
		}
		return false;
	}

	// 총기 반동
	ApplyWeaponRecoil(PlayerController, EquippedGun);

	// 스프레드 증가
	CurrentSpreadRadius = FMath::Min(
		CurrentSpreadRadius + EquippedGun->GetSpreadRadiusIncreasePerShot(),
		EquippedGun->GetMaxSpreadRadius());

	// 플레이어 WeaponAttribute에 스프레드 갱신
	if (UWeaponAttributeSet* WeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet())
	{
		const float UpdatedStateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
		const float UpdatedEffectiveSpreadRadius = FMath::Clamp(
			CurrentSpreadRadius * UpdatedStateSpreadMultiplier,
			0.f,
			EquippedGun->GetMaxSpreadRadius());
		WeaponAttributeSet->SetCurrentSpreadRadius(UpdatedEffectiveSpreadRadius);
	}

	return true;
}

float UPlayerFireGameplayAbility::CalculateStateSpreadMultiplier(const APlayerCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return 1.f;
	}

	const UDA_SpreadSettings* SpreadSettings = PlayerCharacter->GetSpreadSettings();
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	const bool bIsFalling = MovementComponent
		? MovementComponent->IsFalling()
		: (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Falling));

	const float HorizontalSpeed = MovementComponent
		? FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size()
		: FVector(PlayerCharacter->GetVelocity().X, PlayerCharacter->GetVelocity().Y, 0.f).Size();
	const bool bIsMoving = HorizontalSpeed > SpreadSettings->MovingSpeedThreshold;
	const bool bIsSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint);
	const bool bIsWalk = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk);
	const bool bIsCrouching = PlayerCharacter->bIsCrouched || (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching));

	float StateSpreadMultiplier = 1.f;
	if (bIsFalling)
	{
		StateSpreadMultiplier = SpreadSettings->JumpMultiplier;
	}
	else if (bIsMoving)
	{
		if (bIsSprint)
		{
			StateSpreadMultiplier = SpreadSettings->SprintMultiplier;
		}
		else if (bIsWalk)
		{
			StateSpreadMultiplier = SpreadSettings->WalkMultiplier;
		}
		else
		{
			StateSpreadMultiplier = SpreadSettings->MovingMultiplier;
		}
	}
	else if (bIsCrouching)
	{
		StateSpreadMultiplier = SpreadSettings->CrouchStillMultiplier;
	}

	const bool bIsAiming = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	if (bIsAiming)
	{
		StateSpreadMultiplier *= SpreadSettings->AimMultiplier;
	}

	return FMath::Max(0.f, StateSpreadMultiplier);
}

void UPlayerFireGameplayAbility::ApplyWeaponRecoil(AMGSPlayerController* PlayerController, ABaseGun* EquippedGun) const
{
	if (!PlayerController || !EquippedGun || !PlayerController->IsLocalController())
	{
		return;
	}

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	float RecoilScale = 1.f;
	// 조준 상태일 때 총기 반동 스케일 보정
	if (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming))
	{
		RecoilScale *= EquippedGun->GetRecoilADSScale();
	}

	// 수직 반동
	const float PitchRecoil = FMath::Max(0.f, EquippedGun->GetRecoilPitchPerShot() * RecoilScale);
	// 수평 반동
	float YawRecoilMin = EquippedGun->GetRecoilYawPerShotMin() * RecoilScale;
	float YawRecoilMax = EquippedGun->GetRecoilYawPerShotMax() * RecoilScale;
	if (YawRecoilMax < YawRecoilMin)
	{
		Swap(YawRecoilMin, YawRecoilMax);
	}
	const float YawRecoil = FMath::FRandRange(YawRecoilMin, YawRecoilMax);

	// 플레이어 컨트롤러 반동 적용
	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Pitch = FRotator::NormalizeAxis(ControlRotation.Pitch + PitchRecoil);
	ControlRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw + YawRecoil);
	PlayerController->SetControlRotation(ControlRotation);

	if (!PlayerController->PlayerCameraManager)
	{
		return;
	}

	// 총기에 적용된 카메라 셰이크 클래스 가져옴
	const TSubclassOf<UCameraShakeBase> FireCameraShakeClass = EquippedGun->GetFireCameraShakeClass();
	if (!FireCameraShakeClass)
	{
		return;
	}

	// 카메라 셰이크 스케일값 설정
	const float RecoilMagnitude = FMath::Abs(PitchRecoil) + FMath::Abs(YawRecoil);
	const float FireCameraShakeScale = EquippedGun->GetFireCameraShakeScale() * RecoilMagnitude;
	if (FireCameraShakeScale <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// 카메라 셰이크 적용(연출)
	PlayerController->PlayerCameraManager->StartCameraShake(FireCameraShakeClass, FireCameraShakeScale);
}

bool UPlayerFireGameplayAbility::SpawnProjectileShot(APlayerCharacter* PlayerCharacter, AMGSPlayerController* PlayerController,
	ABaseGun* EquippedGun, float AimReferenceDistance, float SpreadRadius) const
{
	if (!PlayerCharacter || !EquippedGun)
	{
		return false;
	}

	const float WeaponDamage = FMath::Max(0.f, EquippedGun->GetBaseDamage());

	const TSubclassOf<ABaseProjectile> ProjectileClassToSpawn = EquippedGun->GetProjectileClass();
	if (!ProjectileClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FireProjectile][FAILED] ProjectileClass is not set in weapon definition. Weapon=%s"),
			*GetNameSafe(EquippedGun));
		return false;
	}

	AActor* DamageCauser = GetAvatarActorFromActorInfo();
	APawn* InstigatorPawn = Cast<APawn>(DamageCauser);
	
	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return false;
	}

	
	// Muzzle(총구) 소켓으로 플레이어 캐릭터 총알 발사 지점 설정
	FVector MuzzleTraceStart = EquippedGun->GetActorLocation();
	FVector MuzzleForwardDirection = EquippedGun->GetActorForwardVector();
	if (USceneComponent* WeaponRootComponent = EquippedGun->GetRootComponent())
	{
		// Muzzle 소켓이 없을 경우 장착 무기의 루트 컴포넌트로
		if (MuzzleSocketName.IsNone())
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
			MuzzleForwardDirection = WeaponRootComponent->GetComponentRotation().Vector();
		}
		// Muzzle 소켓이 존재할 경우 소켓의 위치로
		else if (WeaponRootComponent->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleTraceStart = WeaponRootComponent->GetSocketLocation(MuzzleSocketName);
			MuzzleForwardDirection = WeaponRootComponent->GetSocketRotation(MuzzleSocketName).Vector();
		}
		// 그 외의 경우 장착 무기의 루트 컴포넌트로
		else
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
			MuzzleForwardDirection = WeaponRootComponent->GetComponentRotation().Vector();
		}
	}

	// 화면 중앙 시점(컨트롤러 에임 회전) 기준 목표 지점을 계산합니다.
	FVector AimDirection = FVector::ZeroVector;
	FVector AimTargetPoint = MuzzleTraceStart + (MuzzleForwardDirection.GetSafeNormal() * FMath::Max(1.f, AimReferenceDistance));
	if (PlayerController)
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		AimTargetPoint = ViewLocation + (ViewRotation.Vector() * FMath::Max(1.f, AimReferenceDistance));
	}
	else
	{
		AimTargetPoint = MuzzleTraceStart + (PlayerCharacter->GetBaseAimRotation().Vector() * FMath::Max(1.f, AimReferenceDistance));
	}

	AimDirection = (AimTargetPoint - MuzzleTraceStart).GetSafeNormal();

	if (AimDirection.IsNearlyZero())
	{
		AimDirection = MuzzleForwardDirection.GetSafeNormal();
	}

	// 스프레드 반지름/사거리로 반각(theta)을 계산합니다.
	// tan θ = 높이(스프레드 반지름)/밑변(사거리) <=> atan(높이/밑변) = θ
	const float SpreadHalfAngleRad = FMath::Atan(SpreadRadius / FMath::Max(1.f, AimReferenceDistance));
	const float SpreadHalfAngleDeg = FMath::RadiansToDegrees(SpreadHalfAngleRad);
	FVector MuzzleTraceDirection = AimDirection.GetSafeNormal();
	if (MuzzleTraceDirection.IsNearlyZero())
	{
		MuzzleTraceDirection = PlayerCharacter->GetActorForwardVector();
	}
	if (SpreadHalfAngleRad > KINDA_SMALL_NUMBER)
	{
		// 중심 방향 기준 반각 내 랜덤 콘 방향 생성
		MuzzleTraceDirection = FMath::VRandCone(MuzzleTraceDirection, SpreadHalfAngleRad);
	}

	// 총구/손 근처 캡슐 내부 스폰을 피하기 위해 전진 오프셋 적용
	constexpr float ProjectileSpawnForwardOffset = 20.f;
	const FVector ProjectileSpawnLocation = MuzzleTraceStart + (MuzzleTraceDirection * ProjectileSpawnForwardOffset);

	if (bEnableFireTraceDebug)
	{
		const FVector DebugEnd = MuzzleTraceStart + (MuzzleTraceDirection * AimReferenceDistance);
		DrawDebugLine(World, MuzzleTraceStart, DebugEnd, FColor::Green, false, DebugTraceDuration, 0, 1.2f);
	}

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
		UE_LOG(LogTemp, Warning, TEXT("[FireProjectile][PoolFallback] Class=%s Reason=%s"),
			*GetNameSafe(ProjectileClassToSpawn.Get()),
			ProjectilePoolSubsystem ? TEXT("AcquireFailed") : TEXT("SubsystemMissing"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = DamageCauser;
		SpawnParams.Instigator = InstigatorPawn;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedProjectile = World->SpawnActor<ABaseProjectile>(
			ProjectileClassToSpawn,
			ProjectileSpawnLocation,
			MuzzleTraceDirection.Rotation(),
			SpawnParams);
	}

	if (!SpawnedProjectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FireProjectile][FAILED] Spawn failed. Class=%s Muzzle=%s"),
			*GetNameSafe(ProjectileClassToSpawn.Get()),
			*ProjectileSpawnLocation.ToString());
		return false;
	}

	// Spawn 직후 Owner/Instigator/무기 충돌을 즉시 무시해 초기 프레임 자기 충돌 방지
	if (USphereComponent* ProjectileCollision = SpawnedProjectile->GetCollisionComponent())
	{
		if (DamageCauser)
		{
			ProjectileCollision->IgnoreActorWhenMoving(DamageCauser, true);
		}
		if (InstigatorPawn)
		{
			ProjectileCollision->IgnoreActorWhenMoving(InstigatorPawn, true);
		}
		if (PlayerCharacter)
		{
			ProjectileCollision->IgnoreActorWhenMoving(PlayerCharacter, true);
		}
		if (EquippedGun)
		{
			ProjectileCollision->IgnoreActorWhenMoving(EquippedGun, true);
		}
	}

	SpawnedProjectile->CacheDamageFromWeapon(EquippedGun);
	SpawnedProjectile->InitializeProjectile(MuzzleTraceDirection);

	if (bEnableFireTraceLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[FireProjectile][SPAWN] Class=%s SpreadHalfAngleDeg=%.3f MuzzleStart=%s Direction=%s Damage=%.1f"),
			*GetNameSafe(ProjectileClassToSpawn.Get()),
			SpreadHalfAngleDeg,
			*ProjectileSpawnLocation.ToString(),
			*MuzzleTraceDirection.ToString(),
			WeaponDamage);
	}

	return true;
}

