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
