/*
 * 파일명 : BaseWeapon.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "Weapon/BaseWeapon.h"

#include "Components/BoxComponent.h"

ABaseWeapon::ABaseWeapon()
{
	// 틱관련 기능 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//WeaponCollisionBox->InitBoxExtent(FVector());
}

void ABaseWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles)
{
	GrantedAbilitySpecHandles = SpecHandles;
}

TArray<FGameplayAbilitySpecHandle> ABaseWeapon::GetGrantedAbilitySpecHandles()
{
	return GrantedAbilitySpecHandles;
}
