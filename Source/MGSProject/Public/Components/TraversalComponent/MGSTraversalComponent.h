/*
 * 파일명 : MGSTraversalComponent.h
 * 생성자 : 김동석
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-13
 */
#pragma once

#include "CoreMinimal.h"
#include "MGSEnumType.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearchHistory.h"
#include "MGSTraversalComponent.generated.h"

// 파쿠르 감지 트레이스(Trace)를 위한 물리 세팅
USTRUCT(BlueprintType)
struct FMGSTraversalTraceSettings
{
	GENERATED_BODY()

	// 캐릭터 앞방향
	UPROPERTY() FVector ForwardDirection = FVector::ZeroVector;
	// 속도가 0일때 감지거리 75, 속도가 500일때 감지거리 350 clamp
	UPROPERTY() float ForwardDistance = 0.0f;
	// 캡슐 반지름 (기본값 30)
	UPROPERTY() float Radius = 30.f;
	// 지상에서는 조금 짧게(60) ,공중에서는 더 긴 캡슐(86)로 위아래로 훑음
	UPROPERTY() float HalfHeight = 0.0f;
	// 시작 위치 오프셋
	UPROPERTY() FVector OriginOffset = FVector::ZeroVector;
	// 끝 위치 오프셋 (GASP: 공중 상태일 때 Z축 보정용)
	UPROPERTY() FVector EndOffset = FVector::ZeroVector;
};


// 파쿠르 환경 분석 결과 데이터
USTRUCT(BlueprintType)
struct FMGSTraversalCheckResult
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	// 캐릭터 정렬을 위해 벽이 바라보는 방향 (Forward Trace의 ImpactNormal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	// 얇은 벽인지 판별
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector BackLedgeLocation = FVector::ZeroVector;

	// 장애물 너머 착지 바닥
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector BackFloorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float BackLedgeHeight = 0.0f;
};

// Chooser Table에서 파쿠르 애니메이션을 고르기 위한 입력 데이터
USTRUCT(BlueprintType)
struct FMGSTraversalChooserInputs
{
	GENERATED_BODY()

	// 분석된 액션 타입 (Hurdle, Vault, Mantle 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	EMGSTraversalActionType ActionType = EMGSTraversalActionType::None;

	// 환경 분석 결과 (True/False)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackFloor = false;

	// 장애물 물리 수치 (가장 중요)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float BackLedgeHeight = 0.0f;

	// 캐릭터 상태 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float DistanceToLedge = 0.0f; // 장애물과의 수평 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	EMGSMovementMode MovementMode = EMGSMovementMode::OnGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	EMGSGait Gait = EMGSGait::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float Speed = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Location")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Location")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Location")
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Location")
	FVector BackFloorLocation = FVector::ZeroVector;

	// GASP 대응: 포즈 매칭을 위한 히스토리 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FPoseHistoryReference PoseHistory;
};


UCLASS(meta=(BlueprintSpawnableComponent) )
class MGSPROJECT_API UMGSTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGSTraversalComponent();

	// GA_Traversal에서 호출할 핵심 입구
	UFUNCTION(BlueprintCallable, Category = "MGS|Traversal")
	bool CheckTraversal(FMGSTraversalChooserInputs& OutInputs);
	
	// Chooser가 읽어갈 수 있도록 데이터를 보관하는 변수
	UPROPERTY(BlueprintReadOnly, Category = "MGS|Traversal")
	FMGSTraversalChooserInputs CurrentTraversalInputs;

protected:
	virtual void BeginPlay() override;

private:
	// 전방 벽 확인
	bool FindWall(FHitResult& OutWallHit, FVector& OutTraceDirection);
	
	// 디버깅?
	UPROPERTY(EditAnywhere, Category = "MGS|Traversal|Debug")
	bool bDrawDebug = true;
	
	// front ledge 확인
	bool FindFrontLedge(const FHitResult& WallHit, const FVector& TraceDirection, FVector& OutLedgeLocation, FVector& OutLedgeNormal);
	// back ledge 확인
	bool FindBackLedge(const FVector& FrontLedgeLocation, const FVector& TraceDirection, FVector& OutBackLedgeLocation, float& OutObstacleDepth);
	// back floor 확인
	bool FindBackFloor(const FVector& BackLedgeLocation, const FVector& TraceDirection, FVector& OutBackFloorLocation, float& OutBackLedgeHeight);
	// 장애물 위의 공간 확인
	bool CheckTopRoom(const FVector& FrontLedge, const FVector& BackLedge, float Radius, float HalfHeight, FHitResult& OutHit);
	
	// 어떤 액션을 할것인지
	EMGSTraversalActionType DetermineActionType(float Height, float Depth, bool bHasBackLedge, bool bHasBackFloor, float BackLedgeHeight) const;
	
	// 캐릭터 및 CMC 캐싱
	UPROPERTY()
	TObjectPtr<class ABaseCharacter> OwningCharacter;

	UPROPERTY()
	TObjectPtr<class UMGSCharacterMovementComponent> MGSMovementComponent;
};
