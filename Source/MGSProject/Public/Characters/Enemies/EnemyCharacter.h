/*
 * 파일명 : EnemyCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "EnemyCharacter.generated.h"

class UEnemyCombatComponent;
class UCharacterAttributeSet;
class UMGSAbilitySystemComponent;

UCLASS()
class MGSPROJECT_API AEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	AEnemyCharacter();

	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	virtual UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const override;
	virtual UCharacterAttributeSet* GetCharacterAttributeSet() const override;
protected:
	virtual void PossessedBy(AController* NewController) override;
	
private:
	void InitEnemyStartupData();
	
protected:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMGSAbilitySystemComponent> MGSAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;

protected:
	TObjectPtr<UEnemyCombatComponent> EnemyCombatComponent;
	
public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComponent; }
	
};


