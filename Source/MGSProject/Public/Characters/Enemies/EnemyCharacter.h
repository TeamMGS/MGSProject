/*
 * 파일명 : EnemyCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 김사윤
 * 수정일 : 2026-03-06
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/BaseCharacter.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
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
	virtual void BeginPlay() override;
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

private:
	void SetEnemyStateTag(const FGameplayTag& NewStateTag);
	void ApplyStateMaterial(const FGameplayTag& NewStateTag);
	void BindDebugStateInputs();

	void DebugSetStateClear();
	void DebugSetStateSuspicious();
	void DebugSetStateInvestigation();
	void DebugSetStateCombat();

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	FGameplayTag DefaultEnemyStateTag;

	UPROPERTY(VisibleAnywhere, Category = "AI|State")
	FGameplayTag CurrentEnemyStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	bool bEnableDebugStateInput = false;

	bool bDebugStateInputBound = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State", meta = (ClampMin = 0))
	int32 StateMaterialSlotIndex = 0;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	TObjectPtr<UMaterialInterface> ClearStateMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	TObjectPtr<UMaterialInterface> SuspiciousStateMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	TObjectPtr<UMaterialInterface> InvestigationStateMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "AI|State")
	TObjectPtr<UMaterialInterface> CombatStateMaterial;
	
public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComponent; }
	FORCEINLINE float GetCurrentHp() const
	{
		return CharacterAttributeSet ? CharacterAttributeSet->GetCurrentHp() : 0.0f;
	}

	FORCEINLINE float GetMaxHp() const
	{
		return CharacterAttributeSet ? CharacterAttributeSet->GetMaxHp() : 0.0f;
	}
	
};
