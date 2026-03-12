/*
 * 파일명 : MGSInputComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAssets/Input/DA_InputConfig.h"
#include "MGSInputComponent.generated.h"

UCLASS()
class MGSPROJECT_API UMGSInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	// Bind native input function: 함수 직접 호출
	template<class UserObject, typename CallbackFunc>
	void BindNativeInputAction(const UDA_InputConfig* InInputConfig, const FGameplayTag& InInputTag, ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func);
	
	// Bind ability input function: Tag를 전달하여 ASC에서 Ability 활성화
	template<class UserObject, typename CallbackFunc>
	void BindAbilityInputAction(const UDA_InputConfig* InInputConfig, UserObject* ContextObject, CallbackFunc InputPressedFunc, CallbackFunc InputReleasedFunc);
	
};

template <class UserObject, typename CallbackFunc>
void UMGSInputComponent::BindNativeInputAction(const UDA_InputConfig* InInputConfig, const FGameplayTag& InInputTag,
	ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func)
{
	// UA_InputConfig 데이터 에셋 유효성 확인
	checkf(InInputConfig, TEXT("Input config data asset is null can not proceed with binding"));

	if (!InInputTag.IsValid())
	{
		return;
	}
	
	// Tag로 IA찾고 함수 연결
	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		BindAction(FoundAction, TriggerEvent, ContextObject, Func);
	}
}

template <class UserObject, typename CallbackFunc>
void UMGSInputComponent::BindAbilityInputAction(const UDA_InputConfig* InInputConfig, UserObject* ContextObject,
	CallbackFunc InputPressedFunc, CallbackFunc InputReleasedFunc)
{
	// UA_InputConfig 데이터 에셋 유효성 확인
	checkf(InInputConfig, TEXT("Input config data asset is null can not proceed with binding"));
	
	// Ability input에 사용할 Tag-IA 목록 순회
	for (const FInputActionConfig& AbilityInputActionConfig : InInputConfig->AbilityInputActions)
	{
		if (!AbilityInputActionConfig.IsValid())
		{
			continue;
		}

		if (!AbilityInputActionConfig.InputTag.ToString().StartsWith(TEXT("InputTag.")))
		{
			continue;
		}
		
		// IA에 Pressed/Released 함수(Tag전달) 연결
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Started, ContextObject, InputPressedFunc, AbilityInputActionConfig.InputTag);
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Completed, ContextObject, InputReleasedFunc, AbilityInputActionConfig.InputTag);
	}
}
