/*
 * 파일명 : MGSStructType.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#pragma once

#include "GameplayTagContainer.h"
#include "MGSStructType.generated.h"

class UPlayerGameplayAbility;
class UInputMappingContext;
class UTexture2D;

// 플레이어 Tag-GA 데이터
USTRUCT(BlueprintType)
struct FPlayerAbilitySet
{
	GENERATED_BODY()
	
	// 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	// 부여할 플레이어 어빌리티 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UPlayerGameplayAbility> AbilityToGrant;
	
	bool IsValid() const;
	
};

// 무기 IMC, UI 이미지, 플레이어 Tag-GA 데이터
USTRUCT(BlueprintType)
struct FPlayerWeaponData
{
	GENERATED_BODY()
	
	// 무기 IMC
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> WeaponInputMappingContext;

	// HUD 무기 정보 패널에서 사용할 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> WeaponInfoImage = nullptr;

	// HUD/프롬프트에서 사용할 무기 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText WeaponDisplayName;
	
	// 무기 장착 시 부여할 어빌리티 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FPlayerAbilitySet> WeaponAbilities;
	
	// 무기 장착시 사용할 애니메이션 레이어
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> WeaponAnimLayer;
	
	// 이 무기가 어떤 종류인지 나타내는 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	FGameplayTag WeaponEquippedTag;
	
	// 무기 장전 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;
	
	// 무기 발사 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMontage;
	
	// 무기 장착 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;
	
	// 오른손 기준 왼손이 잡아야 할 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FVector LeftHandIKOffset;
};

// 런타임 중 무기 탄약/탄창 데이터
USTRUCT(BlueprintType)
struct FWeaponRuntimeState
{
	GENERATED_BODY()

	// 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentMagazineAmmo = 0;

	// 탄창
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentCarriedAmmo = 0;
	
};
