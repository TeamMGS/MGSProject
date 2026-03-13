/*
 * 파일명 : MGSAnimInstanceTypes.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-11
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
#include "Components/Combat/PawnCombatComponent.h"
#include "Weapon/BaseWeapon.h"

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

void FMGSEssentialValues::Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, float DeltaSeconds)
{
	if (DeltaSeconds <= 0.f) return;

	// 기초 물리 계산
	const float AccelSize = Data.InputAcceleration.Size();
	AccelerationAmount = (Data.MaxAcceleration > 0.f) ? (AccelSize / Data.MaxAcceleration) : 0.f;
	bHasAcceleration = AccelerationAmount > 0.f;
	Speed2D = Data.Velocity.Size2D();
	bHasVelocity = Speed2D > 5.0f;

	const FVector VelocityAcceleration = (Data.Velocity - Velocity_LastFrame) / DeltaSeconds;
	Velocity_LastFrame = Data.Velocity;
	
	// 기준점 설정 
	// body의 기준을 캡슐(Actor)의 Yaw로 잡습니다.
	float BaseActorYaw = Data.ActorTransform.Rotator().Yaw;
	float MeshTwistYaw = 0.0f; 

	if (AnimInstance)
	{
		int32 NodeIndex = INDEX_NONE;
		if (IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass()))
		{
			if (const FAnimSubsystem_Tag* TagSubsystem = AnimClass->FindSubsystem<FAnimSubsystem_Tag>())
			{
				NodeIndex = TagSubsystem->FindNodeIndexByTag(FName("OffsetRoot"));
			}
		}

		if (NodeIndex != INDEX_NONE)
		{
			FAnimNodeReference OffsetRootNode = UAnimExecutionContextLibrary::GetAnimNodeReference(AnimInstance, NodeIndex);
			// 여기서 가져오는 값은 '상대적 회전량'입니다.
			FTransform RelOffset = UAnimationWarpingLibrary::GetOffsetRootTransform(OffsetRootNode);
			MeshTwistYaw = RelOffset.Rotator().Yaw;
		}
	}

	// 몸통 방향(Body)
	float CurrentBodyWorldYaw;
	float TargetBodyYaw = FRotator::NormalizeAxis(BaseActorYaw + MeshTwistYaw);

	// 이동 중인지 판정 
	if (Speed2D > 5.0f)
	{
		// 이동 중이면 몸통이 진행 방향을 자연스럽게 따라갑니다.
		CurrentBodyWorldYaw = TargetBodyYaw;
	}
	else
	{
		// 멈춰있을 때는 이전 프레임의 몸통 방향을 그대로 유지
		CurrentBodyWorldYaw = LastBodyWorldYaw;
	}

	// 다음 프레임을 위해 현재 값을 저장
	LastBodyWorldYaw = CurrentBodyWorldYaw;
	
	// 에임 오프셋 계산
	if (AnimInstance)
	{
		float CameraYaw = Data.AimingRotation.Yaw;

		// 몸통의 진짜 앞방향 보정 (-90도)
		// 대부분의 UE 캐릭터는 -90도가 정면입니다.
		// 만약 고개가 정면에서 90도 꺾여 보인다면 아래 -90.0f를 0.0f나 +90.0f로 바꾸세요.
		float FinalYaw = FRotator::NormalizeAxis(CameraYaw - (CurrentBodyWorldYaw));
		float PitchOffset = 0.0f;
		if (Data.GameplayTags.HasTag(FGameplayTag::RequestGameplayTag("State.Character.WeaponEquipped.Primary")))
		{
			PitchOffset = 0.0f; // 총구가 너무 위를 보면 이 값을 더 낮추세요.
		}
		float FinalPitch = FRotator::NormalizeAxis(Data.AimingRotation.Pitch + PitchOffset);
		float DisableAO = AnimInstance->GetCurveValue(FName("Disable_AO"));

		// 최종 대입
		AOValue.X = FMath::Lerp(FRotator::NormalizeAxis(FinalYaw), 0.0f, DisableAO);
		AOValue.Y = FMath::Lerp(FinalPitch, 0.0f, DisableAO);
	}

	// 시각적 보정 (RootTransform 업데이트)
	FRotator VisualRot = FRotator(0, CurrentBodyWorldYaw, 0);
	RootTransform.SetLocation(Data.ActorTransform.GetLocation());
	RootTransform.SetRotation(VisualRot.Quaternion());

	// 상대 가속도 계산
	FRotator LeanRefRot = VisualRot;
	LeanRefRot.Yaw = FRotator::NormalizeAxis(LeanRefRot.Yaw + 90.0f);
	RelativeAcceleration = LeanRefRot.Quaternion().UnrotateVector(VelocityAcceleration);
	
	LastFrameActorYaw = Data.ActorTransform.Rotator().Yaw;
	FVector RelativeAccelAmt = FVector::ZeroVector;
	if (Data.MaxAcceleration > 0.f)
	{
		RelativeAccelAmt.X = FMath::Clamp(RelativeAcceleration.X / Data.MaxAcceleration, -1.0f, 1.0f);
		RelativeAccelAmt.Y = FMath::Clamp(RelativeAcceleration.Y / Data.MaxAcceleration, -1.0f, 1.0f);
	}
	float LeanMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(165.f, 375.f), FVector2D(0.5f, 1.0f), Speed2D);
	LeanAmount.X = RelativeAccelAmt.Y * LeanMultiplier;

	if (bHasVelocity) LastNonZeroVelocity = Data.Velocity;

	// 공중 체류 시간
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
	const bool bIsAcceleratingStart = (Trajectory.Trj_FutureVelocity.Size2D() >= Essential.Speed2D + 250.0f);
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

// 03.10 수정 후
bool FMGSLocomotionState::CheckIsPivoting(const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential, const FMGSTrajectoryHandler& Trajectory)
{
	// 1. 움직이는 중이 아니면 피벗도 아님
	if (MovementStateTag != MGSGameplayTags::State_Player_Movement_Moving) return false;

	// 2. 입력이 없으면 피벗이 아님
	FVector DesiredMoveDir = Data.InputAcceleration.GetSafeNormal2D();
	if (DesiredMoveDir.IsNearlyZero()) return false;

	// 3. [핵심] 현재 진행 방향(관성)과 입력 방향(의도)을 비교
	// 캡슐이 아무리 빨리 돌아도 속도 벡터(Velocity)는 관성 때문에 천천히 변합니다.
	FVector CurrentMoveDir = Data.Velocity.GetSafeNormal2D();

	// 만약 속도가 너무 낮다면(출발 직후 등) 캡슐 정면을 대신 사용
	if (Data.Velocity.SizeSquared2D() < 10000.f) // 약 100cm/s 미만
	{
		CurrentMoveDir = Data.ActorTransform.GetUnitAxis(EAxis::X);
	}

	float Dot = FVector::DotProduct(CurrentMoveDir, DesiredMoveDir);
	float TurnAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

	// 4. 임계값 (테스트를 위해 90도 설정)
	float AngleThreshold = 90.0f;
	bool bIsPivoting = TurnAngle > AngleThreshold;

	return bIsPivoting;
}

void FMGSWeaponState::Update(const class ABaseCharacter* Character, bool bIsWeaponEquipped, bool bIsSprinting, bool bIsReloading, bool bIsWeaponEquipping, float DeltaSeconds)
{
	if (!Character) return;

	if (!Character) return;
	
	// 목표가 되는 Alpha 값 (Target Alpha)
	float TargetAlpha = (bIsWeaponEquipped && !bIsSprinting && !bIsReloading && !bIsWeaponEquipping) ? 1.0f : 0.0f;

	// 현재 Alpha에서 목표 Alpha로 부드럽게 보간
	// InterpSpeed를 5.0~8.0 정도로 잡으면 애니메이션 블렌드 시간(0.2~0.3초)과 비슷하게 맞아떨어집니다.
	FinalIKAlpha = FMath::FInterpTo(FinalIKAlpha, TargetAlpha, DeltaSeconds, 3.0f);

	// 무기 데이터 업데이트 로직 (기존과 동일)
	if (bIsWeaponEquipped)
	{
		if (UPawnCombatComponent* CombatComp = Character->GetPawnCombatComponent())
		{
			if (ABaseWeapon* CurrentWeapon = CombatComp->GetCharacterCurrentEquippedWeapon())
			{
				LeftHandIKOffset = CurrentWeapon->GetWeaponData().LeftHandIKOffset;
			}
		}
	}
}
