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
	// 성능을 위해 Tick을 사용하지 않습니다.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// VFX Decal 영향이 필요 없는 캐릭터는 비활성화합니다.
	GetMesh()->bReceivesDecals = false;
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


