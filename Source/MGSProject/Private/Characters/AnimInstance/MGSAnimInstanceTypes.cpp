/*
 * 파일명 : MGSAnimInstanceTypes.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
 */
#include "Characters/AnimInstance/MGSAnimInstanceTypes.h"
#include "Characters/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void FMGSCharacterDataProxy::Update(class ABaseCharacter* Character)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	UMGSAbilitySystemComponent* ASC = Character->GetMGSAbilitySystemComponent();
	if (!MoveComp) return;
	if (ASC)
	{
		GameplayTags.Reset();
		ASC->GetOwnedGameplayTags(GameplayTags);
	}
	// 물리 데이터 업데이트
	Velocity = Character->GetVelocity();
	ActorTransform = Character->GetActorTransform();
	InputAcceleration = MoveComp->GetCurrentAcceleration();
	MaxAcceleration = MoveComp->GetMaxAcceleration();
	MaxDeceleration = MoveComp->BrakingDecelerationWalking;

	// 회전 및 지면 정보
	OrientationIntent = Character->GetActorRotation();
	AimingRotation = Character->IsLocallyControlled() ?
					 Character->GetControlRotation() :
					 Character->GetBaseAimRotation();

	RotationMode = MoveComp->bOrientRotationToMovement ?
			   EMGSRotationMode::VelocityDirection :
			   EMGSRotationMode::LookingDirection;

	if (MoveComp->CurrentFloor.bBlockingHit)
	{
		GroundNormal = MoveComp->CurrentFloor.HitResult.ImpactNormal;
	}
	else
	{
		GroundNormal = FVector::UpVector;
	}

	// // 상태 및 입력 (GAS 태그 연동)
	// if (ASC)
	// {
	// 	Stance = ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching) ?
	// 			 EMGSStance::Crouch : EMGSStance::Stand;
	//
	// 	if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint))
	// 		Gait = EMGSGait::Sprint;
	// 	else if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk))
	// 		Gait = EMGSGait::Walk;
	// 	else
	// 		Gait = EMGSGait::Run;
	//
	// 	InputState.bWantsToSprint = ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint);
	// 	InputState.bWantsToWalk   = ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk);
	// 	InputState.bWantsToAim    = ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Aiming);
	// 	InputState.bWantsToCrouch = ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Crouching);
	// }
}

void FMGSEssentialValues::Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, float DeltaSeconds)
{
	if (DeltaSeconds <= 0.f) return;

	// 가속도량 계산 (GASP: SafeDivide(VSize(Accel), MaxAccel))
	const float AccelSize = Data.InputAcceleration.Size();
	AccelerationAmount = (Data.MaxAcceleration > 0.f) ? (AccelSize / Data.MaxAcceleration) : 0.f;
	bHasAcceleration = AccelerationAmount > 0.f;

	// 속도 및 2D 속도 계산 (GASP: VSizeXY)
	Speed2D = Data.Velocity.Size2D();
	bHasVelocity = Speed2D > 5.0f; // GASP 기준값 5.0

	// 가속도 미분 계산 (GASP: (V - V_Last) / DeltaTime)
	const FVector VelocityAcceleration = (Data.Velocity - Velocity_LastFrame) / DeltaSeconds;
	Velocity_LastFrame = Data.Velocity; // 다음 프레임을 위해 저장

	// RootTransform 및 상대적 가속도 (GASP: UnrotateVector)
	// 실제 GASP는 OffsetRoot 노드에서 값을 가져오지만, 여기서는 액터 회전을 기반으로 90도 보정 로직을 넣습니다.
	FRotator RootRotation = Data.OrientationIntent;
	RootRotation.Yaw += 90.0f; // 스켈레탈 메쉬 -90도 보정
	RootTransform.SetRotation(RootRotation.Quaternion());
	RootTransform.SetLocation(Data.ActorTransform.GetLocation());

	RelativeAcceleration = RootTransform.GetRotation().UnrotateVector(VelocityAcceleration);
	
	// RelativeAccelerationAmount 계산
	FVector RelativeAccelerationAmount = FVector::ZeroVector;
	if (Data.MaxAcceleration > 0.f)
	{
		// X, Y, Z 각각을 MaxAcceleration으로 나누고 클램핑
		RelativeAccelerationAmount.X = FMath::Clamp(RelativeAcceleration.X / Data.MaxAcceleration, -1.0f, 1.0f);
		RelativeAccelerationAmount.Y = FMath::Clamp(RelativeAcceleration.Y / Data.MaxAcceleration, -1.0f, 1.0f);
		RelativeAccelerationAmount.Z = FMath::Clamp(RelativeAcceleration.Z / Data.MaxAcceleration, -1.0f, 1.0f);
	}

	// 속도 기반의 Lean Multiplier 계산
	float LeanMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(165.f, 375.f), FVector2D(0.5f, 1.0f), Speed2D);
	
	LeanAmount.X = RelativeAccelerationAmount.Y * LeanMultiplier;

	// 마지막 유효 속도 기록
	if (bHasVelocity)
	{
		LastNonZeroVelocity = Data.Velocity;
	}
	
	if (AnimInstance)
	{
		// 시선 방향과 루트 방향의 차이 계산
		FRotator DeltaRot = (Data.AimingRotation - RootTransform.GetRotation().Rotator()).GetNormalized();

		// Disable_AO 커브 값 가져오기
		float DisableAO_CurveValue = AnimInstance->GetCurveValue(FName("Disable_AO"));

		// VLerp 적용 및 AOValue 저장
		AOValue.X = FMath::Lerp(DeltaRot.Yaw, 0.0f, DisableAO_CurveValue);
		AOValue.Y = FMath::Lerp(DeltaRot.Pitch, 0.0f, DisableAO_CurveValue);
	}
}

void FMGSMotionMatchingHandler::Update(UAnimInstance* AnimInstance, const FAnimNodeReference& Node)
{
	if (!AnimInstance || !MainChooserTable) return;

	// Chooser 평가 
	ValidDatabases.Empty();
	
	// EvaluateChooserMulti는 여러 개의 결과를 반환합니다.
	TArray<UObject*> ChooserResults = UChooserFunctionLibrary::EvaluateChooserMulti(
		AnimInstance,        // ContextObject (AnimInstance가 컨텍스트 역할)
		MainChooserTable,    // 에디터에서 설정한 Chooser Table
		UPoseSearchDatabase::StaticClass() // 우리가 원하는 결과물 타입
	);

	// 반환된 UObject 배열을 UPoseSearchDatabase 타입으로 캐스팅하여 저장합니다.
	for (UObject* Obj : ChooserResults)
	{
		if (UPoseSearchDatabase* DB = Cast<UPoseSearchDatabase>(Obj))
		{
			ValidDatabases.Add(DB);
		}
	}
	
	// 모션 매칭 노드에 데이터베이스 설정
	FMotionMatchingAnimNodeReference MMNode;
	bool bResult = false;
	UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNodePure(Node, MMNode, bResult);

	if (bResult)
	{
		UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(MMNode, ValidDatabases, EPoseSearchInterruptMode::DoNotInterrupt);
	}
}

void FMGSMotionMatchingHandler::PostSelection(UAnimInstance* AnimInstance, const FAnimNodeReference& Node)
{
	// 노드 참조 확인
	FMotionMatchingAnimNodeReference MMNode;
	bool bNodeValid = false;
	UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNodePure(Node, MMNode, bNodeValid);
	
	if (!bNodeValid) return;

	// 검색 결과 가져오기 (GASP: GetMotionMatchingSearchResult 노드)
	FPoseSearchBlueprintResult SearchResult;
	bool bIsResultValid = false;

	UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MMNode, SearchResult, bIsResultValid);
	
	if (bIsResultValid)
	{
		CurrentSelectedAnim = SearchResult.SelectedAnim;
		CurrentSelectedDatabase = SearchResult.SelectedDatabase;
		// 데이터베이스 태그 가져오기
		if (CurrentSelectedDatabase)
		{
			CurrentDatabaseTags.Empty();
			UPoseSearchLibrary::GetDatabaseTags(CurrentSelectedDatabase, CurrentDatabaseTags);
		}
	}
}

void FMGSTrajectoryHandler::Update(class UAnimInstance* AnimInstance, const FMGSEssentialValues& Values,
                                   float DeltaSeconds)
{
	if (!AnimInstance || DeltaSeconds <= 0.f) return;

	// 현재 컨트롤러(카메라)의 Yaw 값을 가져옵니다.
	APawn* Pawn = AnimInstance->TryGetPawnOwner();
	if (!Pawn) return;
	
	const float CurrentControllerYaw = Pawn->GetControlRotation().Yaw;
	
	// 보간 로직 적용
	// DesiredControllerYawLastUpdate가 컨트롤러의 실제 Yaw를 부드럽게 따라가도록 합니다.
	// 속도는 20.0f 
	DesiredControllerYawLastUpdate = FMath::FInterpTo(
		DesiredControllerYawLastUpdate,
		CurrentControllerYaw,
		DeltaSeconds,
		20.0f
	);
	
	// 현재 상황에 맞는 설정 데이터 선택
	const FPoseSearchTrajectoryData& SelectedSettings = (Values.Speed2D > 0.f) ?
														TrajectorySettings_Moving :
														TrajectorySettings_Idle;

	// 라이브러리 함수 호출 
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(
		AnimInstance,
		SelectedSettings, // 구조체 전달
		DeltaSeconds,
		Trajectory, // InOut
		DesiredControllerYawLastUpdate, // InOut
		Trajectory, // Out
		-1.0f, // History Interval
		30,    // History Count
		0.1f,  // Prediction Interval
		15     // Prediction Count
	);

	// 충돌 보정
	FPoseSearchTrajectory_WorldCollisionResults CollisionResult;
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		AnimInstance,
		AnimInstance,
		Trajectory,
		true, 0.01f, Trajectory, CollisionResult,
		UEngineTypes::ConvertToTraceType(ECC_Visibility), false, TArray<AActor*>(), EDrawDebugTrace::None
	);
}

void FMGSLocomotionState::Update(const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential, const FMGSTrajectoryHandler& Trajectory)
{
	// Movement State 판정 (GASP의 핵심 로직)
	// 현재 속도가 있고, 미래의 예측 속도(Trajectory)도 있는 경우 '이동 중'으로 간주
	const bool bHasCurrentVelocity = Data.Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER;
	const bool bHasFutureVelocity = Trajectory.Trj_FutureVelocity.SizeSquared2D() > KINDA_SMALL_NUMBER;
	const bool bHasAcceleration = Data.InputAcceleration.SizeSquared2D() > KINDA_SMALL_NUMBER;
	
	if (bHasCurrentVelocity || bHasFutureVelocity || bHasAcceleration)
	{
		MovementStateTag = MGSGameplayTags::State_Player_Movement_Moving;
	}
	else
	{
		MovementStateTag = MGSGameplayTags::State_Player_Movement_Idle;
	}
	
	
	// 제자리 회전
	FRotator DeltaRot = (Data.OrientationIntent - Essential.RootTransform.GetRotation().Rotator()).GetNormalized();
	const bool bOverThreshold = FMath::Abs(DeltaRot.Yaw) > 50.0f;
	const bool bWantsToAim = Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Aiming);

	const bool bIsIdle = (MovementStateTag == MGSGameplayTags::State_Player_Movement_Idle);
	const bool bWasMoving = (LastFrameMovementTag == MGSGameplayTags::State_Player_Movement_Moving);
	const bool bStickFlick = bIsIdle && bWasMoving;
	
	bool bIsPivoting = CheckIsPivoting(Data, Essential, Trajectory);
	
	if (bIsPivoting)
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_Pivoting;
	}
	else if (bIsIdle && bOverThreshold && (bWantsToAim || bStickFlick))
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_TurningInPlace;
	}
	else
	{
		LocomotionActionTag = FGameplayTag::EmptyTag;
	}
	
	LastFrameMovementTag = MovementStateTag;
}

bool FMGSLocomotionState::CheckIsPivoting(const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential,
	const FMGSTrajectoryHandler& Trajectory)
{
	// 기본 조건: 움직이고 있어야 함
	if (MovementStateTag != MGSGameplayTags::State_Player_Movement_Moving) return false;

	// 피벗 각도 계산 (현재 속도 방향 vs 미래 예측 궤적 방향)
	FVector CurrentDir = Data.Velocity.GetSafeNormal2D();
	FVector FutureDir = Trajectory.Trj_FutureVelocity.GetSafeNormal2D();

	float Dot = FVector::DotProduct(CurrentDir, FutureDir);
	float TurnAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));
	
	float AngleThreshold = 0.0f;

	// 가중치 계산 (Gait 및 Speed에 따른 동적 임계값)
	if (Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Stance_Crouch))
	{
		AngleThreshold = 65.0f; // 앉은 상태 고정값
	}
	else
	{
		// 서 있는 상태: 속도와 Gait에 따라 MapRangeClamped 적용
		if (Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Gait_Walk))
		{
			AngleThreshold = FMath::GetMappedRangeValueClamped(FVector2D(150.f, 200.f), FVector2D(70.f, 60.f), Essential.Speed2D);
		}
		else if (Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Gait_Run))
		{
			AngleThreshold = FMath::GetMappedRangeValueClamped(FVector2D(300.f, 500.f), FVector2D(70.f, 60.f), Essential.Speed2D);
		}
		else if (Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Gait_Sprint))
		{
			AngleThreshold = FMath::GetMappedRangeValueClamped(FVector2D(300.f, 700.f), FVector2D(60.f, 50.f), Essential.Speed2D);
		}
	}

	// 회전 모드 보정 (MM Pivot conditions 부분)
	if (Data.RotationMode == EMGSRotationMode::VelocityDirection) AngleThreshold += 45.0f;
	else if (Data.RotationMode == EMGSRotationMode::LookingDirection) AngleThreshold += 30.0f;

	// 최종 판정
	return TurnAngle > AngleThreshold;
}
