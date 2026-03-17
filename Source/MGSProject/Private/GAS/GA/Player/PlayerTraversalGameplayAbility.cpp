/*
 * 파일명 : PlayerTraversalGameplayAbility.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-16
 * 수정자 : 김동석
 * 수정일 : 2026-03-17
 */

#include "GAS/GA/Player/PlayerTraversalGameplayAbility.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/TraversalComponent/MGSTraversalComponent.h"
#include "MotionWarpingComponent.h"
#include "ChooserFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "PoseSearch/PoseSearchLibrary.h"

UPlayerTraversalGameplayAbility::UPlayerTraversalGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 필요 시 실행 정책 설정 (PlayerGameplayAbility 기본값은 OnTriggered)
	AbilityActivationPolicy = EBaseAbilityActivationPolicy::OnTriggered;
}

void UPlayerTraversalGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle SpecHandle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(SpecHandle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 부모 클래스의 헬퍼 함수를 통해 플레이어 캐릭터 획득
	APlayerCharacter* Character = GetPlayerCharacterFromActorInfo();
	UMGSTraversalComponent* TraversalComp = Character->FindComponentByClass<UMGSTraversalComponent>();
	UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponentFromActorInfo();
	
	if (!Character || !ASC || !TraversalComp)
	{
		EndAbility(SpecHandle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 지형 분석 실행
	FMGSTraversalChooserInputs Inputs;
	if (TraversalComp->CheckTraversal(Inputs))
	{
		FChooserEvaluationContext ChooserContext;
	
		// 1. 첫 번째 소스: 캐릭터의 애니메이션 인스턴스 (BaseAnimInstance 대응)
		UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			ChooserContext.AddObjectParam(AnimInst);
		}
	
		// 2. 두 번째 소스: 우리가 분석한 구조체 (MGSTraversalChooserInputs 대응)
		ChooserContext.AddStructParam(Inputs);
	
		UAnimMontage* SelectedMontage = nullptr;
		UChooserTable::EvaluateChooser(ChooserContext, TraversalChooserTable,
		                               FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda(
			                               [&SelectedMontage](UObject* InObject)
			                               {
				                               // 첫 번째로 유효한 몽타주를 찾으면 저장하고 중단합니다.
				                               if (UAnimMontage* Montage = Cast<UAnimMontage>(InObject))
				                               {
					                               SelectedMontage = Montage;
					                               return FObjectChooserBase::EIteratorStatus::Stop; // 하나 찾았으니 중단
				                               }
				                               return FObjectChooserBase::EIteratorStatus::Continue; // 계속 탐색
			                               }));
	
		if (SelectedMontage)
		{
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
			// Motion Warping 설정
			SetMotionWarpingTargets(Inputs);
	
			// 몽타주 재생 태스크
			UAbilityTask_PlayMontageAndWait* MontageTask =
				UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
					this, NAME_None, SelectedMontage, 1.0f, NAME_None, false);
			
			MontageTask->OnCompleted.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
			MontageTask->OnBlendOut.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
			MontageTask->OnCancelled.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
			MontageTask->ReadyForActivation();
			
			return;
		}
	}
	EndAbility(SpecHandle, ActorInfo, ActivationInfo, true, false);
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(MGSGameplayTags::Ability_Player_Jump));
}


void UPlayerTraversalGameplayAbility::SetMotionWarpingTargets(const FMGSTraversalChooserInputs& Inputs)
{
	APlayerCharacter* Character = GetPlayerCharacterFromActorInfo();
	if (!Character) return;

	UMotionWarpingComponent* MWComp = Character->FindComponentByClass<UMotionWarpingComponent>();
	if (!MWComp) return;

	// 캐릭터 정렬 회전값 (장애물을 정면으로 응시하도록 Normal 벡터의 반대 방향으로 회전)
	FVector LookAtDir = -Inputs.FrontLedgeNormal;
	LookAtDir.Z = 0.0f; // 수평 회전만 고려
	FRotator LedgeRotation = Character->GetActorRotation();
	FVector AdjustedFrontLedge = Inputs.FrontLedgeLocation + FVector(0, 0, 2.0f);
	FVector AdjustedBackLedge = Inputs.BackLedgeLocation + FVector(0, 0, 2.0f);
	
	// 1. FrontLedge (모든 파쿠르 동작 공통: 손을 짚거나 딛을 앞 모서리)
	MWComp->AddOrUpdateWarpTargetFromLocationAndRotation(FName("FrontLedge"), AdjustedFrontLedge, LedgeRotation);

	// 2. BackLedge (Vault, Hurdle 동작 전용: 반대편 모서리로 위치 보정)
	if (Inputs.bHasBackLedge)
	{
		MWComp->AddOrUpdateWarpTargetFromLocationAndRotation(FName("BackLedge"), AdjustedBackLedge, LedgeRotation);
	}
	else
	{
		MWComp->RemoveWarpTarget(FName("BackLedge"));
	}

	// 3. BackFloor (Hurdle 동작 전용: 장애물을 넘어 바닥 착지 지점 보정)
	if (Inputs.bHasBackFloor && Inputs.ActionType == EMGSTraversalActionType::Hurdle)
	{
		MWComp->AddOrUpdateWarpTargetFromLocationAndRotation(FName("BackFloor"), Inputs.BackFloorLocation, LedgeRotation);
	}
	else
	{
		MWComp->RemoveWarpTarget(FName("BackFloor"));
	}
}

void UPlayerTraversalGameplayAbility::OnTraversalFinished()
{
	APlayerCharacter* Character = GetPlayerCharacterFromActorInfo();
	if (!Character) return;
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	// 몽타주 재생이 끝났거나 취소되었을 때 어빌리티를 종료하여 캐릭터가 다시 움직일 수 있게 합니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPlayerTraversalGameplayAbility::OnTraversalStopEventReceived(FGameplayEventData Payload)
{
	// 노티파이에서 중단 신호가 오면 즉시 몽타주를 중단하고 상태 복구
	APlayerCharacter* Character = GetPlayerCharacterFromActorInfo();
	UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
	if (AnimInst)
	{
		AnimInst->Montage_Stop(0.2f); // 부드럽게 중단
		OnTraversalFinished(); // 상태 복구 함수 호출
	}
}

void UPlayerTraversalGameplayAbility::RestoreMovementState(APlayerCharacter* Character, EMovementMode OriginalMode)
{
	if (!Character) return;

	// 원래 모드가 Falling이었다면 그대로 두되, Walking이었다면 다시 지면 상태로 돌려놓음
	Character->GetCharacterMovement()->SetMovementMode(OriginalMode);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}