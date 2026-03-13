/*
 * 파일명 : BaseCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PawnCombatInterface.h"
#include "AbilitySystemInterface.h"
#include "BaseCharacter.generated.h"

class UMGSLocomotionComponent;
class UMGSCharacterMovementComponent;
class UCharacterAttributeSet;
class UWeaponAttributeSet;
class UMGSAbilitySystemComponent;
class UDA_StartupBase;
class AMGSPlayerState;

UCLASS()
class MGSPROJECT_API ABaseCharacter : public ACharacter, public IPawnCombatInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);
	
	// IPawnCombatInterface pure virtual function override
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	// GameFramework getter
	// Get PlayerState
	AMGSPlayerState* GetMGSPlayerState() const;
	// GAS getter
	// Get MGSAbilitySystemComponent
	virtual UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const;
	// Get Character AttributeSet
	virtual UCharacterAttributeSet* GetCharacterAttributeSet() const;
	// Get Weapon AttributeSet
	virtual UWeaponAttributeSet* GetWeaponAttributeSet() const;
	// 어빌리티 시스템 인터페이스 전용
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	// Movement virtual function override
	// Landed
	virtual void Landed(const FHitResult& Hit) override;
	// Jump
	virtual void OnJumped_Implementation() override;
	// Crouch
	virtual void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	virtual void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
	
	// DA_StartupBase(캐릭터가 생성될 때 ASC에 부여할 GA/GE을 정의하는 데이터 에셋)
	// TSoftObjectPtr : 실제 객체가 아니라 경로만 참조 -> 필요할 때 로딩(지연 로딩) -> 메모리 절약 + 초기 로딩 시간 감소 + Dependency chain 해소
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDA_StartupBase> StartupData;
	
	// 현재의 상태(state)를 정의하는 TAG를 붙이기 위한 Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMGSLocomotionComponent> LocomotionComponent;

	// 캐싱된 커스텀 무브먼트 컴포넌트 포인터 (캐스팅 오버헤드 방지)
	UPROPERTY()
	TObjectPtr<UMGSCharacterMovementComponent> MGSMovementComponent;
	
};
