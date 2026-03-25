/*
 * 파일명 : MGSGameMode.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-25
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGSGameMode.generated.h"

class AMapCaptureActor;

UCLASS()
class MGSPROJECT_API AMGSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGSGameMode();
	
	// Getter
	AMapCaptureActor* GetMapCaptureActor() const { return MapCaptureActor.Get(); }

	// Mission Complete
	UFUNCTION(BlueprintCallable, Category = "Game")
	void MissionComplete() const;
	
protected:
	virtual void BeginPlay() override;
	
private:
	// MapCapture
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Map", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AMapCaptureActor> MapCaptureActor;
	
};
