/*
 * 파일명 : PlayerCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#include "Characters/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"

APlayerCharacter::APlayerCharacter()
{
	// 회전 사용 비활성화
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	// 스프링 암 초기 세팅
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	
	// 카메라 초기 세팅
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	// 캐릭터 움직임 초기 세팅
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!StartupData.IsNull())
	{
		if (UDA_StartupBase* LoadedData = StartupData.LoadSynchronous())
		{
			// Startup 데이터가 Null인 아닌경우 Startup 데이터는 동기화로드를 거쳐서 최종적으로 게임 어빌리티 시스템이 발동된다.
			LoadedData->GiveToAbilitySystemComponent(GetMGSAbilitySystemComponent());
		}
	}
	
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	
}

void APlayerCharacter::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	GetMGSAbilitySystemComponent()->OnAbilityInputPressed(InputTag);
}

void APlayerCharacter::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	GetMGSAbilitySystemComponent()->OnAbilityInputReleased(InputTag);
}
