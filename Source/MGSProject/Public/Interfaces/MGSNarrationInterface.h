/*
 * 파일명 : MGSNarrationInterface.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-23
 * 수정자 : 김동석
 * 수정일 : 2026-03-23
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MGSNarrationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMGSNarrationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MGSPROJECT_API IMGSNarrationInterface
{
	GENERATED_BODY()
public:
	// 나레이션 텍스트 업데이트 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Narration")
	void UpdateNarrationText(const FText& NewText);
};
