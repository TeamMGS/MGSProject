/*
 * 파일명 : MGSBlueprintFunctionLibrary.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "MGSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"

UMGSAbilitySystemComponent* UMGSBlueprintFunctionLibrary::NativeGetAbilitySystemComponentFromActor(AActor* Actor)
{
	check(Actor)
	
	// Cast : 타입이 맞으면 T* 반환 아니면 nullptr 반환
	// CastChecked : 타입이 맞는 걸 전제하고 캐스팅 nullptr/타입 불일치 시 즉시 실패
	return CastChecked<UMGSAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor));
}

void UMGSBlueprintFunctionLibrary::AddTagToActor(AActor* Actor, FGameplayTag Tag)
{
	UMGSAbilitySystemComponent* ASC = NativeGetAbilitySystemComponentFromActor(Actor);
	
	if (!ASC->HasMatchingGameplayTag(Tag))
	{
		// AddLooseGameplayTag : GA/GE 수명에 묶이지 않은, 수동 태그 추가 = 즉시 태그를 켜고, 나중에 코드로 직접 끔
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
	
	// 계층 매칭
	// ex) State.Player.Aiming을 갖고 있으면 State.Player 검사도 true가 될 수 있음
	return ASC->HasMatchingGameplayTag(Tag);
}

UPawnCombatComponent* UMGSBlueprintFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* Actor)
{
	check(Actor);
	
	// PawnCombatInterface를 이용해 CombatComponent를 가져옴
	if (IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(Actor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}
	
	return nullptr;
}
