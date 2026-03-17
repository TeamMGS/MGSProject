/*
 * 파일명 : BaseWeapon.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-10
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpecHandle.h"
#include "MGSStructType.h"
#include "BaseWeapon.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class APlayerCharacter;

UCLASS()
class MGSPROJECT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ABaseWeapon();
	// 무기가 레벨에 떨어져 있을 때
	void SetAsWorldDroppedWeapon();
	// 무기가 캐릭터에게 장비되어 있을 때
	void SetAsEquippedOrHolsteredWeapon();

	// 무기에 GA SpecHandle 목록 적재
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles);

	// 내부에 저장된 핸들을 꺼내고 비웁니다.
	void ConsumeGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>& OutSpecHandles);

	// 월드로 드랍되기 직전 무기 런타임 상태를 저장합니다.
	void SaveDroppedRuntimeState(const FWeaponRuntimeState& RuntimeState);

	// 드랍 무기에서 런타임 상태를 꺼내고 비웁니다.
	bool ConsumeDroppedRuntimeState(FWeaponRuntimeState& OutRuntimeState);
	
	// 장착 시 부여할 무기 어빌리티/입력 컨텍스트 데이터
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FPlayerWeaponData& GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetEquippedSocketName() const { return EquippedSocketName; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetHolsterSocketName() const { return HolsterSocketName; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FGameplayTag GetWeaponTag() const { return WeaponTag; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	float GetPickupDetectionDebugRadius() const;

protected:
	virtual void BeginPlay() override;

	// 무기 데이터(IMC, UI 이미지, Tag-GA)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FPlayerWeaponData WeaponData;

	// 플레이어 인벤토리 슬롯(Primary/Secondary 등)에 매핑할 무기 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (Categories = "Weapon"))
	FGameplayTag WeaponTag;

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
	UFUNCTION()
	void HandleWeaponOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// 무기에 부여된 GA SpecHandle들
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

	// 월드 드랍 상태일 때 보존할 런타임 무기 상태(탄약/탄창)
	bool bHasDroppedRuntimeState = false;
	FWeaponRuntimeState DroppedRuntimeState;
	
};
