/*
 * 파일명: BaseBullet.h
 * 생성자: 장대한
 * 생성일: 2026-03-06
 * 수정자: 장대한
 * 수정일: 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/BaseProjectile.h"
#include "BaseBullet.generated.h"

UCLASS()
class MGSPROJECT_API ABaseBullet : public ABaseProjectile
{
	GENERATED_BODY()

public:
	ABaseBullet();
	
};
