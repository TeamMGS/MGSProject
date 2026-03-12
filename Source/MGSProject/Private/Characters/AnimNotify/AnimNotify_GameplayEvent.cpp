// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AnimNotify/AnimNotify_GameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

void UAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// 인터페이스를 통해 찾기 (가장 빠름)
	IAbilitySystemInterface* ASInterface = Cast<IAbilitySystemInterface>(Owner);
	UAbilitySystemComponent* ASC = ASInterface ? ASInterface->GetAbilitySystemComponent() : nullptr;

	// 인터페이스가 없다면 컴포넌트 직접 검색
	if (!ASC)
	{
		ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	}

	if (ASC)
	{
		FGameplayEventData Payload;
		Payload.Instigator = Owner;
		ASC->HandleGameplayEvent(EventTag, &Payload);
	}
	else
	{
		// 디버깅용 로그
		UE_LOG(LogTemp, Error, TEXT("Could not find ASC on %s"), *Owner->GetName());
	}
}
