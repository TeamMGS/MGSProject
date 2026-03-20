/*
 * 파일명 : MGSInteractableInterface.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-20
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MGSInteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMGSInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MGSPROJECT_API IMGSInteractableInterface
{
	GENERATED_BODY()

public:
	// 상호작용 시작 (게이지 시작 등)
	virtual void OnInteractionStarted(AActor* Interactor) = 0;

	// 상호작용 성공 (2초 완료)
	virtual void OnInteractionSucceeded(AActor* Interactor) = 0;

	// 상호작용 중단 (키를 뗌)
	virtual void OnInteractionCanceled(AActor* Interactor) = 0;

	// 필요한 시간 (기본 2초)
	virtual float GetInteractionDuration() const { return 2.0f; }
};
