/*
 * 파일명 : PlayerAimGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/Player/PlayerAimGameplayAbility.h"

#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseGun.h"

UPlayerAimGameplayAbility::UPlayerAimGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Aim);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
	ActivationOwnedTags.AddTag(MGSGameplayTags::State_Player_Aiming);
}

bool UPlayerAimGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UPlayerCombatComponent* PlayerCombatComponent = Cast<UPlayerCombatComponent>(GetPawnCombatComponentFromActorInfo());
	if (!PlayerCombatComponent)
	{
		return false;
	}

	// 장착한 무기가 없으면 발동하지 않음
	const ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	return EquippedGun != nullptr;
}

void UPlayerAimGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = PlayerCombatComponent ? Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon()) : nullptr;
	AMGSPlayerController* PlayerController = GetMGSPlayerControllerFromActorInfo();
	if (!PlayerController || !PlayerController->PlayerCameraManager || !EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// FOV 캐싱 및 조준 FOV 변경
	CachedFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
	bHasCachedFOV = true;
	PlayerController->PlayerCameraManager->SetFOV(EquippedGun->GetAimFOV());

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		if (USpringArmComponent* CameraBoom = PlayerCharacter->GetCameraBoom())
		{
			// 카메라 오프셋 캐싱 및 조준 오프셋 변경
			CachedCameraSocketOffset = CameraBoom->SocketOffset;
			bHasCachedCameraSocketOffset = true;
			CameraBoom->SocketOffset = EquippedGun->GetAimCameraSocketOffset();

			// 조준 중에는 벽 근처에서 카메라가 급격히 앞으로 당겨지지 않도록 충돌 테스트 비활성화
			bCachedCameraCollisionTest = CameraBoom->bDoCollisionTest;
			bHasCachedCameraCollisionTest = true;
			CameraBoom->bDoCollisionTest = false;
		}

		PlayerCharacter->StartAimObstructionTrace();
	}
}

void UPlayerAimGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPlayerAimGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bHasCachedFOV)
	{
		if (AMGSPlayerController* PlayerController = GetMGSPlayerControllerFromActorInfo())
		{
			if (PlayerController->PlayerCameraManager)
			{
				// FOV 복원
				PlayerController->PlayerCameraManager->SetFOV(CachedFOV);
			}
		}

		bHasCachedFOV = false;
	}

	if (bHasCachedCameraSocketOffset)
	{
		if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
		{
			if (USpringArmComponent* CameraBoom = PlayerCharacter->GetCameraBoom())
			{
				// 카메라 오프셋 복원
				CameraBoom->SocketOffset = CachedCameraSocketOffset;

				if (bHasCachedCameraCollisionTest)
				{
					CameraBoom->bDoCollisionTest = bCachedCameraCollisionTest;
				}
			}
		}

		bHasCachedCameraSocketOffset = false;
	}

	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		PlayerCharacter->StopAimObstructionTrace();
	}

	bHasCachedCameraCollisionTest = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
