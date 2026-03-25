/*
 * 파일명 : ExitPoint.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-25
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */


#include "Mission/ExitPoint.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/MGSGameMode.h"

AExitPoint::AExitPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetGenerateOverlapEvents(true);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::CollisionBeginOverlap);
}

void AExitPoint::CollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMGSGameMode* GameMode = Cast<AMGSGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (GameMode->AreAllTargetsCollected())
		{
			GameMode->MissionComplete();
		}
		else
		{
		}
	}
}
