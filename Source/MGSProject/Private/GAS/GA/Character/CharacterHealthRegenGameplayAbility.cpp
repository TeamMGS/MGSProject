/*
 * 파일명 : CharacterHealthRegenGameplayAbility.cpp
 * 생성자 : Codex
 * 생성일 : 2026-03-24
 */

#include "GAS/GA/Character/CharacterHealthRegenGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Characters/BaseCharacter.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/MGSGameplayTags.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"

UCharacterHealthRegenGameplayAbility::UCharacterHealthRegenGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnGiven;
	bClearAbilityOnEndWhenGiven = false;

	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(MGSGameplayTags::Ability_Character_HealthRegen);
	SetAssetTags(AbilityAssetTags);
}

void UCharacterHealthRegenGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	if (!bIsHpDelegateBound)
	{
		CurrentHpChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
			.AddUObject(this, &ThisClass::HandleCurrentHpChanged);
		bIsHpDelegateBound = true;
	}

	if (CanRegenerate() && GetCurrentHp() < GetMaxHp())
	{
		RestartRegenDelay();
	}
}

void UCharacterHealthRegenGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearTimers();

	if (bIsHpDelegateBound)
	{
		if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetCurrentHpAttribute())
				.Remove(CurrentHpChangedDelegateHandle);
		}

		CurrentHpChangedDelegateHandle.Reset();
		bIsHpDelegateBound = false;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCharacterHealthRegenGameplayAbility::HandleCurrentHpChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	if (AttributeChangeData.NewValue < AttributeChangeData.OldValue - KINDA_SMALL_NUMBER)
	{
		RestartRegenDelay();
		return;
	}

	if (AttributeChangeData.NewValue >= GetMaxHp() - KINDA_SMALL_NUMBER)
	{
		StopRegeneration();
	}
}

void UCharacterHealthRegenGameplayAbility::RestartRegenDelay()
{
	ClearTimers();

	if (!CanRegenerate() || HealPerSecond <= 0.0f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RegenDelayTimerHandle,
			this,
			&ThisClass::StartRegeneration,
			FMath::Max(0.0f, RegenStartDelay),
			false);
	}
}

void UCharacterHealthRegenGameplayAbility::StartRegeneration()
{
	RegenDelayTimerHandle.Invalidate();

	if (!CanRegenerate() || GetCurrentHp() >= GetMaxHp() - KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RegenTickTimerHandle,
			this,
			&ThisClass::ApplyRegenerationTick,
			RegenTickInterval,
			true);
	}
}

void UCharacterHealthRegenGameplayAbility::StopRegeneration()
{
	ClearTimers();
}

void UCharacterHealthRegenGameplayAbility::ApplyRegenerationTick()
{
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	if (!ASC || !CanRegenerate())
	{
		StopRegeneration();
		return;
	}

	const float CurrentHp = GetCurrentHp();
	const float MaxHp = GetMaxHp();
	if (CurrentHp >= MaxHp - KINDA_SMALL_NUMBER)
	{
		StopRegeneration();
		return;
	}

	const float HealAmount = FMath::Max(0.0f, HealPerSecond) * FMath::Max(KINDA_SMALL_NUMBER, RegenTickInterval);
	const float NewHp = FMath::Min(CurrentHp + HealAmount, MaxHp);
	ASC->SetNumericAttributeBase(UCharacterAttributeSet::GetCurrentHpAttribute(), NewHp);

	if (NewHp >= MaxHp - KINDA_SMALL_NUMBER)
	{
		StopRegeneration();
	}
}

bool UCharacterHealthRegenGameplayAbility::CanRegenerate() const
{
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	return ASC && !ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Character_Dead) && GetCurrentHp() > KINDA_SMALL_NUMBER;
}

float UCharacterHealthRegenGameplayAbility::GetCurrentHp() const
{
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	return ASC ? ASC->GetNumericAttribute(UCharacterAttributeSet::GetCurrentHpAttribute()) : 0.0f;
}

float UCharacterHealthRegenGameplayAbility::GetMaxHp() const
{
	const UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	return ASC ? FMath::Max(1.0f, ASC->GetNumericAttribute(UCharacterAttributeSet::GetMaxHpAttribute())) : 1.0f;
}

void UCharacterHealthRegenGameplayAbility::ClearTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
		World->GetTimerManager().ClearTimer(RegenTickTimerHandle);
	}
}
