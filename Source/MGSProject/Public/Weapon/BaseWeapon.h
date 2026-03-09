/*
 * 파일명 : BaseWeapon.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpecHandle.h"
#include "MGSStructType.h"
#include "BaseWeapon.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class MGSPROJECT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ABaseWeapon();

	// 무기에 GA SpecHandle 목록 적재
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles);

	// 내부에 저장된 핸들을 꺼내고 비웁니다.
	void ConsumeGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>& OutSpecHandles);
	
	// 장착 시 부여할 무기 어빌리티/입력 컨텍스트 데이터
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FPlayerWeaponData& GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetEquippedSocketName() const { return EquippedSocketName; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetHolsterSocketName() const { return HolsterSocketName; }

protected:
	// 무기 데이터(IMC, UI 이미지, Tag-GA)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FPlayerWeaponData WeaponData;

	// 무기 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// 무기 콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBoxComponent> WeaponCollisionBox;
	
	// 장착 소켓 (캐릭터 메시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName EquippedSocketName = NAME_None;

	// 보관 소켓 (캐릭터 메시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName HolsterSocketName = NAME_None;

private:
	// 무기에 부여된 GA SpecHandle들
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
	
};
