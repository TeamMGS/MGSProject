/*
 * 파일명 : BaseCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-09
 */

#include "Characters/BaseCharacter.h"

#include "Characters/Player/MGSPlayerState.h"
#include "Components/MovementComponent/MGSCharacterMovementComponent.h"
#include "Components/StateTag/MGSLocomotionComponent.h"

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMGSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 성능을 위해 Tick을 사용하지 않습니다.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// VFX Decal 영향이 필요 없는 캐릭터는 비활성화합니다.
	GetMesh()->bReceivesDecals = false;
	
	// 1. 커스텀 CMC 캐싱
	MGSMovementComponent = Cast<UMGSCharacterMovementComponent>(GetCharacterMovement());

	// 2. 로코모션 컴포넌트 생성
	LocomotionComponent = CreateDefaultSubobject<UMGSLocomotionComponent>(TEXT("LocomotionComponent"));
}

UPawnCombatComponent* ABaseCharacter::GetPawnCombatComponent() const
{
	// 부모 클래스에서는 전투 컴포넌트를 직접 소유하지 않습니다.
	return nullptr;
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ensureMsgf(!StartupData.IsNull(), TEXT("Forgot to assigned startup data to %s"), *GetName());
}

UMGSAbilitySystemComponent* ABaseCharacter::GetMGSAbilitySystemComponent() const
{
	if (const AMGSPlayerState* MGSPlayerState = GetMGSPlayerState())
	{
		return MGSPlayerState->GetMGSAbilitySystemComponent();
	}

	return nullptr;
}

UCharacterAttributeSet* ABaseCharacter::GetCharacterAttributeSet() const
{
	if (const AMGSPlayerState* MGSPlayerState = GetMGSPlayerState())
	{
		return MGSPlayerState->GetCharacterAttributeSet();
	}

	return nullptr;
}

UWeaponAttributeSet* ABaseCharacter::GetWeaponAttributeSet() const
{
	if (const AMGSPlayerState* MGSPlayerState = GetMGSPlayerState())
	{
		return MGSPlayerState->GetWeaponAttributeSet();
	}

	return nullptr;
}

AMGSPlayerState* ABaseCharacter::GetMGSPlayerState() const
{
	if (const AController* CharacterController = GetController())
	{
		return CharacterController->GetPlayerState<AMGSPlayerState>();
	}

	return nullptr;
}

void ABaseCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	// 로코모션 컴포넌트에게 점프 사실을 전달합니다.
	if (LocomotionComponent)
	{
		LocomotionComponent->HandleOnJumped();
	}
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	// 모든 캐릭터 공통: 로코모션 컴포넌트에게 착지 알림
	if (LocomotionComponent)
	{
		LocomotionComponent->HandleOnLanded(GetVelocity());
	}
}

void ABaseCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
	if (LocomotionComponent)
	{
		LocomotionComponent->UpdateMovementTags(0.0f);
	}
}

void ABaseCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
	if (LocomotionComponent)
	{
		LocomotionComponent->UpdateMovementTags(0.0f);
	}
}


