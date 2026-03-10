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
class UWeaponAttributeSet;
class UMGSAbilitySystemComponent;

UCLASS()
class MGSPROJECT_API AMGSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AMGSPlayerState();
	
	void InitASC(AActor* Avatar); // ASC 초기화
	
protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMGSAbilitySystemComponent> MGSAbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWeaponAttributeSet> WeaponAttributeSet;
	
public:
	FORCEINLINE UMGSAbilitySystemComponent* GetMGSAbilitySystemComponent() const { return MGSAbilitySystemComponent; }
	FORCEINLINE UCharacterAttributeSet* GetCharacterAttributeSet() const { return CharacterAttributeSet; }
	FORCEINLINE UWeaponAttributeSet* GetWeaponAttributeSet() const { return WeaponAttributeSet; }
	
};
