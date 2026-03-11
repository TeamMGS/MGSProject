/*
 * 파일명 : PlayerCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-11
 */

#include "Characters/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Components/Combat/PlayerCombatComponent.h"
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


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	// 카메라 높이를 올려 정면 사격 시 과도한 상향 조준을 줄입니다.
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 90.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 220.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	PlayerCombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("PlayerCombatComponent"));
	
	// 모션워핑
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
	PerceptionStimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	PerceptionStimuliSource->RegisterForSense(UAISense_Hearing::StaticClass());
}

UPawnCombatComponent* APlayerCharacter::GetPawnCombatComponent() const
{
	return PlayerCombatComponent;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PerceptionStimuliSource)
	{
		PerceptionStimuliSource->RegisterWithPerceptionSystem();
	}

	ApplyAlwaysAimFacingMode();
	if (CameraBoom)
	{
		CameraBoom->TargetOffset.Z = bIsCrouched ? CrouchCameraCrouchedOffsetZ : CrouchCameraStandingOffsetZ;
		DesiredCrouchCameraOffsetZ = CameraBoom->TargetOffset.Z;
	}
	OnCharacterMovementUpdated.AddDynamic(this, &ThisClass::HandleSpreadMovementUpdated);

	if (PlayerCombatComponent)
	{
		EquippedWeaponChangedHandle = PlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().AddUObject(
			this,
			&ThisClass::HandleEquippedWeaponChanged);
		PlayerCombatComponent->RefreshNearbyDroppedWeaponCandidate();
	}

	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCharacterMovementUpdated.RemoveDynamic(this, &ThisClass::HandleSpreadMovementUpdated);
	GetWorldTimerManager().ClearTimer(CrouchCameraBlendTimerHandle);

	if (PlayerCombatComponent && EquippedWeaponChangedHandle.IsValid())
	{
		PlayerCombatComponent->GetOnEquippedWeaponChangedDelegate().Remove(EquippedWeaponChangedHandle);
		EquippedWeaponChangedHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ASC 초기화
	if (AMGSPlayerState* MGSPlayerState = GetPlayerState<AMGSPlayerState>())
	{
		MGSPlayerState->InitASC(this);
	}

	// DA_StartupPlayer의 어빌리티들을 동기 로드하여 ASC에 부여
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent(); ASC && !StartupData.IsNull())
	{
		if (UDA_StartupBase* LoadedData = StartupData.LoadSynchronous())
		{
			LoadedData->GiveToAbilitySystemComponent(ASC);
		}
	}

	UpdateFallingStateTag();
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	RequestRestoreHeldMovementAbilityInputNextTick();
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	UpdateFallingStateTag();
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
		ASC->OnAbilityInputPressed(InputTag);
	}

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
		ASC->OnAbilityInputReleased(InputTag);
	}

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

void APlayerCharacter::RequestRestoreHeldMovementAbilityInputNextTick()
{
	// Re-evaluate held movement inputs after character movement state settles.
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::TryRestoreHeldMovementAbilityInput);
}

void APlayerCharacter::ApplyAlwaysAimFacingMode()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	// 조준(컨트롤러) 방향과 캐릭터 Yaw를 항상 일치시킵니다.
	bUseControllerRotationYaw = true;
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->bUseControllerDesiredRotation = false;
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

void APlayerCharacter::UpdateFallingStateTag()
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!ASC || !MovementComponent)
	{
		return;
	}

	if (MovementComponent->IsFalling())
	{
		ASC->AddLooseGameplayTag(MGSGameplayTags::State_Player_Movement_Falling);
		return;
	}

	ASC->RemoveLooseGameplayTag(MGSGameplayTags::State_Player_Movement_Falling);
}

void APlayerCharacter::TryRestoreHeldMovementAbilityInput()
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!ASC || !MovementComponent || !MovementComponent->IsMovingOnGround())
	{
		return;
	}

	if (ASC->IsAbilityInputTagPressed(MGSGameplayTags::InputTag_Sprint))
	{
		ASC->OnAbilityInputPressed(MGSGameplayTags::InputTag_Sprint);
		return;
	}

	if (ASC->IsAbilityInputTagPressed(MGSGameplayTags::InputTag_Walk))
	{
		ASC->OnAbilityInputPressed(MGSGameplayTags::InputTag_Walk);
	}
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

	bPendingSpreadRefreshRequest = true;
	World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateCurrentSpreadFromState);
}

void APlayerCharacter::HandleSpreadMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	(void)DeltaSeconds;
	(void)OldLocation;
	(void)OldVelocity;
	if (PlayerCombatComponent)
	{
		PlayerCombatComponent->RefreshNearbyDroppedWeaponCandidate();
	}
	RequestSpreadRefreshNextTick();
}

void APlayerCharacter::HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag, FGameplayTag CurrentWeaponTag)
{
	(void)PreviousWeaponTag;
	(void)CurrentWeaponTag;
	RequestSpreadRefreshNextTick();
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
		// 발사 입력 유지 중에는 Fire 어빌리티의 누적 스프레드를 유지합니다.
		return;
	}

	const float MaxSpread = FMath::Max(0.f, EquippedGun->GetMaxSpreadRadius());
	const float BaseSpread = FMath::Clamp(EquippedGun->GetBaseSpreadRadius(), 0.f, MaxSpread);
	const float StateSpread = FMath::Clamp(BaseSpread * CalculateCurrentSpreadStateMultiplier(), 0.f, MaxSpread);
	if (!FMath::IsNearlyEqual(WeaponAttributeSet->GetCurrentSpreadRadius(), StateSpread))
	{
		WeaponAttributeSet->SetCurrentSpreadRadius(StateSpread);
	}
}

float APlayerCharacter::CalculateCurrentSpreadStateMultiplier() const
{
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent();
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	const bool bIsFalling = MovementComponent
		? MovementComponent->IsFalling()
		: (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Falling));

	const float HorizontalSpeed = MovementComponent
		? FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f).Size()
		: FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	const bool bIsMoving = HorizontalSpeed > SpreadMovingSpeedThreshold;
	const bool bIsSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint);
	const bool bIsWalk = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk);
	const bool bIsCrouching = bIsCrouched || (ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching));

	float StateSpreadMultiplier = 1.f;
	if (bIsFalling)
	{
		StateSpreadMultiplier = SpreadJumpMultiplier;
	}
	else if (bIsMoving)
	{
		if (bIsSprint)
		{
			StateSpreadMultiplier = SpreadSprintMultiplier;
		}
		else if (bIsWalk)
		{
			StateSpreadMultiplier = SpreadWalkMultiplier;
		}
		else
		{
			StateSpreadMultiplier = SpreadMovingMultiplier;
		}
	}
	else if (bIsCrouching)
	{
		StateSpreadMultiplier = SpreadCrouchStillMultiplier;
	}

	const bool bIsAiming = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	if (bIsAiming)
	{
		StateSpreadMultiplier *= SpreadAimMultiplier;
	}

	return FMath::Max(0.f, StateSpreadMultiplier);
}
