/*
 * 파일명 : MGSBlueprintFunctionLibrary.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MGSBlueprintFunctionLibrary.generated.h"

class UPawnCombatComponent;
class UMGSAbilitySystemComponent;

UCLASS()
class MGSPROJECT_API UMGSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UMGSAbilitySystemComponent* NativeGetAbilitySystemComponentFromActor(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category = "FunctionLibrary")
	static void AddTagToActor(AActor* Actor, FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable, Category = "FunctionLibrary")
	static void RemoveTagFromActor(AActor* Actor, FGameplayTag Tag);
	
	static bool NativeActorHasTag(AActor* Actor, FGameplayTag Tag);
	
	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* Actor);
};
