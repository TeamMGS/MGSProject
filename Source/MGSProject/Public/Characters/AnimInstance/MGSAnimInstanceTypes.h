/*
 * 파일명 : MGSAnimInstanceTypes.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-05
 */
#pragma once

#include "CoreMinimal.h"
#include "MGSEnumType.h"
#include "Animation/TrajectoryTypes.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Animation/AnimNodeReference.h"
#include "MGSAnimInstanceTypes.generated.h"

class UPoseSearchDatabase;

/**
 * 0. 입력 상태 데이터 (GAS 태그 연동용)
 */
USTRUCT(BlueprintType)
struct FMGSInputState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToSprint = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToWalk = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToAim = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToCrouch = false;
};

/**
 * 1. 캐릭터 원본 데이터 프록시
 * (캐릭터 -> 애니메이션 스레드로 데이터를 복사해오는 창구)
 */
USTRUCT(BlueprintType)
struct FMGSCharacterDataProxy
{
	GENERATED_BODY()

	// 입력 상태 추가 (구조체 통합)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FMGSInputState InputState;
	
	// 상태 관련 (Enum 활용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSMovementMode MovementMode = EMGSMovementMode::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSStance Stance = EMGSStance::Stand;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSGait Gait = EMGSGait::Run;

	// 물리 데이터 (GASP 노드 대응)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FTransform ActorTransform = FTransform::Identity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Velocity = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector InputAcceleration = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float MaxAcceleration = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float MaxDeceleration = 0.f;

	// 회전 및 지면 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FRotator OrientationIntent = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FRotator AimingRotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector GroundNormal = FVector::UpVector;
	
	// 캐릭터의 회전 방식 (OrientToMovement 여부 판정용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSRotationMode RotationMode = EMGSRotationMode::VelocityDirection;
	
	void Update(class ABaseCharacter* Character);
};

/**
 * 2. 핵심 물리/수학 가공 데이터
 * (원본 데이터를 계산하여 Lean, Speed 등 실질적 값을 만듦)
 */
USTRUCT(BlueprintType)
struct FMGSEssentialValues
{
	GENERATED_BODY()

	// 기본 가공 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Speed2D = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float AccelerationAmount = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bHasAcceleration = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bHasVelocity = false;

	// 트랜스폼 및 회전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector2D LeanAmount = FVector2D::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FTransform RootTransform = FTransform::Identity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector RelativeAcceleration = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector LastNonZeroVelocity = FVector::ForwardVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector2D AOValue = FVector2D::ZeroVector;

	// 계산을 위한 이전 프레임 저장용 (내부 변수)
	FVector Velocity_LastFrame = FVector::ZeroVector;
	FTransform CharacterTransform_LastFrame = FTransform::Identity;

	// GASP 로직을 C++로 이식한 업데이트 함수
	void Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, float DeltaSeconds);
};

/**
 * 3. 모션 매칭 제어
 * (Chooser 결과 및 현재 사용 중인 데이터베이스 정보)
 */
USTRUCT(BlueprintType)
struct FMGSMotionMatchingHandler
{
	GENERATED_BODY()

	// --- 검색 설정 데이터 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<class UChooserTable> MainChooserTable;

	// --- 런타임 캐싱 데이터 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UPoseSearchDatabase>> ValidDatabases;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FName> CurrentDatabaseTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> CurrentSelectedAnim;
	
	
	void Update(UAnimInstance* AnimInstance, const FAnimNodeReference& Node);
	void PostSelection(UAnimInstance* AnimInstance, const FAnimNodeReference& Node);
};


/**
 * 4. 궤적 핸들러
 */
USTRUCT(BlueprintType)
struct FMGSTrajectoryHandler
{
	GENERATED_BODY()

	// --- 결과 저장 변수 (ABP 노드에 연결될 최종 데이터) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTransformTrajectory Trajectory;

	// --- 내부 상태 저장용 (GASP 변수 대응) ---
	float DesiredControllerYawLastUpdate = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Trj_PastVelocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Trj_CurrentVelocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Trj_FutureVelocity;

	// --- 해결책: 클래스 에셋 대신 구조체를 멤버로 가짐 ---
	// 에디터의 상세(Details) 패널에서 직접 속도 조절, 회전 속도 등을 설정할 수 있게 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FPoseSearchTrajectoryData TrajectorySettings_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FPoseSearchTrajectoryData TrajectorySettings_Moving;
	
	// 핵심 업데이트 함수
	void Update(class UAnimInstance* AnimInstance, const FMGSEssentialValues& Values, float DeltaSeconds);
};

/**
 * 5. 로코모션 상태 관리 핸들러
 * (MovementMode, Gait, Stance, RotationMode 및 이전 프레임 기록)
 */
USTRUCT(BlueprintType)
struct FMGSLocomotionState
{
	GENERATED_BODY()

	// 현재 프레임 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSMovementMode MovementMode = EMGSMovementMode::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSStance Stance = EMGSStance::Stand;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSGait Gait = EMGSGait::Run;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EMGSRotationMode RotationMode = EMGSRotationMode::VelocityDirection;

	// 이전 프레임 상태 (Last Frame)
	EMGSMovementMode MovementMode_LastFrame = EMGSMovementMode::None;
	EMGSStance Stance_LastFrame = EMGSStance::Stand;
	EMGSGait Gait_LastFrame = EMGSGait::Run;
	EMGSRotationMode RotationMode_LastFrame = EMGSRotationMode::VelocityDirection;

	// 특수 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bIsMoving = false;

	// 핵심 업데이트 함수
	void Update(const FMGSCharacterDataProxy& Data, const FMGSTrajectoryHandler& Trajectory);
};
