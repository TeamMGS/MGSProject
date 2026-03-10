/*
 * 파일명 : MGSDamageGameplayEffect.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-09
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MGSDamageGameplayEffect.generated.h"

// SetByCaller(Data.Damage) 값을 CurrentHp에 Additive로 적용하는 즉시 데미지 GE
UCLASS()
class MGSPROJECT_API UMGSDamageGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMGSDamageGameplayEffect();
	
};
