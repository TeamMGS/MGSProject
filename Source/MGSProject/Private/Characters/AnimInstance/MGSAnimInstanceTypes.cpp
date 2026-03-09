/*
 * 파일명 : MGSAnimInstanceTypes.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-09
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
#include "AnimationWarpingLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "Animation/AnimSubsystem_Tag.h"
#include "Animation/AnimNodeReference.h"

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
	//OrientationIntent = Character->GetActorRotation();
	// [수정 후] 플레이어의 '진짜 의도'를 가져옵니다.
	if (MoveComp->GetCurrentAcceleration().SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		// 이동 중일 때는 플레이어가 스틱을 밀고 있는 '가속도 방향'이 곧 의도입니다.
		OrientationIntent = MoveComp->GetCurrentAcceleration().ToOrientationRotator();
	}
	else
	{
		// 멈춰있을 때는 카메라(Control Rotation)가 보고 있는 방향이 의도입니다.
		OrientationIntent = FRotator(0.f, AimingRotation.Yaw, 0.f);
	}
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


}

// void FMGSEssentialValues::Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, float DeltaSeconds)
// {
// 	if (DeltaSeconds <= 0.f) return;
//
// 	// 가속도량 계산 (GASP: SafeDivide(VSize(Accel), MaxAccel))
// 	const float AccelSize = Data.InputAcceleration.Size();
// 	AccelerationAmount = (Data.MaxAcceleration > 0.f) ? (AccelSize / Data.MaxAcceleration) : 0.f;
// 	bHasAcceleration = AccelerationAmount > 0.f;
//
// 	// 속도 및 2D 속도 계산 (GASP: VSizeXY)
// 	Speed2D = Data.Velocity.Size2D();
// 	bHasVelocity = Speed2D > 5.0f; // GASP 기준값 5.0
//
// 	// 가속도 미분 계산 (GASP: (V - V_Last) / DeltaTime)
// 	const FVector VelocityAcceleration = (Data.Velocity - Velocity_LastFrame) / DeltaSeconds;
// 	Velocity_LastFrame = Data.Velocity; // 다음 프레임을 위해 저장
// 	
// 	FTransform RawRootTransform = Data.ActorTransform;
// 	// RootTransform 및 상대적 가속도 (GASP: UnrotateVector)
// 	// 노드 참조 가져오기 (Tag가 "OffsetRoot"인 노드를 찾습니다)
// 	if (AnimInstance)
// 	{
// 		int32 NodeIndex = INDEX_NONE;
//
// 		// AnimInstance 내부의 태그 데이터를 뒤져서 인덱스를 찾습니다.
// 		if (IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass()))
// 		{
// 			// 태그 서브시스템 데이터가 들어있는지 확인 후 인덱스 추출
// 			const FAnimSubsystem_Tag& TagSubsystem = AnimClass->GetSubsystem<FAnimSubsystem_Tag>();
// 			NodeIndex = TagSubsystem.FindNodeIndexByTag(FName("OffsetRoot"));
// 		}
//
// 		if (NodeIndex != INDEX_NONE)
// 		{
// 			// 인덱스를 사용하여 노드 참조 생성
// 			FAnimNodeReference OffsetRootNode = UAnimExecutionContextLibrary::GetAnimNodeReference(AnimInstance, NodeIndex);
//
// 			// 오프셋 트랜스폼 가져오기
// 			RootTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(OffsetRootNode);
// 		}
// 		else
// 		{
// 			RootTransform = Data.ActorTransform;
// 		}
// 	}
//
// 	//  스켈레탈 메쉬의 -90도 보정 적용
// 	FRotator CorrectedRotation = RootTransform.GetRotation().Rotator();
// 	CorrectedRotation.Yaw += 90.0f;
// 	RootTransform.SetRotation(CorrectedRotation.Quaternion());
// 	
// 	// 이제 이 '진짜 메쉬 방향'을 기준으로 상대적 가속도를 계산합니다.
// 	RelativeAcceleration = RootTransform.GetRotation().UnrotateVector(VelocityAcceleration);
// 	
// 	// RelativeAccelerationAmount 계산
// 	FVector RelativeAccelerationAmount = FVector::ZeroVector;
// 	if (Data.MaxAcceleration > 0.f)
// 	{
// 		// X, Y, Z 각각을 MaxAcceleration으로 나누고 클램핑
// 		RelativeAccelerationAmount.X = FMath::Clamp(RelativeAcceleration.X / Data.MaxAcceleration, -1.0f, 1.0f);
// 		RelativeAccelerationAmount.Y = FMath::Clamp(RelativeAcceleration.Y / Data.MaxAcceleration, -1.0f, 1.0f);
// 		RelativeAccelerationAmount.Z = FMath::Clamp(RelativeAcceleration.Z / Data.MaxAcceleration, -1.0f, 1.0f);
// 	}
//
// 	// 속도 기반의 Lean Multiplier 계산
// 	float LeanMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(165.f, 375.f), FVector2D(0.5f, 1.0f), Speed2D);
// 	
// 	LeanAmount.X = RelativeAccelerationAmount.Y * LeanMultiplier;
//
// 	// 마지막 유효 속도 기록
// 	if (bHasVelocity)
// 	{
// 		LastNonZeroVelocity = Data.Velocity;
// 	}
// 	
// 	if (AnimInstance)
// 	{
// 		float CameraYaw = Data.AimingRotation.Yaw;
// 		float ActorYaw = Data.ActorTransform.Rotator().Yaw;
//
// 		// [Pitch 수정] NormalizeAxis를 사용하여 350도 같은 값을 -10도 형태로 변환합니다.
// 		float CameraPitch = FRotator::NormalizeAxis(Data.AimingRotation.Pitch);
//
// 		// 몸통(메쉬)의 뒤처짐 계산
// 		float MeshOffsetYaw = FRotator::NormalizeAxis(RawRootTransform.Rotator().Yaw - ActorYaw);
//
// 		// 최종 Yaw: (카메라-캡슐) 차이에서 몸통이 못 따라온 만큼을 더 보정
// 		float FinalYaw = FRotator::NormalizeAxis(CameraYaw - ActorYaw) - MeshOffsetYaw;
//
// 		float DisableAO = AnimInstance->GetCurveValue(FName("Disable_AO"));
//
// 		// AOValue에 정규화된 값 적용
// 		AOValue.X = FMath::Lerp(FRotator::NormalizeAxis(FinalYaw), 0.0f, DisableAO);
// 		AOValue.Y = FMath::Lerp(CameraPitch, 0.0f, DisableAO); // 하늘 보는 문제 해결
// 	}
// 	
// 	// 캐릭터가 공중 상태인지 확인
// 	const bool bIsAirborne = Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Mode_InAir) ||
// 							 Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Movement_Falling);
//
// 	if (bIsAirborne)
// 	{
// 		// 공중에 있다면 시간 누적
// 		InAirTime += DeltaSeconds;
// 	}
// 	else
// 	{
// 		// 지면에 닿아 있다면 0으로 초기화
// 		InAirTime = 0.f;
// 	}
// }
void FMGSEssentialValues::Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, float DeltaSeconds)
{
	if (DeltaSeconds <= 0.f) return;

	// 1. 기초 물리 계산
	const float AccelSize = Data.InputAcceleration.Size();
	AccelerationAmount = (Data.MaxAcceleration > 0.f) ? (AccelSize / Data.MaxAcceleration) : 0.f;
	bHasAcceleration = AccelerationAmount > 0.f;
	Speed2D = Data.Velocity.Size2D();
	bHasVelocity = Speed2D > 5.0f;

	const FVector VelocityAcceleration = (Data.Velocity - Velocity_LastFrame) / DeltaSeconds;
	Velocity_LastFrame = Data.Velocity;

	// 2. [수정] 오프셋 루트 트랜스폼 가져오기 (임시 변수에 저장)
	FTransform RawRootTransform = Data.ActorTransform; // 기본값
	if (AnimInstance)
	{
		int32 NodeIndex = INDEX_NONE;
		if (IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass()))
		{
			const FAnimSubsystem_Tag& TagSubsystem = AnimClass->GetSubsystem<FAnimSubsystem_Tag>();
			NodeIndex = TagSubsystem.FindNodeIndexByTag(FName("OffsetRoot"));
		}

		if (NodeIndex != INDEX_NONE)
		{
			FAnimNodeReference OffsetRootNode = UAnimExecutionContextLibrary::GetAnimNodeReference(
				AnimInstance, NodeIndex);
			RawRootTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(OffsetRootNode);
		}
	}

	// 3. [핵심] 조준 각도 계산 (메쉬 보정 전의 순수 값 사용)
	if (AnimInstance)
	{
		float CameraYaw = Data.AimingRotation.Yaw;
		float ActorYaw = Data.ActorTransform.Rotator().Yaw;

		// [Pitch 수정] NormalizeAxis를 사용하여 350도 같은 값을 -10도 형태로 변환합니다.
		float CameraPitch = FRotator::NormalizeAxis(Data.AimingRotation.Pitch);

		// 몸통(메쉬)의 뒤처짐 계산
		float MeshOffsetYaw = FRotator::NormalizeAxis(RawRootTransform.Rotator().Yaw - ActorYaw);

		// 최종 Yaw: (카메라-캡슐) 차이에서 몸통이 못 따라온 만큼을 더 보정
		float FinalYaw = FRotator::NormalizeAxis(CameraYaw - ActorYaw) - MeshOffsetYaw;

		float DisableAO = AnimInstance->GetCurveValue(FName("Disable_AO"));

		// AOValue에 정규화된 값 적용
		AOValue.X = FMath::Lerp(FRotator::NormalizeAxis(FinalYaw), 0.0f, DisableAO);
		AOValue.Y = FMath::Lerp(CameraPitch, 0.0f, DisableAO); // 하늘 보는 문제 해결
	}

	// 4. 시각적 보정 및 가속도 계산 (여기서 멤버 변수 RootTransform 업데이트)
	RootTransform = RawRootTransform;
	FRotator VisualRootRot = RootTransform.Rotator();
	VisualRootRot.Yaw += 90.0f; // 스켈레탈 메쉬 -90도 상쇄
	RootTransform.SetRotation(VisualRootRot.Quaternion());

	// 이제 보정된 방향을 기준으로 상대적 가속도 계산
	RelativeAcceleration = RootTransform.GetRotation().UnrotateVector(VelocityAcceleration);

	// RelativeAccelerationAmount 및 Lean 계산 (기존 로직 유지)
	FVector RelativeAccelAmt = FVector::ZeroVector;
	if (Data.MaxAcceleration > 0.f)
	{
		RelativeAccelAmt.X = FMath::Clamp(RelativeAcceleration.X / Data.MaxAcceleration, -1.0f, 1.0f);
		RelativeAccelAmt.Y = FMath::Clamp(RelativeAcceleration.Y / Data.MaxAcceleration, -1.0f, 1.0f);
	}
	float LeanMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(165.f, 375.f), FVector2D(0.5f, 1.0f), Speed2D);
	LeanAmount.X = RelativeAccelAmt.Y * LeanMultiplier;

	if (bHasVelocity) LastNonZeroVelocity = Data.Velocity;

	// 5. 공중 체류 시간 (기존 로직 유지)
	const bool bIsAirborne = Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Mode_InAir) ||
		Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Movement_Falling);
	InAirTime = bIsAirborne ? (InAirTime + DeltaSeconds) : 0.f;
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
		UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(MMNode, ValidDatabases, EPoseSearchInterruptMode::InterruptOnDatabaseChange);
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

void FMGSLocomotionState::Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential, const FMGSTrajectoryHandler& Trajectory, const FMGSMotionMatchingHandler& MMHandler)
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
	
	// 2. 공용 변수 계산 (재사용)
	const bool bIsIdle = (MovementStateTag == MGSGameplayTags::State_Player_Movement_Idle);
	const bool bIsMoving = (MovementStateTag == MGSGameplayTags::State_Player_Movement_Moving);
	const bool bWasMoving = (LastFrameMovementTag == MGSGameplayTags::State_Player_Movement_Moving);

	// [재사용] 캡슐 방향과 메쉬(루트) 방향의 차이
	FRotator DeltaRot = (Data.OrientationIntent - Essential.RootTransform.GetRotation().Rotator()).GetNormalized();
	const float AbsYawDiff = FMath::Abs(DeltaRot.Yaw);

	// [재사용] 조준 여부 및 스틱 튕기기
	const bool bWantsToAim = Data.GameplayTags.HasTag(MGSGameplayTags::State_Player_Aiming);
	const bool bStickFlick = bIsIdle && bWasMoving;

	// 3. 각 특수 상태 판정 (조건들)

	// A. Starting (급출발)
	const bool bIsAcceleratingStart = (Trajectory.Trj_FutureVelocity.Size2D() >= Essential.Speed2D + 100.0f);
	const bool bIsNotPivotingDB = !MMHandler.CurrentDatabaseTags.Contains(FName("Pivots"));
	const bool bIsStarting = bIsMoving && bIsAcceleratingStart && bIsNotPivotingDB;

	// B. Spin Transition (이동 중 급회전) - 기존 AbsYawDiff 재사용
	const bool bShouldSpin = bIsMoving && (AbsYawDiff >= 130.0f) && (Essential.Speed2D >= 150.0f) && bIsNotPivotingDB;
	
	// C. 제자리 회전
	const bool bOverThreshold = AbsYawDiff > 50.0f;
	
	// 파쿠르 상태 판정
	bool bJustTraversed = false;
	if (AnimInstance)
	{
		// default 슬롯 비활성화 ( 슬롯 default로 놓을건지 체크 )
		const bool bSlotNotActive = !AnimInstance->IsSlotActive(FName("DefaultSlot"));
		// movingtraversal 커브값이 0보다 큼
		const bool bIsTraversalCurveActive = AnimInstance->GetCurveValue(FName("MovingTraversal")) > 0.0f;

		// TrajectoryHandler의 함수를 호출하며 Acceleration과 Velocity를 전달
		const float TurnAngle = Trajectory.GetTrajectoryTurnAngle(Data.InputAcceleration, Data.Velocity);
		const bool bIsGoingStraight = FMath::Abs(TurnAngle) <= 50.0f;

		bJustTraversed = bSlotNotActive && bIsTraversalCurveActive && bIsGoingStraight;
	}
	
	
	bool bIsPivoting = CheckIsPivoting(Data, Essential, Trajectory);
	
	if (bIsPivoting)
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_Pivoting;
	}
	else if (bShouldSpin)
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_SpinTransition;
	}
	else if (bIsStarting)
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_Starting;
	}
	else if (bJustTraversed)
	{
		LocomotionActionTag = MGSGameplayTags::State_Player_Movement_FromTraversal;
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
