/*
 * 파일명 : EnemyCharacter.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 김사윤
 * 수정일 : 2026-03-23
 */

#pragma once

#include "AbilitySystemInterface.h"
#include "Characters/BaseCharacter.h"
#include "CoreMinimal.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GameplayTagContainer.h"
#include "EnemyCharacter.generated.h"


class UPaperSpriteComponent;
struct FOnAttributeChangeData;
class UAbilitySystemComponent;
class UAIPerceptionStimuliSourceComponent;
class UCharacterAttributeSet;
class UDA_SpreadSettings;
class UEnemyCombatComponent;
class UMGSAbilitySystemComponent;
class USphereComponent;

UCLASS()
class MGSPROJECT_API AEnemyCharacter : public ABaseCharacter {
  GENERATED_BODY()

public:
  AEnemyCharacter(const FObjectInitializer &ObjectInitializer);

  // Getter
  FORCEINLINE UEnemyCombatComponent *GetEnemyCombatComponent() const {
    return EnemyCombatComponent;
  }

  // IPawnCombatInterface pure virtual function override
  virtual UPawnCombatComponent *GetPawnCombatComponent() const override;
  // IAbilitySystemInterface pure virtual function override
  virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;
  // MGSASC
  virtual UMGSAbilitySystemComponent *
  GetMGSAbilitySystemComponent() const override;
  // Character Attribute
  virtual UCharacterAttributeSet *GetCharacterAttributeSet() const override;
  // Weapon Attribute
  virtual UWeaponAttributeSet *GetWeaponAttributeSet() const override;
  // Spread settings
  const UDA_SpreadSettings *GetSpreadSettings() const;
  // 피격 부위에 따른 데미지 배율 조회
  virtual float GetDamageMultiplierForHit(const FHitResult &Hit) const override;

  // 시야 기준점(AI Perception 등에서 사용)
  virtual void GetActorEyesViewPoint(FVector &OutLocation,
                                     FRotator &OutRotation) const override;

  // Set movement
  // Default
  bool SetDefaultMovementMode();
  // Walk
  bool SetWalkMovementMode();
  // Sprint
  bool SetSprintMovementMode();
  // Crouch
  bool SetCrouchState(bool bShouldCrouch);
  // Ability
  UFUNCTION(BlueprintCallable, Category = "AI|Ability")
  bool ActivateEnemyAbilityByTag(const FGameplayTag &AbilityTag);
  UFUNCTION(BlueprintCallable, Category = "AI|Ability")
  bool CancelEnemyAbilityByTag(const FGameplayTag &AbilityTag);
  // Set Tag
  void SetEnemyStateTagFromAI(const FGameplayTag &NewStateTag);
  // Debug
  void DebugPrintOwnedTags() const;
  // Spread
  void RequestSpreadRefreshNextTick();

protected:
  virtual void BeginPlay() override;
  virtual void PossessedBy(AController *NewController) override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // Init Attribute
  void InitializeEnemyAttributes();
  // Set Tag
  void SetEnemyStateTag(const FGameplayTag &NewStateTag);
  void InitEnemyStartupData();
  bool ApplyEnemyMovementMode(const FGameplayTag &NewMovementStateTag,
                              float NewMaxWalkSpeed);

  // Debug
  // HP
  void BindHpChangedDelegate();
  void
  HandleCurrentHpChanged(const FOnAttributeChangeData &AttributeChangeData);
  // Spread
  UFUNCTION()
  void HandleMovementUpdated(float DeltaSeconds, FVector OldLocation,
                             FVector OldVelocity);
  UFUNCTION()
  void HandleEquippedWeaponChanged(FGameplayTag PreviousWeaponTag,
                                   FGameplayTag CurrentWeaponTag);
  void UpdateCurrentSpreadFromState();
  float CalculateCurrentSpreadStateMultiplier() const;
  // Material
  void ApplyStateMaterial(const FGameplayTag &NewStateTag);
  // Log
  void BindDebugStateInputs();
  void DebugSetStateClear();
  void DebugSetStateSuspicious();
  void DebugSetStateInvestigation();
  void DebugSetStateCombat();

protected:
  // GAS
  // ASC
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UMGSAbilitySystemComponent> MGSAbilitySystemComponent;
  // Character AttributeSet
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;
  // Weapon AttributeSet
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UWeaponAttributeSet> WeaponAttributeSet;
  // Spread settings
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Spread",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UDA_SpreadSettings> SpreadSettingsData;

  // Component
  // Combat
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UEnemyCombatComponent> EnemyCombatComponent;
  // AI
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI",
            meta = (AllowPrivateAccess = true))
  TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliSource;

  // Collision
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<USphereComponent> HeadHitSphere;

  // Minimap Indicator Sprite
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UPaperSpriteComponent> IndicatorSprite;

  // Character AttributeSet
  // Max HP
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attribute",
            meta = (AllowPrivateAccess = true, ClampMin = "1.0"))
  float DefaultMaxHp = 100.0f;
  // Current HP
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attribute",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float DefaultCurrentHp = 100.0f;

  // Tag
  // Default
  UPROPERTY(EditDefaultsOnly, Category = "AI|State")
  FGameplayTag DefaultEnemyStateTag;
  // Current
  UPROPERTY(VisibleAnywhere, Category = "AI|State")
  FGameplayTag CurrentEnemyStateTag;

  // Movement
  // Current movement state tag
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement",
            meta = (AllowPrivateAccess = true))
  FGameplayTag CurrentEnemyMovementStateTag;
  // Default speed
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float DefaultMovementSpeed = 450.0f;
  // Walk speed
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float WalkMovementSpeed = 225.0f;
  // Sprint speed
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float SprintMovementSpeed = 450.0f;
  // Crouch speed
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Movement",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float CrouchMovementSpeed = 180.0f;

  // Head Collision
  // Size
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage",
            meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
  float HeadHitSphereRadius = 18.0f;
  // Damage scale
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage",
            meta = (AllowPrivateAccess = true, ClampMin = "1.0"))
  float HeadshotDamageMultiplier = 2.0f;

  // Debug
  UPROPERTY(EditDefaultsOnly, Category = "AI|State")
  bool bEnableDebugStateInput = false;
  bool bDebugStateInputBound = false;
  FDelegateHandle EquippedWeaponChangedHandle;
  bool bPendingSpreadRefreshRequest = false;
  // HP
  FDelegateHandle CurrentHpChangedDelegateHandle;
  bool bHasBoundHpChangedDelegate = false;
  // Material
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
};
