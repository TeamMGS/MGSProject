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
#include "PoseSearch/PoseSearchDatabase.h"
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
	
		// 캐릭터의 애니메이션 인스턴스
		UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			ChooserContext.AddObjectParam(AnimInst);
		}
		ChooserContext.AddStructParam(Inputs);
		
		UObject* SelectedAsset = nullptr;
		UChooserTable::EvaluateChooser(ChooserContext, TraversalChooserTable,
									   FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda(
										   [&SelectedAsset](UObject* InObject)
										   {
											   SelectedAsset = InObject;
											   return FObjectChooserBase::EIteratorStatus::Stop;
										   }));
		
		UAnimMontage* MontageToPlay = nullptr;
		float StartTime = 0.0f;
		if (SelectedAsset)
		{
			// --- 상황 A: 결과가 Pose Search Database인 경우 (GASP 방식) ---
			if (UPoseSearchDatabase* SelectedDatabase = Cast<UPoseSearchDatabase>(SelectedAsset))
			{
				FPoseSearchBlueprintResult SearchResult;
				TArray<UObject*> AssetsToSearch;
				AssetsToSearch.Add(SelectedDatabase);

				UPoseSearchLibrary::MotionMatch(AnimInst, AssetsToSearch, FName("PoseHistory"),
				                                FPoseSearchContinuingProperties(), FPoseSearchFutureProperties(),
				                                SearchResult);

				// UE 5.4+ 에서는 SelectedAnimation 멤버를 사용합니다.
				MontageToPlay = Cast<UAnimMontage>(SearchResult.SelectedAnim);
				StartTime = SearchResult.SelectedTime;
			}
			// --- 상황 B: 결과가 일반 Anim Montage인 경우 (사용자 요청 방식) ---
			else if (UAnimMontage* SelectedMontage = Cast<UAnimMontage>(SelectedAsset))
			{
				MontageToPlay = SelectedMontage;

				// [핵심] 거리에 따른 수동 스킵 로직
				// 거리가 150cm 이상이면 0초부터, 50cm 이하면 1.5초(파쿠르 시작점)부터 재생
				// (이 수치들은 사용하시는 몽타주의 실제 '걷기' 길이에 맞춰 조절하세요)
				StartTime = FMath::GetMappedRangeValueClamped(
					FVector2D(50.0f, 225.0f), // 입력 범위 (최소거리, 최대거리)
					FVector2D(0.1f, 0.0f), // 출력 범위 (최대스킵시간, 최소스킵시간)
					Inputs.DistanceToLedge // 현재 장애물과의 수평 거리
				);
			}

			// --- [2] 결정된 몽타주와 시간으로 재생 실행 ---
			if (MontageToPlay)
			{
				Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
				SetMotionWarpingTargets(Inputs);

				UAbilityTask_PlayMontageAndWait* MontageTask =
					UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
						this, NAME_None, MontageToPlay, 1.0f, NAME_None, false, 1.0f, StartTime);

				MontageTask->OnCompleted.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
				MontageTask->OnBlendOut.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
				MontageTask->OnInterrupted.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
				MontageTask->OnCancelled.AddDynamic(this, &UPlayerTraversalGameplayAbility::OnTraversalFinished);
				MontageTask->ReadyForActivation();

				return;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Traversal] Search Failed: No matching pose found in Database."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Traversal] Chooser Failed: No Database selected for current inputs."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Traversal] Detection Failed: Not a traversable terrain."));
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
	FRotator LedgeRotation = Character->GetActorRotation();
	
	float HeightCorrection = -Inputs.ObstacleHeight;
	FVector AdjustedFrontLedge = Inputs.FrontLedgeLocation + FVector(-40.0f, 0.f, 40.0f + HeightCorrection);
	FVector AdjustedBackLedge = Inputs.BackLedgeLocation + FVector(0, 0, 40.0f + HeightCorrection);
	
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