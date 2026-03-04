/*
 * 파일명 : BaseWeapon.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
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

	UFUNCTION(BlueprintCallable)
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles);

	UFUNCTION(BlueprintPure)
	TArray<FGameplayAbilitySpecHandle> GetGrantedAbilitySpecHandles();

	// ASC에서 제거 후 핸들 배열을 비우기 위해 참조로 제공합니다.
	TArray<FGameplayAbilitySpecHandle>& GetGrantedAbilitySpecHandlesMutable() { return GrantedAbilitySpecHandles; }

	// 장착 시 부여할 무기 어빌리티/입력 컨텍스트 데이터
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FPlayerWeaponData& GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetEquippedSocketName() const { return EquippedSocketName; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetHolsterSocketName() const { return HolsterSocketName; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FPlayerWeaponData WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName EquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName HolsterSocketName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBoxComponent> WeaponCollisionBox;

public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const { return WeaponCollisionBox; }

private:
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
	
};


