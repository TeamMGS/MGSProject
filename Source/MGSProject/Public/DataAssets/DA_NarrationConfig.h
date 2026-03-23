/*
 * 파일명 : DA_NarrationConfig.h
 * 생성자 : Gemini CLI
 * 생성일 : 2026-03-23
 * 설명 : 상황별 나레이션 정보를 관리하는 데이터 자산
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGSEnumType.h"
#include "MGSStructType.h"
#include "DA_NarrationConfig.generated.h"

UCLASS(BlueprintType)
class MGSPROJECT_API UDA_NarrationConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 상황별 나레이션 정보 (Enum -> Struct)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration")
	TMap<ENarrationSituation, FNarrationInfo> NarrationMap;
};
