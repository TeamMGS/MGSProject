/*
 * 파일명 : MGSPlayerState.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "MGSPlayerState.generated.h"

class UCharacterAttributeSet;
class UMGSAbilitySystemComponent;
class UWeaponAttributeSet;

UCLASS()
class MGSPROJECT_API AMGSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AMGSPlayerState();
	
	// ASC 초기화
	void InitASC(AActor* Avatar);
	
	// Getter
	FORCEINLINE UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const { return MGSAbilitySystemComponent; }
	FORCEINLINE UCharacterAttributeSet* GetCharacterAttributeSet() const { return CharacterAttributeSet; }
	FORCEINLINE UWeaponAttributeSet* GetWeaponAttributeSet() const { return WeaponAttributeSet; }
	
protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
private:
	// ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMGSAbilitySystemComponent> MGSAbilitySystemComponent;
	
	// Attribute
	// Character Attribute
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;
	// Weapon Attribute
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWeaponAttributeSet> WeaponAttributeSet;
	
};
