/*
 * 파일명 : MGSAnimInstanceTypes.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-09
 */
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MGSEnumType.h"
#include "Animation/TrajectoryTypes.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Animation/AnimNodeReference.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
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
	
	// 캐릭터의 모든 상태를 담을 마스터 태그 컨테이너
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayTagContainer GameplayTags;

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
	
	// 공중에 떠있는 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float InAirTime = 0.f;

	// 트랜스폼 및 회전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector2D LeanAmount = FVector2D::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FTransform RootTransform = FTransform::Identity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector RelativeAcceleration = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector LastNonZeroVelocity = FVector::ForwardVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector2D AOValue = FVector2D::ZeroVector;

	// 계산을 위한 이전 프레임 저장용 (내부 변수)
	FVector Velocity_LastFrame = FVector::ZeroVector;
	FTransform CharacterTransform_LastFrame = FTransform::Identity;
	float LastFrameActorYaw = 0.f;	

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
	
	float GetTrajectoryTurnAngle(const FVector& Acceleration, const FVector& Velocity) const
	{
		// 두 벡터 중 하나라도 거의 0이라면 계산할 의미가 없으므로 0 반환
		if (Acceleration.SizeSquared2D() < KINDA_SMALL_NUMBER || Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
		{
			return 0.0f;
		}

		// Acceleration(가속도)을 Rotator로 변환
		FRotator AccelRot = Acceleration.Rotation();

		// Velocity(속도)를 Rotator로 변환
		FRotator VelocityRot = Velocity.Rotation();

		// 두 회전값의 차이를 계산
		FRotator DeltaRot = (AccelRot - VelocityRot).GetNormalized();

		// Yaw 값 반환
		return DeltaRot.Yaw;
	}
	
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

	// 메인 이동 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag MovementStateTag;
	
	// 부가 액션 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag LocomotionActionTag;
	
	// 마지막 프레임 검사
	FGameplayTag LastFrameMovementTag;

	// 핵심 업데이트 함수
	void Update(UAnimInstance* AnimInstance, const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential, const FMGSTrajectoryHandler& Trajectory, const FMGSMotionMatchingHandler& MMHandler);
	
	// 헬퍼 함수
	bool CheckIsPivoting(const FMGSCharacterDataProxy& Data, const FMGSEssentialValues& Essential, const FMGSTrajectoryHandler& Trajectory);
};


USTRUCT(BlueprintType)
struct FMGSProceduralSettings
{
	GENERATED_BODY()

	// 기본 설정들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FFootPlacementPlantSettings PlantSettings_Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FFootPlacementInterpolationSettings InterpolationSettings_Default;

	// 멈춤(Stop) 전용 설정들 (더 민감한 접지용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FFootPlacementPlantSettings PlantSettings_Stops;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FFootPlacementInterpolationSettings InterpolationSettings_Stops;

	FMGSProceduralSettings()
	{
		// 초기화 로직 (에디터에서 수정 가능하도록 기본값 세팅)
	}
};