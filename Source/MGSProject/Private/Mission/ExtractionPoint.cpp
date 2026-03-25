/*
 * 파일명 : ExtractionPoint.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-20
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */
#include "Mission/ExtractionPoint.h"
#include "MGSEnumType.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/UI/PlayerHUDPresenterComponent.h"

AExtractionPoint::AExtractionPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	RootComponent = Trigger;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	// 처음에는 비활성화 (시각적/충돌적 처리)
	SetActorHiddenInGame(true);
	Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExtractionPoint::ActivateExtraction()
{
	bIsActive = true;
	SetActorHiddenInGame(false);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 디버그
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Extraction Point Active! Get out now!"));
}

void AExtractionPoint::NotifyActorBeginOverlap(AActor* OtherActor)
{
	// 1. 활성화 상태인지 확인
	if (!bIsActive) return;

	// 2. 부딪힌 액터가 플레이어인지 클래스로 직접 확인
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (AMGSPlayerController* MyPC = Cast<AMGSPlayerController>(Player->GetController()))
		{
			if (UPlayerHUDPresenterComponent* HUDPresenter = MyPC->FindComponentByClass<UPlayerHUDPresenterComponent>())
			{
				HUDPresenter->PlayNarration(ENarrationSituation::ExtractionSuccess);
			}
		}
	}
}


