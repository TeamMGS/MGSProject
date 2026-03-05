/*
 * 파일명 : PlayerCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#include "Characters/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Math/RotationMatrix.h"
#include "TimerManager.h"

APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	// 카메라 높이를 올려 정면 사격 시 과도한 상향 조준을 줄입니다.
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 90.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
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
}

UPawnCombatComponent* APlayerCharacter::GetPawnCombatComponent() const
{
	return PlayerCombatComponent;
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
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	RequestRestoreHeldMovementAbilityInputNextTick();
}

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	UpdateFallingStateTag();
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
}

void APlayerCharacter::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
	{
		ASC->OnAbilityInputReleased(InputTag);
	}
}

void APlayerCharacter::RequestRestoreHeldMovementAbilityInputNextTick()
{
	// Re-evaluate held movement inputs after character movement state settles.
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::TryRestoreHeldMovementAbilityInput);
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


