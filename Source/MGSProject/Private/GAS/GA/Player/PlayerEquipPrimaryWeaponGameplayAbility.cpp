/*
 * 파일명 : PlayerEquipPrimaryWeaponGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/Player/PlayerEquipPrimaryWeaponGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/Player/MGSPlayerState.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "Weapon/BaseGun.h"
#include "Weapon/BaseWeapon.h"

UPlayerEquipPrimaryWeaponGameplayAbility::UPlayerEquipPrimaryWeaponGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Equip_Primary);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

void UPlayerEquipPrimaryWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bShouldUnequipCurrentWeapon = false;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UPlayerCombatComponent* PlayerCombatComponent = GetPlayerCombatComponentFromActorInfo();
	if (!PlayerCombatComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABaseWeapon* CurrentWeapon = PlayerCombatComponent->GetCharacterCurrentEquippedWeapon();
	ABaseWeapon* TargetWeapon = PlayerCombatComponent->GetCharacterCarriedWeaponByTag(MGSGameplayTags::Weapon_Primary);
	
	if (!TargetWeapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); 
		return;
	}

	bShouldUnequipCurrentWeapon = (CurrentWeapon == TargetWeapon);

	// 몽타주 및 시작 섹션 결정
	UAnimMontage* EquipMontage = TargetWeapon->GetWeaponData().EquipMontage;
	if (!EquipMontage) 
	{
		// 같은 슬롯 재입력은 즉시 해제, 아니면 즉시 장착
		if (bShouldUnequipCurrentWeapon)
		{
			PlayerCombatComponent->UnequipCurrentWeapon();
		}
		else
		{
			PlayerCombatComponent->EquipPrimaryWeapon();
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	// 시작 세션 결정
	FName StartSection = (CurrentWeapon != nullptr) ? FName("Holster") : FName("Equip");
	
	// 몽타주 재생 태스크
	UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, EquipMontage, 1.0f, StartSection);

	PlayTask->OnCompleted.AddDynamic(this, &UPlayerEquipPrimaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnInterrupted.AddDynamic(this, &UPlayerEquipPrimaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnBlendOut.AddDynamic(this, &UPlayerEquipPrimaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnCancelled.AddDynamic(this, &UPlayerEquipPrimaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->ReadyForActivation();

	// 애니메이션 이벤트 대기 태스크 (Attach/Detach 모두 수신)
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip"), nullptr, false, false);
	
	EventTask->EventReceived.AddDynamic(this, &UPlayerEquipPrimaryWeaponGameplayAbility::OnEventReceived);
	EventTask->ReadyForActivation();
	
	const ABaseGun* CueGun = Cast<ABaseGun>(TargetWeapon);
	const APlayerCharacter* PlayerCharacter = ActorInfo ? Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const AMGSPlayerController* PlayerController = PlayerCharacter ? PlayerCharacter->GetController<AMGSPlayerController>() : nullptr;
	if (PlayerCharacter && PlayerController)
	{
		if (const AMGSPlayerState* PlayerState = PlayerController->GetPlayerState<AMGSPlayerState>())
		{
			if (UMGSAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetMGSAbilitySystemComponent())
			{
				FGameplayCueParameters Parameters;
				Parameters.Location = CueGun ? CueGun->GetActorLocation() : PlayerCharacter->GetActorLocation();
				Parameters.SourceObject = CueGun;
				AbilitySystemComponent->ExecuteGameplayCue(MGSGameplayTags::GameplayCue_Weapon_Rack, Parameters);
			}
		}
	}
}

void UPlayerEquipPrimaryWeaponGameplayAbility::OnEventReceived(FGameplayEventData Payload)
{
	UPlayerCombatComponent* CombatComp = GetPlayerCombatComponentFromActorInfo();
	if (!CombatComp) return;

	FGameplayTag EventTag = Payload.EventTag;
	// 노티파이에서 보낸 태그에 따라 분기
	if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip.Detach")))
	{
		if (bShouldUnequipCurrentWeapon)
		{
			CombatComp->UnequipCurrentWeapon();
		}
		else
		{
			// 기존 무기를 안보이게
			if (ABaseWeapon* CurrentWeapon = CombatComp->GetCharacterCurrentEquippedWeapon())
			{
				CurrentWeapon->SetActorHiddenInGame(true);
			}
		}
	}
	else if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip.Attach")))
	{
		if (!bShouldUnequipCurrentWeapon)
		{
			// 이 시점에 실제 로직 교체 (데이터 갱신, 레이어 교체, 새 무기 손에 부착)
			CombatComp->EquipPrimaryWeapon();
		}
	}
}

void UPlayerEquipPrimaryWeaponGameplayAbility::OnMontageFinished()
{
	bShouldUnequipCurrentWeapon = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
