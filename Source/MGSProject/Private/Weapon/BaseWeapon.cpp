/*
 * 파일명 : BaseWeapon.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "Weapon/BaseWeapon.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ABaseWeapon::ABaseWeapon()
{
	// 성능을 위해 Tick을 사용하지 않습니다.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles)
{
	GrantedAbilitySpecHandles = SpecHandles;
}

void ABaseWeapon::ConsumeGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>& OutSpecHandles)
{
	// MoveTemp : rvalue로 캐스팅 후 이동 대입을 타게 함 (내부 버퍼를 복사 없이 넘겨 성능 향상)
	OutSpecHandles = MoveTemp(GrantedAbilitySpecHandles);
	GrantedAbilitySpecHandles.Reset();
}
