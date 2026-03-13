/*
 * 파일명 : BaseCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#include "Characters/BaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Components/MovementComponent/MGSCharacterMovementComponent.h"
#include "Components/StateTag/MGSLocomotionComponent.h"

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMGSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Tick을 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;
	// Decal의 영향을 받지 않음
	GetMesh()->bReceivesDecals = false;
	
	// 1. 커스텀 CMC 캐싱
	MGSMovementComponent = Cast<UMGSCharacterMovementComponent>(GetCharacterMovement());
	// 2. 로코모션 컴포넌트 생성
	LocomotionComponent = CreateDefaultSubobject<UMGSLocomotionComponent>(TEXT("LocomotionComponent"));
}

UPawnCombatComponent* ABaseCharacter::GetPawnCombatComponent() const
{
	// 파생 클래스에서 CombatComponent 소유
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

float ABaseCharacter::GetDamageMultiplierForHit(const FHitResult& Hit) const
{
	return 1.0f;
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	// playerstate가 인터페이스를 구현하고 있으니 요청을 넘김
	if (IAbilitySystemInterface* ASI = GetPlayerState<IAbilitySystemInterface>())
	{
		return ASI->GetAbilitySystemComponent();
	}

	// 만약 AI 캐릭터라면 PlayerState가 없을 수 있으니, 컴포넌트가 있는지 마지막으로 확인
	return Cast<UAbilitySystemComponent>(GetMGSAbilitySystemComponent());
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ensureMsgf(!StartupData.IsNull(), TEXT("Forgot to assigned startup data to %s"), *GetName());
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

void ABaseCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	// 로코모션 컴포넌트에게 점프 사실을 전달합니다.
	if (LocomotionComponent)
	{
		LocomotionComponent->HandleOnJumped();
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
