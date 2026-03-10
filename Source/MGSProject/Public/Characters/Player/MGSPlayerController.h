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

struct FInputActionValue;
struct FGameplayTag;
class UDA_InputConfig;
class UPlayerHUDPresenterComponent;
class UMGSPlayerStatusWidget;

UCLASS()
class MGSPROJECT_API AMGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMGSPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetupInputMappingContext() const;
	void BindInputActions();

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDA_InputConfig> InputConfigDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPlayerHUDPresenterComponent> PlayerHUDPresenterComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = true))
	TSubclassOf<UMGSPlayerStatusWidget> PlayerStatusWidgetClass;
};
