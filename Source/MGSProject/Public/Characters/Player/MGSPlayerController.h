/*
 * 파일명 : MGSPlayerController.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-05
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MGSPlayerController.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class UDA_InputConfig;
class UMGSPlayerStatusWidget;
class UPlayerHUDPresenterComponent;

UCLASS()
class MGSPROJECT_API AMGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMGSPlayerController();

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
	
	// Ability input
	// Pressed
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	// Released
	void Input_AbilityInputReleased(FGameplayTag InputTag);

private:
	// DA_InputConfig : Native, Ability input 목록(Tag-IA 매핑) 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDA_InputConfig> InputConfigDataAsset;

	// Player HUD component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPlayerHUDPresenterComponent> PlayerHUDPresenterComponent;

	// Player HUD widget class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TSubclassOf<UMGSPlayerStatusWidget> PlayerStatusWidgetClass;
	
};
