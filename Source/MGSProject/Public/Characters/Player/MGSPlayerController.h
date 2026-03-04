/*
 * 파일명 : MGSPlayerController.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MGSPlayerController.generated.h"

struct FInputActionValue;
struct FGameplayTag;
class UDA_InputConfig;

UCLASS()
class MGSPROJECT_API AMGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void SetupInputMappingContext() const; // 기본 IMC 추가
	void BindInputActions(); // 네이티브 입력과 어빌리티 입력 바인딩

	// 네이티브 입력
	void Input_Move(const FInputActionValue& InputActionValue); // 이동
	void Input_Look(const FInputActionValue& InputActionValue); // 시점

	// 어빌리티 입력
	void Input_AbilityInputPressed(FGameplayTag InputTag); // 눌렀을 때
	void Input_AbilityInputReleased(FGameplayTag InputTag); // 땠을 때

private:
	// DA_InputConfig : 태그-AI 연결 데이터 에셋 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDA_InputConfig> InputConfigDataAsset;
	
};
