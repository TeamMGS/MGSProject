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
#include "BaseWeapon.generated.h"

struct FGameplayAbilitySpecHandle;
class UBoxComponent;

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBoxComponent> WeaponCollisionBox;
	
public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const { return WeaponCollisionBox; }
	
private:
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

};
