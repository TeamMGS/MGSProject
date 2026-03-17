/*
 * 파일명 : DA_InputConfig.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DA_InputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

// Tag-IA 데이터
USTRUCT(BlueprintType)
struct FInputActionConfig
{
	GENERATED_BODY()
	
public:
	// Tag
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
	
	// Native input: 직접 함수 호출
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FInputActionConfig> NativeInputActions;
	
	// Ability input: Pressed/Released 함수로 태그를 ASC에 전달
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FInputActionConfig> AbilityInputActions;
	
	// Tag로 Native input IA 탐색
	UInputAction* FindNativeInputActionByTag(const FGameplayTag& InInputTag) const;
	
};
