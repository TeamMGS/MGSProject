/*
 * 파일명 : PlayerFireGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-16
 */

#include "GAS/GA/Player/PlayerFireGameplayAbility.h"

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
#include "MGSDebugHelper.h"
#include "MGSStructType.h"
#include "Engine/World.h"
#include "Projectiles/BaseProjectile.h"
#include "Subsystems/ProjectilePoolWorldSubsystem.h"
#include "TimerManager.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Weapon/BaseGun.h"

UPlayerFireGameplayAbility::UPlayerFireGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Fire);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
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
	const UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	const ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 장착 총기의 기본 스프레드값으로 현재 스프레드 설정
	CurrentSpreadRadius = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, EquippedGun->GetMaxSpreadRadius());
	if (const APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
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
			// WeaponAttributeSet에 현재 스프레드 값 갱신 
			WeaponAttributeSet->SetCurrentSpreadRadius(EffectiveSpreadRadius);
		}
	}
	// 장착 총기의 총기 연사 간격값으로 현재 연사 간격 설정
	CurrentFireInterval = FMath::Max(0.01f, EquippedGun->GetFireInterval());

	// 실제 한 발 처리
	if (!FireSingleShot())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
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
			// 자동 연사 타이머 종료
			World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		}
	}

	// 탄착군 초기화
	if (const APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (UWeaponAttributeSet* WeaponAttributeSet = PlayerCharacter->GetWeaponAttributeSet())
		{
			CurrentSpreadRadius = FMath::Max(0.0f, WeaponAttributeSet->GetBaseSpreadRadius());
			WeaponAttributeSet->SetCurrentSpreadRadius(CurrentSpreadRadius);
		}
		else
		{
			CurrentSpreadRadius = 0.0f;
		}
	}
	else
	{
		CurrentSpreadRadius = 0.0f;
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
	const UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	AMGSPlayerController* PlayerController = GetMGSPlayerControllerFromActorInfo();
	if (!EquippedGun || !PlayerCharacter)
	{
		return false;
	}

	// 발사 몽타쥬 재생
	if (UAnimMontage* MontageToPlay = EquippedGun->GetWeaponData().FireMontage)
	{
		PlayerCharacter->PlayAnimMontage(MontageToPlay);
	}
	
	// 스프레드 보정값 계산
	const float StateSpreadMultiplier = CalculateStateSpreadMultiplier(PlayerCharacter);
	// 최종 스프레드 = 총기 스프레드 초기값 * 스프레드 보정값
	const float EffectiveSpreadRadius = FMath::Clamp(
		CurrentSpreadRadius * StateSpreadMultiplier,
		0.f,
		EquippedGun->GetMaxSpreadRadius());

	// 총알 한 발 소비
	if (!EquippedGun->ConsumeAmmo(1))
	{
		return false;
	}
	
	// 장착 총기 라인트레이싱 사거리값 가져옴
	const float AimReferenceDistance = EquippedGun->GetAimReferenceDistance();
	// 발사체 스폰
	if (!SpawnProjectileShot(
		PlayerCharacter,
		PlayerController,
		EquippedGun,
		AimReferenceDistance,
		EffectiveSpreadRadius))
	{
		// 발사체 스폰 실패 시 총알 한 발 복구
		const bool bRefunded = EquippedGun->RefundAmmo(1);
		
		return false;
	}

	// 총기 반동
	ApplyWeaponRecoil(PlayerController, EquippedGun);

	// 현재 스프레드 갱신
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

	if (AMGSPlayerState* PlayerState = PlayerController->GetPlayerState<AMGSPlayerState>())
	{
		FGameplayCueParameters Parameters;
		Parameters.Location = EquippedGun->GetMuzzleLocation();
		Parameters.SourceObject = EquippedGun;
		PlayerState->GetMGSAbilitySystemComponent()->ExecuteGameplayCue(MGSGameplayTags::GameplayCue_Weapon_Fire, Parameters);
	}

	return true;
}

float UPlayerFireGameplayAbility::CalculateStateSpreadMultiplier(const APlayerCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return 1.0f;
	}

	const UDA_SpreadSettings* SpreadSettings = PlayerCharacter->GetSpreadSettings();
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	// 플레이어가 공중에서 떨어지는 상태인지
	const bool bIsFalling = MovementComponent
		? MovementComponent->IsFalling()
		: (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Falling));
	// 플레이어 2D 이동 속도
	const float HorizontalSpeed = MovementComponent
		? FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size()
		: FVector(PlayerCharacter->GetVelocity().X, PlayerCharacter->GetVelocity().Y, 0.f).Size();
	// 플레이어가 이동 상태인지
	const bool bIsMoving = HorizontalSpeed > SpreadSettings->MovingSpeedThreshold;
	// 플레이어가 뛰는 상태인지
	const bool bIsSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint);
	// 플레이어가 걷는 상태인지
	const bool bIsWalk = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk);
	// 플레이어가 웅크린 상태인지
	const bool bIsCrouching = PlayerCharacter->bIsCrouched || (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching));

	// 스프레드 보정값
	float StateSpreadMultiplier = 1.0f;
	// 공중에서 떨어지는 상태
	if (bIsFalling)
	{
		StateSpreadMultiplier = SpreadSettings->JumpMultiplier;
	}
	// 이동 상태
	else if (bIsMoving)
	{
		// 뛰는 상태
		if (bIsSprint)
		{
			StateSpreadMultiplier = SpreadSettings->SprintMultiplier;
		}
		// 걷는 상태
		else if (bIsWalk)
		{
			StateSpreadMultiplier = SpreadSettings->WalkMultiplier;
		}
		// 기본 이동 상태
		else
		{
			StateSpreadMultiplier = SpreadSettings->MovingMultiplier;
		}
	}
	// 웅크린 상태
	else if (bIsCrouching)
	{
		StateSpreadMultiplier = SpreadSettings->CrouchStillMultiplier;
	}

	// 조준 상태일 때 최종 보정
	const bool bIsAiming = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	if (bIsAiming)
	{
		StateSpreadMultiplier *= SpreadSettings->AimMultiplier;
	}

	return FMath::Max(0.f, StateSpreadMultiplier);
}

bool UPlayerFireGameplayAbility::SpawnProjectileShot(APlayerCharacter* PlayerCharacter, const AMGSPlayerController* PlayerController,
	ABaseGun* EquippedGun, float AimReferenceDistance, float SpreadRadius) const
{
	if (!PlayerCharacter || !EquippedGun)
	{
		return false;
	}

	// Projectile class
	const TSubclassOf<ABaseProjectile> ProjectileClassToSpawn = EquippedGun->GetProjectileClass();
	if (!ProjectileClassToSpawn)
	{
		const FString& Msg = FString::Printf(TEXT("[%s::%s] ProjectileClass is not set in %s definition"), *GetNameSafe(GetAvatarActorFromActorInfo()), *GetName(), *GetNameSafe(EquippedGun));
		Debug::Print(Msg, FColor::Yellow);
		return false;
	}
	
	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return false;
	}
	
	// 총알 발사 지점 설정
	FVector MuzzleTraceStart = EquippedGun->GetActorLocation();
	FVector MuzzleForwardDirection = EquippedGun->GetActorForwardVector();
	if (USceneComponent* WeaponRootComponent = EquippedGun->GetRootComponent())
	{
		// Muzzle 소켓이 존재할 경우 소켓의 위치로
		if (WeaponRootComponent->DoesSocketExist(MuzzleSocketName))
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

	// 목표 지점 설정
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	// PlayerController가 존재할 경우 사용자가 바라보는 방향으로 설정
	if (PlayerController)
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	// PlayerController가 존재하지 않을 경우 캐릭터가 바라보는 방향으로 설정
	else
	{
		ViewLocation = PlayerCharacter->GetPawnViewLocation();
		ViewRotation = PlayerCharacter->GetBaseAimRotation();
	}

	// 라인 트레이스 거리
	const float TraceDistance = FMath::Max(1.0f, AimReferenceDistance);
	const FVector ViewTraceEnd = ViewLocation + (ViewRotation.Vector() * TraceDistance);
	
	FCollisionQueryParams AimQueryParams(SCENE_QUERY_STAT(PlayerFireAimTrace), false, PlayerCharacter);
	// 플레이어 캐릭터 무시
	AimQueryParams.AddIgnoredActor(PlayerCharacter);
	// 장착 무기 무시
	AimQueryParams.AddIgnoredActor(EquippedGun);
	// 조준 시 플레이어와 카메라 사이 숨겨진 장애물, 벽같은 오브젝트 무시
	TArray<AActor*> HiddenObstructionActors;
	PlayerCharacter->GetAimObstructionActorsToIgnore(HiddenObstructionActors);
	for (AActor* HiddenActor : HiddenObstructionActors)
	{
		if (IsValid(HiddenActor))
		{
			AimQueryParams.AddIgnoredActor(HiddenActor);
		}
	}
	
	FVector AimTargetPoint = ViewTraceEnd;
	FHitResult AimHitResult;
	// 라인 트레이스
	if (World->LineTraceSingleByChannel(AimHitResult, ViewLocation, ViewTraceEnd, ECC_Visibility, AimQueryParams))
	{
		AimTargetPoint = AimHitResult.ImpactPoint;
	}

	FVector AimDirection = (AimTargetPoint - MuzzleTraceStart).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = MuzzleForwardDirection.GetSafeNormal();
	}

	// 스프레드 반지름/사거리로 반각(theta)을 계산합니다.
	// tan θ = 높이(스프레드 반지름)/밑변(사거리) <=> atan(높이/밑변) = θ
	const float SpreadHalfAngleRad = FMath::Atan(SpreadRadius / FMath::Max(1.f, AimReferenceDistance));
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
	constexpr float ProjectileSpawnForwardOffset = 20.0f;
	const FVector ProjectileSpawnLocation = MuzzleTraceStart + (MuzzleTraceDirection * ProjectileSpawnForwardOffset);

	// Owner
	AActor* DamageCauser = PlayerCharacter;
	// Instigator
	APawn* InstigatorPawn = PlayerCharacter;
	
	// 풀링 시스템에서 총알 한개 가져옴
	UMGSProjectilePoolWorldSubsystem* ProjectilePoolSubsystem = World->GetSubsystem<UMGSProjectilePoolWorldSubsystem>();
	ABaseProjectile* SpawnedProjectile = ProjectilePoolSubsystem
		? ProjectilePoolSubsystem->AcquireProjectile(
			ProjectileClassToSpawn,
			FTransform(MuzzleTraceDirection.Rotation(), ProjectileSpawnLocation),
			DamageCauser,
			InstigatorPawn)
		: nullptr;
	
	// 풀링 시스템에서 총알 가져오기를 실패하면
	if (!SpawnedProjectile)
	{
		const FString& Msg = FString::Printf(TEXT("[%s::%s] %s get failed = %s"), 
			*GetNameSafe(GetAvatarActorFromActorInfo()), *GetName(), 
			*GetNameSafe(ProjectileClassToSpawn.Get()), 
			ProjectilePoolSubsystem ? TEXT("AcquireFailed") : TEXT("SubsystemMissing"));
		Debug::Print(Msg, FColor::Yellow);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = DamageCauser;
		SpawnParams.Instigator = InstigatorPawn;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 총알 스폰
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
	
	// 총알 스폰도 실패
	if (!SpawnedProjectile)
	{
		const FString& Msg = FString::Printf(TEXT("[%s::%s] %s spawn failed"), *GetNameSafe(GetAvatarActorFromActorInfo()), *GetName(), *GetNameSafe(ProjectileClassToSpawn.Get()));
		Debug::Print(Msg, FColor::Red);
		
		return false;
	}

	// Spawn 직후 Owner/Instigator/무기/숨겨진 장애물 충돌을 즉시 무시해 초기 프레임 자기 충돌 방지
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
		for (AActor* HiddenActor : HiddenObstructionActors)
		{
			if (IsValid(HiddenActor))
			{
				ProjectileCollision->IgnoreActorWhenMoving(HiddenActor, true);
			}
		}
	}

	const float WeaponDamage = FMath::Max(0.0f, EquippedGun->GetBaseDamage());
	
	FMGSProjectileAttackPayload AttackPayload;
	AttackPayload.SourceActor = DamageCauser;
	AttackPayload.SourceObject = EquippedGun;
	AttackPayload.SourceASC = GetAbilitySystemComponentFromActorInfo();
	AttackPayload.DamageGameplayEffectClass = EquippedGun->GetDamageGameplayEffectClass();
	AttackPayload.BaseDamage = WeaponDamage;
	// Projectile에 Payload 정보 저장
	SpawnedProjectile->SetAttackPayload(AttackPayload);
	// Projectile 초기화 (속도, 방향, 수명 타이머 등)
	SpawnedProjectile->InitializeProjectile(MuzzleTraceDirection);

	return true;
}

void UPlayerFireGameplayAbility::ApplyWeaponRecoil(AMGSPlayerController* PlayerController, const ABaseGun* EquippedGun) const
{
	if (!PlayerController || !EquippedGun || !PlayerController->IsLocalController())
	{
		return;
	}

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	float RecoilScale = 1.0f;
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
