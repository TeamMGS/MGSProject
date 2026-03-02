/*
 * 파일명 : BaseCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PawnCombatInterface.h"
#include "BaseCharacter.generated.h"

class UCharacterAttributeSet;
class UMGSAbilitySystemComponent;
class UDA_StartupBase;

UCLASS()
class MGSPROJECT_API ABaseCharacter : public ACharacter, public IPawnCombatInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:	
	UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const;
	UCharacterAttributeSet* GetCharacterAttributeSet() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDA_StartupBase> StartupData;

};
