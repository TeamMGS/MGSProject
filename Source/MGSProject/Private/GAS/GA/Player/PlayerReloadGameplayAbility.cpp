/*
 * 파일명 : PlayerReloadGameplayAbility.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-04
 * 수정자 : 김동석
 * 수정일 : 2026-03-12
 */

#include "GAS/GA/Player/PlayerReloadGameplayAbility.h"

#include "Components/Combat/PlayerCombatComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Weapon/BaseGun.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UPlayerReloadGameplayAbility::UPlayerReloadGameplayAbility()
{
	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Player_Reload);
	SetAssetTags(AbilityAssetTags);
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
	bClearAbilityOnEndWhenGiven = false;
}

bool UPlayerReloadGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	
	const ABaseGun* EquippedGun = Cast<ABaseGun>(PlayerCombatComponent->GetCharacterCurrentEquippedWeapon());
	// 장전 가능 확인
	return EquippedGun && EquippedGun->CanReload();
}

void UPlayerReloadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	UAnimMontage* MontageToPlay = EquippedGun->GetWeaponData().ReloadMontage;
	
	if (!EquippedGun)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 2. 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);

	// 3. 애니메이션 이벤트(총알 충전) 대기 태스크 생성
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MGSGameplayTags::Event_Player_Weapon_Reload_Ammo_Refill);

	// 4. 이벤트 수신 시 실제 총알 충전 로직 연결 (Lambda 활용)
	WaitEventTask->EventReceived.AddDynamic(this, &UPlayerReloadGameplayAbility::OnAmmoRefillEventReceived);

	// 5. 몽타주 종료 시 어빌리티 종료 연결
	PlayMontageTask->OnCompleted.AddDynamic(this, &UPlayerReloadGameplayAbility::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UPlayerReloadGameplayAbility::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UPlayerReloadGameplayAbility::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UPlayerReloadGameplayAbility::K2_EndAbility);
	
	// 태스크 활성화
	PlayMontageTask->ReadyForActivation();
	WaitEventTask->ReadyForActivation();
}

void UPlayerReloadGameplayAbility::OnAmmoRefillEventReceived(FGameplayEventData Payload)
{
	UPlayerCombatComponent* CombatComp = GetPlayerCombatComponentFromActorInfo();
	ABaseGun* EquippedGun = Cast<ABaseGun>(CombatComp->GetCharacterCurrentEquippedWeapon());
	
	// 장전
	const int32 ReloadedAmmo = EquippedGun->ReloadAmmo();
	if (bEnableReloadLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[Reload] Reloaded=%d Current=%d/%d Carried=%d"),
			ReloadedAmmo,
			EquippedGun->GetCurrentMagazineAmmo(),
			EquippedGun->GetMaxMagazineAmmo(),
			EquippedGun->GetCarriedAmmo());
	}
}
