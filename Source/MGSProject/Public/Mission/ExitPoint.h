/*
 * 파일명 : ExitPoint.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-25
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitPoint.generated.h"

class USphereComponent;

UCLASS()
class MGSPROJECT_API AExitPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AExitPoint();

protected:
	UFUNCTION()
	void CollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision;

};
