/*
 * 파일명 : PlayerCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-16
 */

#include "Characters/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "DataAssets/Spread/DA_SpreadSettings.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Math/RotationMatrix.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"
#include "MotionWarpingComponent.h"
#include "Weapon/BaseGun.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameplayEffectTypes.h"
#include "Components/TraversalComponent/MGSTraversalComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Collision size setting
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	// SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 90.0f);
	CameraBoom->ProbeSize = 6.0f;

	// Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Movement component
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	// Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));

	// Combat component
	PlayerCombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("PlayerCombatComponent"));
	
	// 파쿠르 컴포넌트
	TraversalComponent = CreateDefaultSubobject<UMGSTraversalComponent>(TEXT("TraversalComponent"));
	
	// 모션워핑
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// PerceptionStimuliSource component
	PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
	PerceptionStimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	PerceptionStimuliSource->RegisterForSense(UAISense_Hearing::StaticClass());
	
	// Controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
}

UPawnCombatComponent* APlayerCharacter::GetPawnCombatComponent() const
{
	return PlayerCombatComponent;
}

const UDA_SpreadSettings* APlayerCharacter::GetSpreadSettings() const
{
	return SpreadSettingsData ? SpreadSettingsData : GetDefault<UDA_SpreadSettings>();
}

void APlayerCharacter::RequestRestoreHeldMovementAbilityInputNextTick()
{
	// 다음 틱에 현재 누르고 있는 입력 어빌리티 상태 복원
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::TryRestoreHeldMovementAbilityInput);
}

void APlayerCharacter::StartAimObstructionTrace()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UpdateAimObstructionTrace();
	if (!World->GetTimerManager().IsTimerActive(AimObstructionTraceTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			AimObstructionTraceTimerHandle,
			this,
			&ThisClass::UpdateAimObstructionTrace,
			AimObstructionTraceTickInterval,
			true);
	}
}

void APlayerCharacter::StopAimObstructionTrace()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AimObstructionTraceTimerHandle);
	}

	RestoreAimObstructionVisibility();
}

void APlayerCharacter::GetAimObstructionActorsToIgnore(TArray<AActor*>& OutActors) const
{
	OutActors.Reset();

	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, bool>& HiddenComponentPair : HiddenAimObstructionComponents)
	{
		const UPrimitiveComponent* HiddenComponent = HiddenComponentPair.Key.Get();
		AActor* HiddenActor = HiddenComponent ? HiddenComponent->GetOwner() : nullptr;
		if (!IsValid(HiddenActor))
		{
			continue;
		}

		if (HiddenActor == this || HiddenActor->IsOwnedBy(this))
		{
			continue;
		}

		OutActors.AddUnique(HiddenActor);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 카메라의 시작 상태를 현재 자세와 맞춤
	// 웅크린 상태로 사작할 수도 있다는 가능성 + 카메라 보간용 내부 목표값을 실제 카메라 상태와 동기화
	if (CameraBoom)
	{
		CameraBoom->TargetOffset.Z = bIsCrouched ? CrouchCameraCrouchedOffsetZ : CrouchCameraStandingOffsetZ;
		DesiredCrouchCameraOffsetZ = CameraBoom->TargetOffset.Z;
	}
	
	// 이동 갱신 델리게이트에 이동 갱신에 따른 핸들러 바인딩 
	OnCharacterMovementUpdated.AddDynamic(this, &ThisClass::HandleMovementUpdated);

	if (PlayerCombatComponent)
	{
		// 무기 장착 변경 델리게이트에 무기 장착 변경에 따른 핸들러 바인딩
		EquippedWeaponChangedHandle = PlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().AddUObject(
			this,
			&ThisClass::HandleEquippedWeaponChanged);
		// 근처 드랍 무기 후보 평가 : 첫 프레임 근처에 드랍 무기가 있을 수 있기 때문
		PlayerCombatComponent->RefreshNearbyDroppedWeaponCandidate();
	}
	
	// 플레이어 캐릭터를 AIPerception이 감지할 수 있는 객체로 등록
	if (PerceptionStimuliSource)
	{
		PerceptionStimuliSource->RegisterWithPerceptionSystem();
	}

	// 다음 틱에 스프레드 갱신 : 첫 프레임 무기/이동 상태/조준 상태 기준으로 스프레드 동기화
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ASC 초기화
	if (AMGSPlayerState* MGSPlayerState = GetPlayerState<AMGSPlayerState>())
	{
		MGSPlayerState->InitASC(this);
	}

	// DA_StartupPlayer의 GA, GE들을 동기 로드하여 ASC에 부여
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent(); ASC && !StartupData.IsNull())
	{
		if (UDA_StartupBase* LoadedData = StartupData.LoadSynchronous())
		{
			LoadedData->GiveToAbilitySystemComponent(ASC);
		}
	}

	BindHpChangedDelegate();

	// ASC 초기화 이후 첫 프레임 공중 상태 태그 갱신
	UpdateFallingStateTag();
	// ASC 초기화, Tag부여 이후 상태가 변경될 수 있기 때문에 스프레드 재계산
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 이동 갱신 델리게이트에 바인딩 해제
	OnCharacterMovementUpdated.RemoveDynamic(this, &ThisClass::HandleMovementUpdated);

	// 무기 장창 변경 델리게이트에 바인딩 해제
	if (PlayerCombatComponent && EquippedWeaponChangedHandle.IsValid())
	{
		PlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().Remove(EquippedWeaponChangedHandle);
		EquippedWeaponChangedHandle.Reset();
	}

	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent(); ASC && bHasBoundHpChangedDelegate)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
			.Remove(CurrentHpChangedDelegateHandle);
		CurrentHpChangedDelegateHandle.Reset();
		bHasBoundHpChangedDelegate = false;
	}
	
	// 웅크리기 카메라 보간 타이머 핸들 정리
	GetWorldTimerManager().ClearTimer(CrouchCameraBlendTimerHandle);
	GetWorldTimerManager().ClearTimer(AimObstructionTraceTimerHandle);
	RestoreAimObstructionVisibility();

	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	// 공중 상태 조회 후 태그 갱신
	UpdateFallingStateTag();
	// 다음 틱에 스프레드 갱신
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 현재 누르고 있는 입력 어빌리티를 다음 틱에 복원 요청
	RequestRestoreHeldMovementAbilityInputNextTick();
	// 다음 틱에 스프레드 갱신
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
	
	// 웅크리기 시작할 때 아래로 카메라 보간 시작
	StartCrouchCameraBlend(CrouchCameraCrouchedOffsetZ);
}

void APlayerCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
	
	// 웅크리기 끝날 때 위로 카메라 보간 시작
	StartCrouchCameraBlend(CrouchCameraStandingOffsetZ);
}

void APlayerCharacter::BindHpChangedDelegate()
{
	if (bHasBoundHpChangedDelegate)
	{
		return;
	}

	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	CurrentHpChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
		.AddUObject(this, &ThisClass::HandleCurrentHpChanged);
	bHasBoundHpChangedDelegate = true;
}

void APlayerCharacter::HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (AttributeChangeData.NewValue > KINDA_SMALL_NUMBER || AttributeChangeData.OldValue <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (!ASC || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Character_Dead))
	{
		return;
	}

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Death);
	ASC->TryActivateAbilitiesByTag(AbilityTags, true);
}

void APlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
	
	// 다음 틱에 스프레드 갱신
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::Input_AbilityInputPressed(FGameplayTag InputTag)
{
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		// ASC에 Tag와 매칭되는 GA 활성화
		ASC->OnAbilityInputPressed(InputTag);
	}

	// 스프레드 보정을 진행해야 하는 태그일 경우
	const bool bShouldRefreshSpread =
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Fire) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Aim) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Crouch) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Walk) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Sprint);
	
	if (bShouldRefreshSpread)
	{
		RequestSpreadRefreshNextTick();
	}
}

void APlayerCharacter::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		// ASC에 Tag와 매칭되는 GA 비활성화
		ASC->OnAbilityInputReleased(InputTag);
	}

	// 스프레드 보정을 진행해야 하는 태그일 경우
	const bool bShouldRefreshSpread =
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Fire) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Aim) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Crouch) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Walk) ||
		InputTag.MatchesTagExact(MGSGameplayTags::InputTag_Sprint);
	
	if (bShouldRefreshSpread)
	{
		RequestSpreadRefreshNextTick();
	}
}

void APlayerCharacter::HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	(void)DeltaSeconds;
	(void)OldLocation;
	(void)OldVelocity;
	
	if (PlayerCombatComponent)
	{
		// 근처 드랍 무기 후보 평가
		PlayerCombatComponent->RefreshNearbyDroppedWeaponCandidate();
	}
	
	// 다음 틱에 스프레드 갱신
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag)
{
	(void)PreviousWeaponTag;
	(void)CurrentWeaponTag;
	
	// 다음 틱에 스프레드 갱신
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::TryRestoreHeldMovementAbilityInput()
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!ASC || !MovementComponent || !MovementComponent->IsMovingOnGround())
	{
		return;
	}

	// Sprint GA 복원
	if (ASC->IsAbilityInputTagPressed(MGSGameplayTags::InputTag_Sprint))
	{
		ASC->OnAbilityInputPressed(MGSGameplayTags::InputTag_Sprint);
		return;
	}

	// Walk GA 복원
	if (ASC->IsAbilityInputTagPressed(MGSGameplayTags::InputTag_Walk))
	{
		ASC->OnAbilityInputPressed(MGSGameplayTags::InputTag_Walk);
	}
}

void APlayerCharacter::UpdateFallingStateTag()
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!ASC || !MovementComponent)
	{
		return;
	}

	// 플레이어가 공중에 떠있다면
	if (MovementComponent->IsFalling())
	{
		ASC->AddLooseGameplayTag(MGSGameplayTags::State_Player_Movement_Falling);
		return;
	}

	ASC->RemoveLooseGameplayTag(MGSGameplayTags::State_Player_Movement_Falling);
}

void APlayerCharacter::RequestSpreadRefreshNextTick()
{
	if (bPendingSpreadRefreshRequest)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 스프레드 갱신 진행 여부 플래그 참으로 설정
	bPendingSpreadRefreshRequest = true;
	// 다음 틱에 스프레드 보정
	World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateCurrentSpreadFromState);
}

void APlayerCharacter::UpdateCurrentSpreadFromState()
{
	bPendingSpreadRefreshRequest = false;

	UWeaponAttributeSet* WeaponAttributeSet = GetWeaponAttributeSet();
	if (!WeaponAttributeSet || !PlayerCombatComponent)
	{
		return;
	}

	ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	if (!EquippedGun)
	{
		return;
	}

	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	if (ASC && ASC->IsAbilityInputTagPressed(MGSGameplayTags::InputTag_Fire))
	{
		// 발사 입력 유지 중에는 Fire GA의 누적 스프레드를 유지
		return;
	}

	// 장착 무기의 최대 스프레드 반경
	const float MaxSpread = FMath::Max(0.f, EquippedGun->GetMaxSpreadRadius());
	// 장착 무기의 기본 스프레드 반경
	const float BaseSpread = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, MaxSpread);
	// 목표 스프레즈 값 계산
	const float StateSpread = FMath::Clamp(BaseSpread * CalculateCurrentSpreadStateMultiplier(), 0.f, MaxSpread);
	if (!FMath::IsNearlyEqual(WeaponAttributeSet->GetCurrentSpreadRadius(), StateSpread))
	{
		// Weapon AttributeSet의 현재 스프레드 갱신 
		WeaponAttributeSet->SetCurrentSpreadRadius(StateSpread);
	}
}

float APlayerCharacter::CalculateCurrentSpreadStateMultiplier() const
{
	const UDA_SpreadSettings* SpreadSettings = GetSpreadSettings();
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	const bool bIsFalling = MovementComponent
		? MovementComponent->IsFalling()
		: (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Falling));

	const float HorizontalSpeed = MovementComponent
		? FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size()
		: FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	const bool bIsMoving = HorizontalSpeed > SpreadSettings->MovingSpeedThreshold;
	const bool bIsSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint);
	const bool bIsWalk = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk);
	const bool bIsCrouching = bIsCrouched || (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching));

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

	// 조준 상태는 마지막에 보정
	const bool bIsAiming = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	if (bIsAiming)
	{
		StateSpreadMultiplier *= SpreadSettings->AimMultiplier;
	}

	return FMath::Max(0.f, StateSpreadMultiplier);
}

void APlayerCharacter::StartCrouchCameraBlend(float TargetOffsetZ)
{
	if (!CameraBoom)
	{
		return;
	}

	DesiredCrouchCameraOffsetZ = TargetOffsetZ;
	UWorld* World = GetWorld();
	// 타이머를 못돌리는 경우
	if (!World)
	{
		FVector CurrentTargetOffset = CameraBoom->TargetOffset;
		CurrentTargetOffset.Z = DesiredCrouchCameraOffsetZ;
		// 바로 목표 카메라 Z 오프셋으로 설정
		CameraBoom->TargetOffset = CurrentTargetOffset;
		return;
	}

	if (!World->GetTimerManager().IsTimerActive(CrouchCameraBlendTimerHandle))
	{
		// 타이머를 돌려 자연스러운 카메라 보간 진행
		World->GetTimerManager().SetTimer(
			CrouchCameraBlendTimerHandle,
			this,
			&ThisClass::UpdateCrouchCameraBlend,
			CrouchCameraBlendTickInterval,
			true);
	}
}

void APlayerCharacter::UpdateCrouchCameraBlend()
{
	UWorld* World = GetWorld();
	if (!CameraBoom || !World)
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(CrouchCameraBlendTimerHandle);
		}
		return;
	}

	const float DeltaSeconds = FMath::Max(World->GetDeltaSeconds(), KINDA_SMALL_NUMBER);
	FVector CurrentTargetOffset = CameraBoom->TargetOffset;
	// 카메라 보간
	CurrentTargetOffset.Z = FMath::FInterpTo(
		CurrentTargetOffset.Z,
		DesiredCrouchCameraOffsetZ,
		DeltaSeconds,
		CrouchCameraInterpSpeed);
	CameraBoom->TargetOffset = CurrentTargetOffset;

	// 현재 카메라 Z 오프셋이 목표 카메라 Z 오프셋과 거의 같을 경우 종료
	if (FMath::IsNearlyEqual(CurrentTargetOffset.Z, DesiredCrouchCameraOffsetZ, 0.1f))
	{
		CurrentTargetOffset.Z = DesiredCrouchCameraOffsetZ;
		CameraBoom->TargetOffset = CurrentTargetOffset;
		World->GetTimerManager().ClearTimer(CrouchCameraBlendTimerHandle);
	}
}

void APlayerCharacter::UpdateAimObstructionTrace()
{
	if (!FollowCamera)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AimObstructionTrace), false, this);
	QueryParams.AddIgnoredActor(this);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	TArray<FHitResult> HitResults;
	const FVector TraceStart = FollowCamera->GetComponentLocation();
	const FVector TraceEnd = GetActorLocation() + FVector(0.0f, 0.0f, BaseEyeHeight);
	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(AimObstructionSweepRadius);
	// Sweep
	World->SweepMultiByObjectType(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ObjectQueryParams,
		SweepShape,
		QueryParams);

	TSet<TWeakObjectPtr<UPrimitiveComponent>> CurrentHitComponents;
	for (const FHitResult& HitResult : HitResults)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitComponent) || !IsValid(HitActor))
		{
			continue;
		}

		if (HitActor == this || HitActor->IsOwnedBy(this))
		{
			continue;
		}

		// 이번에 Hit된 컴포넌트의 원래 가시성 캐싱 후 가시성 비활성화
		if (!HiddenAimObstructionComponents.Contains(HitComponent))
		{
			HiddenAimObstructionComponents.Add(HitComponent, HitComponent->IsVisible());
			HitComponent->SetVisibility(false, true);
		}

		CurrentHitComponents.Add(HitComponent);
	}

	// 비활성화된 컴포넌트 중 이번에 Hit되지 않은 컴포넌트 찾음 
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ComponentsToRestore;
	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, bool>& HiddenComponentPair : HiddenAimObstructionComponents)
	{
		if (!CurrentHitComponents.Contains(HiddenComponentPair.Key))
		{
			ComponentsToRestore.Add(HiddenComponentPair.Key);
		}
	}

	// 원래 가시성으로 복원 후 캐시에서 제거
	for (const TWeakObjectPtr<UPrimitiveComponent>& HiddenComponentKey : ComponentsToRestore)
	{
		if (bool* CachedVisibility = HiddenAimObstructionComponents.Find(HiddenComponentKey))
		{
			if (UPrimitiveComponent* HiddenComponent = HiddenComponentKey.Get())
			{
				HiddenComponent->SetVisibility(*CachedVisibility, true);
			}
		}

		HiddenAimObstructionComponents.Remove(HiddenComponentKey);
	}
}

void APlayerCharacter::RestoreAimObstructionVisibility()
{
	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, bool>& HiddenComponentPair : HiddenAimObstructionComponents)
	{
		if (UPrimitiveComponent* HiddenComponent = HiddenComponentPair.Key.Get())
		{
			HiddenComponent->SetVisibility(HiddenComponentPair.Value, true);
		}
	}

	HiddenAimObstructionComponents.Reset();
}
