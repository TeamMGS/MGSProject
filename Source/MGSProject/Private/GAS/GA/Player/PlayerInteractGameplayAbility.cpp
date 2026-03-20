/*
 * 파일명 : PlayerInteractGameplayAbility.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-20
 * 수정자 : 김동석
 * 수정일 : 2026-03-20
 */
#include "GAS/GA/Player/PlayerInteractGameplayAbility.h"

#include "Interfaces/MGSInteractableInterface.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Camera/CameraComponent.h"
#include "Characters/Player/PlayerCharacter.h"

UPlayerInteractGameplayAbility::UPlayerInteractGameplayAbility()
{
}

void UPlayerInteractGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 플레이어 캐릭터와 카메라 가져오기
	APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo();
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCameraComponent* Camera = PlayerCharacter->GetFollowCamera();
	if (!Camera)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Trace 시작점(카메라 위치)과 끝점(정면 200cm) 설정
	const FVector TraceStart = Camera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (Camera->GetForwardVector() * 400.0f);

	// Trace 설정 (플레이어 자신은 제외)
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);

	// LineTrace 실행 (Visibility 채널 사용)
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);
	
	// 디버그 라인
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 2.0f);
	
	// 결과 확인 및 상호작용 시작
	if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<UMGSInteractableInterface>())
	{
		CurrentTarget = Hit.GetActor();
		IMGSInteractableInterface* Interactable = Cast<IMGSInteractableInterface>(CurrentTarget);

		// 물건 집기 애니메이션 재생
		if (InteractMontage && PlayerCharacter)
		{
			PlayerCharacter->PlayAnimMontage(InteractMontage);
		}
		
		// 인터랙터(플레이어)에게 시작 알림
		Interactable->OnInteractionStarted(PlayerCharacter);

		// 2초 대기 태스크 (WaitDelay)
		UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Interactable->GetInteractionDuration());
		DelayTask->OnFinish.AddDynamic(this, &UPlayerInteractGameplayAbility::OnSuccess);
		DelayTask->ReadyForActivation();

		// 키 뗌 감지 태스크 (WaitInputRelease)
		UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
		ReleaseTask->OnRelease.AddDynamic(this, &UPlayerInteractGameplayAbility::OnRelease);
		ReleaseTask->ReadyForActivation();
	}
	else
	{
		// 대상이 없거나 상호작용 불가능한 경우 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UPlayerInteractGameplayAbility::OnRelease(float TimeHeld)
{
	// 키를 떼면 애니메이션 중단
	if (APlayerCharacter* PlayerCharacter = GetPlayerCharacterFromActorInfo())
	{
		PlayerCharacter->StopAnimMontage(InteractMontage);
	}
	
	if (CurrentTarget)
	{
		IMGSInteractableInterface* Interactable = Cast<IMGSInteractableInterface>(CurrentTarget);
		Interactable->OnInteractionCanceled(GetAvatarActorFromActorInfo());
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPlayerInteractGameplayAbility::OnSuccess()
{
	if (CurrentTarget)
	{
		IMGSInteractableInterface* Interactable = Cast<IMGSInteractableInterface>(CurrentTarget);
		Interactable->OnInteractionSucceeded(GetAvatarActorFromActorInfo());
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
