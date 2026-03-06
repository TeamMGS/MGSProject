/*
 * 파일명 : BaseCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PawnCombatInterface.h"
#include "BaseCharacter.generated.h"

class UMGSLocomotionComponent;
class UMGSCharacterMovementComponent;
class UCharacterAttributeSet;
class UWeaponAttributeSet;
class UMGSAbilitySystemComponent;
class UDA_StartupBase;
class AMGSPlayerState;

UCLASS()
class MGSPROJECT_API ABaseCharacter : public ACharacter, public IPawnCombatInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;

protected:
	virtual void PossessedBy(AController* NewController) override;

public:	
	virtual UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const;
	virtual UCharacterAttributeSet* GetCharacterAttributeSet() const;
	virtual UWeaponAttributeSet* GetWeaponAttributeSet() const;

protected:
	AMGSPlayerState* GetMGSPlayerState() const;
	
	virtual void OnJumped_Implementation() override;
	
	virtual void Landed(const FHitResult& Hit) override;
	
protected:
	// DA_StartupBase : 초기 부여 어빌리티 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDA_StartupBase> StartupData;
	
	// 현재의 상태(state)를 정의하는 TAG를 붙이기 위한 Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMGSLocomotionComponent> LocomotionComponent;

	// 캐싱된 커스텀 무브먼트 컴포넌트 포인터 (캐스팅 오버헤드 방지)
	UPROPERTY()
	TObjectPtr<UMGSCharacterMovementComponent> MGSMovementComponent;
};
