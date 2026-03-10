/*
 * 파일명 : DA_InputConfig.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-04
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DA_InputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

USTRUCT(BlueprintType)
struct FInputActionConfig
{
	GENERATED_BODY()
	
public:
	// 입력 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category = "InputTag"))
	FGameplayTag InputTag;
	
	// IA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction;
	
	bool IsValid() const
	{
		return InputTag.IsValid() && InputAction;
	}
	
};

UCLASS()
class MGSPROJECT_API UDA_InputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// ICM
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	// 네이티브 입력 배열 : 직접 함수 호출
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FInputActionConfig> NativeInputActions;
	
	// 어빌리티 입력 배열 : Pressed/Released를 ASC로 전달
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FInputActionConfig> AbilityInputActions;
	
	// 입력 태그로 Native 입력 액션을 찾아 반환한다.
	UInputAction* FindNativeInputActionByTag(const FGameplayTag& InInputTag) const;
	
};
