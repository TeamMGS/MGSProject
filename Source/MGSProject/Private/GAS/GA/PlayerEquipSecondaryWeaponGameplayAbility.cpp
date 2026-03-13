/*
 * 파일명 : PlayerEquipSecondaryWeaponGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/PlayerEquipSecondaryWeaponGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseWeapon.h"

UPlayerEquipSecondaryWeaponGameplayAbility::UPlayerEquipSecondaryWeaponGameplayAbility()
{
	AbilityTags.AddTag(MGSGameplayTags::Ability_Player_Equip_Secondary);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

void UPlayerEquipSecondaryWeaponGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	if (!PlayerCombatComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABaseWeapon* CurrentWeapon = PlayerCombatComponent->GetCharacterCurrentEquippedWeapon();
	ABaseWeapon* TargetWeapon = PlayerCombatComponent->GetCharacterCarriedWeaponByTag(MGSGameplayTags::Weapon_Player_Secondary);
	
	if (!TargetWeapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); 
		return;
	}

	// 몽타주 및 시작 섹션 결정
	UAnimMontage* EquipMontage = TargetWeapon->GetWeaponData().EquipMontage;
	if (!EquipMontage) 
	{
		// 몽타주가 없으면 그냥 즉시 장착하고 종료
		PlayerCombatComponent->EquipSecondaryWeapon();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	// 시작 세션 결정
	FName StartSection = (CurrentWeapon != nullptr) ? FName("Holster") : FName("Equip");
	
	// 몽타주 재생 태스크
	UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, EquipMontage, 1.0f, StartSection);

	PlayTask->OnCompleted.AddDynamic(this, &UPlayerEquipSecondaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnInterrupted.AddDynamic(this, &UPlayerEquipSecondaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnBlendOut.AddDynamic(this, &UPlayerEquipSecondaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->OnCancelled.AddDynamic(this, &UPlayerEquipSecondaryWeaponGameplayAbility::OnMontageFinished);
	PlayTask->ReadyForActivation();

	// 애니메이션 이벤트 대기 태스크 (Attach/Detach 모두 수신)
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip"), nullptr, false, false);
	
	EventTask->EventReceived.AddDynamic(this, &UPlayerEquipSecondaryWeaponGameplayAbility::OnEventReceived);
	EventTask->ReadyForActivation();
}

void UPlayerEquipSecondaryWeaponGameplayAbility::OnEventReceived(FGameplayEventData Payload)
{
	UPlayerCombatComponent* CombatComp = GetPlayerCombatComponentFromActorInfo();
	if (!CombatComp) return;

	FGameplayTag EventTag = Payload.EventTag;
	// 노티파이에서 보낸 태그에 따라 분기
	if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip.Detach")))
	{
		// 기존 무기를 안보이게
		if (ABaseWeapon* CurrentWeapon = CombatComp->GetCharacterCurrentEquippedWeapon())
		{
			CurrentWeapon->SetActorHiddenInGame(true);
		}
	}
	else if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Player.Weapon.Equip.Attach")))
	{
		// 이 시점에 실제 로직 교체 (데이터 갱신, 레이어 교체, 새 무기 손에 부착)
		CombatComp->EquipSecondaryWeapon();
	}
}

void UPlayerEquipSecondaryWeaponGameplayAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
