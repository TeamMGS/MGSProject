/*
 * 파일명 : AICoreEnemyCharacter.cpp
 * 생성자 : 김사윤
 * 생성일 : 2026-03-05
 * 수정자 : 김사윤
 * 수정일 : 2026-03-05
 */
#include "AI/Core/AICoreEnemyCharacter.h"

#include "AbilitySystemComponent.h"

AAICoreEnemyCharacter::AAICoreEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AAICoreEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
