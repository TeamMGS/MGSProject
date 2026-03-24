/*
 * 파일명 : MGSPlayerState.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "Characters/Player/MGSPlayerState.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"

AMGSPlayerState::AMGSPlayerState()
{
	MGSAbilitySystemComponent = CreateDefaultSubobject<UMGSAbilitySystemComponent>(TEXT("MGSAbilitySystemComponent"));
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
}

void AMGSPlayerState::InitASC(AActor* Avatar)
{
	if (!Avatar || !MGSAbilitySystemComponent)
	{
		return;
	}

	MGSAbilitySystemComponent->InitAbilityActorInfo(this, Avatar);
}

bool AMGSPlayerState::SetCurrentHpForDebug(float NewCurrentHp)
{
	if (!MGSAbilitySystemComponent || !CharacterAttributeSet)
	{
		return false;
	}

	const float MaxHp = FMath::Max(1.0f, CharacterAttributeSet->GetMaxHp());
	const float ClampedHp = FMath::Clamp(NewCurrentHp, 0.0f, MaxHp);
	MGSAbilitySystemComponent->SetNumericAttributeBase(UCharacterAttributeSet::GetCurrentHpAttribute(), ClampedHp);
	return true;
}

UAbilitySystemComponent* AMGSPlayerState::GetAbilitySystemComponent() const
{
	return GetMGSAbilitySystemComponent();
}
