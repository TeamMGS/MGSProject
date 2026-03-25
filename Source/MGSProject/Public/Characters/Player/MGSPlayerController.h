/*
 * 파일명 : MGSPlayerController.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-17
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MGSEnumType.h"
#include "MGSPlayerController.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class UDA_InputConfig;
class UMGSPlayerStatusWidget;
class UPlayerHUDPresenterComponent;
class AEnemyCharacter;
class USoundCue;
class UAudioComponent;

UCLASS()
class MGSPROJECT_API AMGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMGSPlayerController();

	UFUNCTION(Exec)
	void ActivateEnemyAbility(const FString& AbilityTagString);

	UFUNCTION(Exec)
	void ActivateEnemyAbilityOn(const FString& EnemyName, const FString& AbilityTagString);
	
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundCue* BeginPlayBGM;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundCue* ContainedBGM;
	
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ChangeBGM(class USoundCue* NewBGMCue);
	
	UFUNCTION(Exec)
	void SetPlayerHp(float NewCurrentHp);
	
	UFUNCTION()
	void HandleNarrationFinished(ENarrationSituation FinishedSituation);

protected:
	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

private:
	// IMC Setting
	void SetupInputMappingContext() const;
	// Bind Native, Ability input
	void BindInputActions();

	// Native input
	// Move
	void Input_Move(const FInputActionValue& InputActionValue);
	// Look
	void Input_Look(const FInputActionValue& InputActionValue);
	// 무기 장착 해제
	void Input_UnequipWeapons();
	// Map
	void Input_Map();
	
	// Ability input
	// Pressed
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	// Released
	void Input_AbilityInputReleased(FGameplayTag InputTag);

	AEnemyCharacter* FindNearestEnemyCharacter() const;
	AEnemyCharacter* FindEnemyCharacterByName(const FString& EnemyName) const;
	void ExecuteEnemyAbilityCommand(AEnemyCharacter* TargetEnemy, const FString& AbilityTagString);

private:
	// DA_InputConfig : Native, Ability input 목록(Tag-IA 매핑) 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDA_InputConfig> InputConfigDataAsset;

	// Player HUD component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPlayerHUDPresenterComponent> PlayerHUDPresenter;

	// Player HUD widget class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TSubclassOf<UMGSPlayerStatusWidget> PlayerStatusWidgetClass;
	
	// 재생 중인 사운드를 제어하기 위한 컴포넌트
	UPROPERTY()
	UAudioComponent* StartupAudioComponent;
	
	UPROPERTY()
	UAudioComponent* CurrentBGMComponent;

	// 사운드를 정지시킬 함수 (타이머에서 호출해야 하므로 UFUNCTION 필요)
	UFUNCTION()
	void StopStartupSound();

	// 타이머 핸들
	FTimerHandle SoundStopTimerHandle;
};
