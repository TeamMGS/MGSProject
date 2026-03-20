/*
 * 파일명 : ExtractionPoint.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-20
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExtractionPoint.generated.h"

class USphereComponent;

UCLASS()
class MGSPROJECT_API AExtractionPoint : public AActor
{
	GENERATED_BODY()
	
public:
	AExtractionPoint();

	// 미션 성공 시 외부에서 호출해줄 함수
	void ActivateExtraction();

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	bool bIsActive = false;
};
