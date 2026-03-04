/*
 * 파일명 : BaseCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PawnCombatInterface.h"
#include "BaseCharacter.generated.h"

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
	ABaseCharacter();
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;

protected:
	virtual void PossessedBy(AController* NewController) override;

public:	
	virtual UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const;
	virtual UCharacterAttributeSet* GetCharacterAttributeSet() const;
	virtual UWeaponAttributeSet* GetWeaponAttributeSet() const;

protected:
	AMGSPlayerState* GetMGSPlayerState() const;
	
protected:
	// DA_StartupBase : 초기 부여 어빌리티 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDA_StartupBase> StartupData;

};


