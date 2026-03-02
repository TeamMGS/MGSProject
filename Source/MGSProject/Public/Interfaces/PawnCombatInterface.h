/*
 * 파일명 : PawnCombatInterface.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnCombatInterface.generated.h"

class UPawnCombatComponent;

UINTERFACE(MinimalAPI)
class UPawnCombatInterface : public UInterface
{
	GENERATED_BODY()
	
};

class MGSPROJECT_API IPawnCombatInterface
{
	GENERATED_BODY()

public:
	virtual UPawnCombatComponent* GetPawnCombatComponent() const = 0;
	
};
