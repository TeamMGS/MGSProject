/*
 * 파일명 : MGSPlayerState.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "Characters/Player/MGSPlayerState.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"

AMGSPlayerState::AMGSPlayerState()
{
	MGSAbilitySystemComponent = CreateDefaultSubobject<UMGSAbilitySystemComponent>(TEXT("MGSAbilitySystemComponent"));
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
}

void AMGSPlayerState::InitASC(AActor* Avatar)
{
	if (MGSAbilitySystemComponent)
	{
		// AbilitySystemComponent에 접근 어빌리티 정보 초기값 부여
		MGSAbilitySystemComponent->InitAbilityActorInfo(this, Avatar);
	}
}

UAbilitySystemComponent* AMGSPlayerState::GetAbilitySystemComponent() const
{
	return GetMGSAbilitySystemComponent();
}
