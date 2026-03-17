/*
 * 파일명 : MGSDamageStatics.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-16
 * 수정자 : 장대한
 * 수정일 : 2026-03-16
 */

#pragma once

#include "CoreMinimal.h"

struct FHitResult;
struct FMGSProjectileAttackPayload;
class AActor;
class APawn;

struct MGSPROJECT_API FMGSDamageStatics
{
	// Projectile 데미지 처리
	static bool ApplyProjectileDamage(
		const FMGSProjectileAttackPayload& AttackPayload,
		AActor* EffectCauser,
		APawn* InstigatorPawn,
		AActor* TargetActor,
		const FHitResult& Hit);
};
