/*
 * 파일명: PlayerFireGameplayAbility.cpp
 * 생성자: 장대한
 * 생성일: 2026-03-04
 * 수정자:  장대한
 * 수정일:  2026-03-05
 */

#include "GAS/GA/PlayerFireGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
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

	const APlayerCharacter* PlayerCharacter = ActorInfo ? Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter ? PlayerCharacter->GetPlayerCombatComponent() : nullptr;
	if (!PlayerCombatComponent)
	{
		return false;
	}

	const ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	if (!EquippedGun)
	{
		return false;
	}

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

	// 장착 장비 재확인
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 현재 탄착군 초기화
	CurrentSpreadRadius = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, EquippedGun->GetMaxSpreadRadius());
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (UWeaponAttributeSet* WeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet())
		{
			const float StateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
			const float EffectiveSpreadRadius = FMath::Clamp(
				CurrentSpreadRadius * StateSpreadMultiplier,
				0.f,
				EquippedGun->GetMaxSpreadRadius());
			WeaponAttributeSet->SetCurrentSpreadRadius(EffectiveSpreadRadius);
		}
	}

	CurrentFireInterval = FMath::Max(0.01f, EquippedGun->GetFireInterval());
	EnableCombatFacing(); // 사격 시 캐릭터 회전 설정
	if (!FireSingleShot()) // 실제 한 발 처리
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

	// 캐릭터 회전 설정 복원
	RestoreMovementFacing();

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

	const float StateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
	const float EffectiveSpreadRadius = FMath::Clamp(
		CurrentSpreadRadius * StateSpreadMultiplier,
		0.f,
		EquippedGun->GetMaxSpreadRadius());
	const float FireRange = EquippedGun->GetFireRange();

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

	AActor* DamageCauser = GetAvatarActorFromActorInfo();
	AController* InstigatorController = nullptr;
	if (const APawn* AvatarPawn = Cast<APawn>(DamageCauser))
	{
		InstigatorController = AvatarPawn->GetController();
	}

	// 라인 트레이스
	FireTraceAndApplyDamage(
		PlayerCharacter,
		PlayerController,
		EquippedGun,
		DamageCauser,
		InstigatorController,
		FireRange,
		EquippedGun->GetBaseDamage(),
		EffectiveSpreadRadius);

	// 탄착군 넓어짐
	CurrentSpreadRadius = FMath::Min(
		CurrentSpreadRadius + EquippedGun->GetSpreadRadiusIncreasePerShot(),
		EquippedGun->GetMaxSpreadRadius());

	// 무기 속성에 현재 탄착군 동기화
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

void UPlayerFireGameplayAbility::EnableCombatFacing()
{
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	if (!PlayerCharacter || !MovementComponent)
	{
		return;
	}

	// 회전 설정 캐싱
	if (!bHasCachedMovementFacing)
	{
		bCachedUseControllerRotationYaw = PlayerCharacter->bUseControllerRotationYaw;
		bCachedOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
		bCachedUseControllerDesiredRotation = MovementComponent->bUseControllerDesiredRotation;
		bHasCachedMovementFacing = true;
	}

	// 컨트롤러 Yaw를 즉시 고정하지 않고 RotationRate를 사용해 자연스럽게 회전합니다.
	PlayerCharacter->bUseControllerRotationYaw = false;
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->bUseControllerDesiredRotation = true;
}

void UPlayerFireGameplayAbility::RestoreMovementFacing()
{
	if (!bHasCachedMovementFacing)
	{
		return;
	}

	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	if (PlayerCharacter && MovementComponent)
	{
		PlayerCharacter->bUseControllerRotationYaw = bCachedUseControllerRotationYaw;
		MovementComponent->bOrientRotationToMovement = bCachedOrientRotationToMovement;
		MovementComponent->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
	}

	bHasCachedMovementFacing = false;
}

float UPlayerFireGameplayAbility::CalculateStateSpreadMultiplier(const APlayerCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return 1.f;
	}

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	const bool bIsFalling = MovementComponent
		? MovementComponent->IsFalling()
		: (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Falling));

	float MovementSpreadMultiplier = 1.f;
	if (bIsFalling)
	{
		MovementSpreadMultiplier = JumpSpreadMultiplier;
	}
	else if (MovementComponent)
	{
		const float HorizontalSpeed = FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size();
		if (HorizontalSpeed > MinMovingSpeedForSpread)
		{
			if (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint))
			{
				MovementSpreadMultiplier = SprintSpreadMultiplier;
			}
			else if (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk))
			{
				MovementSpreadMultiplier = WalkSpreadMultiplier;
			}
			else
			{
				MovementSpreadMultiplier = MovingSpreadMultiplier;
			}
		}
	}

	const bool bIsCrouching = PlayerCharacter->bIsCrouched;
	if (bIsCrouching)
	{
		MovementSpreadMultiplier *= CrouchSpreadMultiplier;
	}

	const bool bIsAiming = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	if (bIsAiming)
	{
		MovementSpreadMultiplier *= AimSpreadMultiplier;
	}

	return FMath::Max(0.f, MovementSpreadMultiplier);
}

bool UPlayerFireGameplayAbility::FireTraceAndApplyDamage(APlayerCharacter* PlayerCharacter, AMGSPlayerController* PlayerController,
	ABaseGun* EquippedGun, AActor* DamageCauser, AController* InstigatorController, float FireRange, float Damage,
	float SpreadRadius) const
{
	if (!PlayerCharacter || !EquippedGun)
	{
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;

	if (PlayerController)
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		PlayerCharacter->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	const FVector CameraTraceStart = ViewLocation;
	const FVector CameraTraceEnd = CameraTraceStart + (ViewRotation.Vector() * FireRange);
	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams AimQueryParams(SCENE_QUERY_STAT(PlayerWeaponAimTrace), false);
	AimQueryParams.AddIgnoredActor(PlayerCharacter);
	if (DamageCauser)
	{
		AimQueryParams.AddIgnoredActor(DamageCauser);
	}
	AimQueryParams.AddIgnoredActor(EquippedGun);

	// 카메라 시점 기준 라인 트레이스해서 AimPoint 결정
	FHitResult CameraHitResult;
	const bool bCameraHit = World->LineTraceSingleByChannel(CameraHitResult, CameraTraceStart, CameraTraceEnd, ECC_Visibility, AimQueryParams);
	const FVector AimPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraTraceEnd;

	
	// 총구 소켓(Muzzle) 위치를 시작점으로
	FVector MuzzleTraceStart = EquippedGun->GetActorLocation();
	if (USceneComponent* WeaponRootComponent = EquippedGun->GetRootComponent())
	{
		if (MuzzleSocketName.IsNone())
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
		}
		else if (WeaponRootComponent->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleTraceStart = WeaponRootComponent->GetSocketLocation(MuzzleSocketName);
		}
		else
		{
			MuzzleTraceStart = WeaponRootComponent->GetComponentLocation();
		}
	}

	const FVector AimDirectionNoSpread = (AimPoint - MuzzleTraceStart).GetSafeNormal();
	const float AimDistance = FMath::Max(1.f, FVector::Distance(MuzzleTraceStart, AimPoint));
	const float SpreadHalfAngleRad = FMath::Atan(SpreadRadius / AimDistance);
	const float SpreadHalfAngleDeg = FMath::RadiansToDegrees(SpreadHalfAngleRad);
	FVector MuzzleTraceDirection = AimDirectionNoSpread;
	if (MuzzleTraceDirection.IsNearlyZero())
	{
		MuzzleTraceDirection = ViewRotation.Vector();
	}

	if (SpreadHalfAngleRad > KINDA_SMALL_NUMBER)
	{
		MuzzleTraceDirection = FMath::VRandCone(MuzzleTraceDirection, SpreadHalfAngleRad);
	}

	const FVector MuzzleTraceEnd = MuzzleTraceStart + (MuzzleTraceDirection * FireRange);

	FHitResult HitResult;
	FCollisionQueryParams DamageQueryParams(SCENE_QUERY_STAT(PlayerWeaponDamageTrace), false);
	DamageQueryParams.AddIgnoredActor(PlayerCharacter);
	if (DamageCauser)
	{
		DamageQueryParams.AddIgnoredActor(DamageCauser);
	}
	DamageQueryParams.AddIgnoredActor(EquippedGun);

	// 총구 기준 라인 트레이스로 실제 피격 판정
	const bool bHit = World->LineTraceSingleByChannel(HitResult, MuzzleTraceStart, MuzzleTraceEnd, ECC_Visibility, DamageQueryParams);
	if (!bHit)
	{
		if (bEnableFireTraceDebug)
		{
			DrawDebugLine(World, MuzzleTraceStart, MuzzleTraceEnd, FColor::Red, false, DebugTraceDuration, 0, 1.2f);
		}

		if (bEnableFireTraceLog)
		{
			UE_LOG(LogTemp, Log, TEXT("[FireTrace][MISS] CameraStart=%s AimPoint=%s SpreadHalfAngleDeg=%.3f MuzzleStart=%s MuzzleEnd=%s"),
				*CameraTraceStart.ToString(), *AimPoint.ToString(), SpreadHalfAngleDeg, *MuzzleTraceStart.ToString(), *MuzzleTraceEnd.ToString());
		}

		return false;
	}

	if (bEnableFireTraceDebug)
	{
		DrawDebugLine(World, MuzzleTraceStart, HitResult.ImpactPoint, FColor::Green, false, DebugTraceDuration, 0, 1.6f);
		DrawDebugPoint(World, HitResult.ImpactPoint, 12.0f, FColor::Yellow, false, DebugTraceDuration, 0);
	}

	if (bEnableFireTraceLog)
	{
		const UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		const FString CollisionProfile = HitComponent ? HitComponent->GetCollisionProfileName().ToString() : TEXT("None");
		const int32 CollisionEnabled = HitComponent ? static_cast<int32>(HitComponent->GetCollisionEnabled()) : INDEX_NONE;

		UE_LOG(LogTemp, Log,
			TEXT("[FireTrace][HIT] Actor=%s Component=%s Profile=%s CollisionEnabled=%d Bone=%s Impact=%s Normal=%s Distance=%.1f"),
			*GetNameSafe(HitResult.GetActor()),
			*GetNameSafe(HitComponent),
			*CollisionProfile,
			CollisionEnabled,
			*HitResult.BoneName.ToString(),
			*HitResult.ImpactPoint.ToString(),
			*HitResult.ImpactNormal.ToString(),
			FVector::Distance(MuzzleTraceStart, HitResult.ImpactPoint));
	}

	// 히트 시
	UGameplayStatics::ApplyPointDamage(
		HitResult.GetActor(),
		Damage,
		MuzzleTraceDirection,
		HitResult,
		InstigatorController,
		DamageCauser,
		nullptr);

	return true;
}
