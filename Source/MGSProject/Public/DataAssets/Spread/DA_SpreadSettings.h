/*
 * 파일명 : DA_SpreadSettings.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-11
 * 수정자 : 장대한
 * 수정일 : 2026-03-11
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_SpreadSettings.generated.h"

UCLASS(BlueprintType)
class MGSPROJECT_API UDA_SpreadSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	// Jump
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float JumpMultiplier = 2.4f;

	// Sprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float SprintMultiplier = 1.6f;

	// Move
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float MovingMultiplier = 1.3f;

	// Walk
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float WalkMultiplier = 1.1f;

	// Crouch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float CrouchStillMultiplier = 0.7f;

	// Aim
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AimMultiplier = 0.65f;

	// 정지-이동 상태 판단 수평 속도 임계값(MovementComponent의 HorizontalSpeed와 비교)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float MovingSpeedThreshold = 10.0f;
	
};
