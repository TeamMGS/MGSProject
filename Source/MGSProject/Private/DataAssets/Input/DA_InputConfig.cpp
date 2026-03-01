/*
 * 파일명 : DA_InputConfig.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#include "DataAssets/Input/DA_InputConfig.h"

UInputAction* UDA_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
	for (const FInputActionConfig& InputActionConfig : NativeInputActions)
	{
		return InputActionConfig.InputAction;
	}
	
	return nullptr;
}
