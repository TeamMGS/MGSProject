/*
 * 파일명 : MGSBlueprintFunctionLibrary.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
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
	// 액터에서 MGSASC(MGSAbilitySystemComponent)를 가져옴
	static UMGSAbilitySystemComponent* NativeGetAbilitySystemComponentFromActor(AActor* Actor);
	
	// 액터에 태그 추가
	UFUNCTION(BlueprintCallable, Category = "FunctionLibrary")
	static void AddTagToActor(AActor* Actor, FGameplayTag Tag);
	
	// 액터에 태그 제거
	UFUNCTION(BlueprintCallable, Category = "FunctionLibrary")
	static void RemoveTagFromActor(AActor* Actor, FGameplayTag Tag);
	
	// 액터에 태그 소유 검사
	static bool NativeActorHasTag(AActor* Actor, FGameplayTag Tag);
	
	// 액터에서 CombatComponent를 가져옴
	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* Actor);
	
};
