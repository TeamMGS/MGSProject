/*
 * 파일명 : BaseCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "Characters/BaseCharacter.h"

#include "Characters/Player/MGSPlayerState.h"

ABaseCharacter::ABaseCharacter()
{
	// 틱관련 기능 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// VFX 지상 데칼 투영 영향을 주지 않도록 처리
	GetMesh()->bReceivesDecals = false;
}

UPawnCombatComponent* ABaseCharacter::GetPawnCombatComponent() const
{
	// 부모 클래스에선 재구현 사용안함.
	return nullptr;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ensureMsgf(!StartupData.IsNull(), TEXT("Forgot to assigned startup data to %s"), *GetName());
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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

AMGSPlayerState* ABaseCharacter::GetMGSPlayerState() const
{
	if (const AController* CharacterController = GetController())
	{
		return CharacterController->GetPlayerState<AMGSPlayerState>();
	}

	return nullptr;
}

