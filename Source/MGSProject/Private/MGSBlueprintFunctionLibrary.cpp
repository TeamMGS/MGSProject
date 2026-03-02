/*
 * 파일명 : MGSBlueprintFunctionLibrary.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "MGSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"

UMGSAbilitySystemComponent* UMGSBlueprintFunctionLibrary::NativeGetAbilitySystemComponentFromActor(AActor* Actor)
{
	check(Actor)
	
	return CastChecked<UMGSAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor));
}

void UMGSBlueprintFunctionLibrary::AddTagToActor(AActor* Actor, FGameplayTag Tag)
{
	UMGSAbilitySystemComponent* ASC = NativeGetAbilitySystemComponentFromActor(Actor);
	
	if (!ASC->HasMatchingGameplayTag(Tag))
	{
		ASC->AddLooseGameplayTag(Tag);
	}
}

void UMGSBlueprintFunctionLibrary::RemoveTagFromActor(AActor* Actor, FGameplayTag Tag)
{
	UMGSAbilitySystemComponent* ASC = NativeGetAbilitySystemComponentFromActor(Actor);
	
	if (ASC->HasMatchingGameplayTag(Tag))
	{
		ASC->RemoveLooseGameplayTag(Tag);
	}
}

bool UMGSBlueprintFunctionLibrary::NativeActorHasTag(AActor* Actor, FGameplayTag Tag)
{
	UMGSAbilitySystemComponent* ASC = NativeGetAbilitySystemComponentFromActor(Actor);
	
	return ASC->HasMatchingGameplayTag(Tag);
}

UPawnCombatComponent* UMGSBlueprintFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* Actor)
{
	check(Actor);
	
	if (IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(Actor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}
	
	return nullptr;
}
