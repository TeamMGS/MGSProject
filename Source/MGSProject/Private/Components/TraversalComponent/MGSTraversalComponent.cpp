/*
 * 파일명 : MGSTraversalComponent.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-16
 */

#include "Components/TraversalComponent/MGSTraversalComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/MovementComponent/MGSCharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/AnimNode_PoseSearchHistoryCollector.h"

UMGSTraversalComponent::UMGSTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMGSTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningCharacter = Cast<ABaseCharacter>(GetOwner());
	if (OwningCharacter)
	{
		MGSMovementComponent = Cast<UMGSCharacterMovementComponent>(OwningCharacter->GetCharacterMovement());
	}
}

bool UMGSTraversalComponent::CheckTraversal(FMGSTraversalChooserInputs& OutInputs)
{
	if (!OwningCharacter || !MGSMovementComponent) return false;

	// 단계별 지형 분석 시작

	FHitResult WallHit;
	FVector TraceDir;
	// 내 앞에 벽이 있는가?
	if (!FindWall(WallHit, TraceDir)) return false;

	FVector FrontLedgeLoc, FrontLedgeNormal;
	// 벽의 꼭대기(높이)는 어디인가?
	if (!FindFrontLedge(WallHit, TraceDir, FrontLedgeLoc, FrontLedgeNormal)) return false;

	FVector BackLedgeLoc;
	float ObstacleDepth = 0.0f;
	// 장애물의 두께는 얼마인가? 
	bool bHasBackLedge = FindBackLedge(FrontLedgeLoc, TraceDir, BackLedgeLoc, ObstacleDepth);

	// 장애물 위쪽 공간 확인
	FHitResult RoomHit;
	bool bFinalHasBackLedge = bHasBackLedge;
	float FinalDepth = ObstacleDepth;

	if (bHasBackLedge)
	{
		float Radius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		float HalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		// 캡슐 스윕으로 경로상 천장이나 벽 확인
		if (CheckTopRoom(FrontLedgeLoc, BackLedgeLoc, Radius, HalfHeight, RoomHit))
		{
			// [공간 부족] -> 뒤쪽 모서리 무효화 및 두께를 충돌 지점까지로 제한
			bFinalHasBackLedge = false;
			FinalDepth = (RoomHit.ImpactPoint - FrontLedgeLoc).Size2D();
		}
	}

	FVector BackFloorLoc;
	float BackLedgeHeight = 0.0f;
	// 장애물 너머에 착지할 바닥이 있는가
	bool bHasBackFloor = FindBackFloor(bFinalHasBackLedge ? BackLedgeLoc : FrontLedgeLoc, TraceDir, BackFloorLoc,
	                                   BackLedgeHeight);

	// 최종 ActionType 결정 
	float ObstacleHeight = FrontLedgeLoc.Z - (OwningCharacter->GetActorLocation().Z - OwningCharacter->
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	EMGSTraversalActionType FinalActionType = DetermineActionType(ObstacleHeight, FinalDepth, bFinalHasBackLedge,
	                                                              bHasBackFloor, BackLedgeHeight);

	if (FinalActionType == EMGSTraversalActionType::None) return false;

	// --- 최종 데이터 패키징 (Chooser Table 입력용) ---

	OutInputs.ActionType = FinalActionType;
	OutInputs.bHasFrontLedge = true;
	OutInputs.bHasBackLedge = bFinalHasBackLedge; // 보정된 값 전달
	OutInputs.bHasBackFloor = bHasBackFloor;

	OutInputs.ObstacleHeight = ObstacleHeight;
	OutInputs.ObstacleDepth = FinalDepth; // 보정된 값 전달
	OutInputs.BackLedgeHeight = BackLedgeHeight;

	// 캐릭터 상태 정보
	OutInputs.DistanceToLedge = FVector::Dist2D(OwningCharacter->GetActorLocation(), FrontLedgeLoc);
	OutInputs.Speed = OwningCharacter->GetVelocity().Size2D();
	OutInputs.MovementMode = (MGSMovementComponent->IsFalling()) ? EMGSMovementMode::InAir : EMGSMovementMode::OnGround;

	OutInputs.FrontLedgeLocation = FrontLedgeLoc;
	OutInputs.FrontLedgeNormal = FrontLedgeNormal;
	OutInputs.BackLedgeLocation = BackLedgeLoc;
	OutInputs.BackFloorLocation = BackFloorLoc;

	// [추가] GASP 대응: 포즈 히스토리 데이터 채우기
	if (UAnimInstance* AnimInst = OwningCharacter->GetMesh()->GetAnimInstance())
	{
		if (const auto* HistoryNode = UPoseSearchLibrary::FindPoseHistoryNode(FName("PoseHistory"), AnimInst))
		{
			// 1. 데이터를 안전하게 복사할 Archived 객체 생성 (SharedPtr)
			TSharedRef<UE::PoseSearch::FArchivedPoseHistory> ArchivedHistory = MakeShared<UE::PoseSearch::FArchivedPoseHistory>();

			// 2. 현재 노드의 히스토리 상태를 스냅샷으로 복사
			ArchivedHistory->InitFrom(&HistoryNode->GetPoseHistory());

			// 3. 구조체에 할당 (TSharedPtr<IPoseHistory>로 자동 업캐스팅됨)
			OutInputs.PoseHistory.PoseHistory = ArchivedHistory;
			// [추가] PoseHistory 유효성 검사 로그
			UE_LOG(LogTemp, Warning, TEXT("[Traversal] PoseHistory populated! Entries: %d"), ArchivedHistory->GetNumEntries());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Traversal] Failed to find PoseHistoryCollector node with Tag 'PoseHistory'!"));
		}
	}
	else
	{
		// 이 로그가 뜬다면 메쉬에 AnimInstance가 할당되지 않은 상태입니다.
		UE_LOG(LogTemp, Error, TEXT("[Traversal] Critical: AnimInstance is NULL!"));
	}
	// 컴포넌트 내부 변수에 데이터 저장 (Chooser가 읽어갈 수 있도록)
	CurrentTraversalInputs = OutInputs;
	
	return true;
}

bool UMGSTraversalComponent::FindWall(FHitResult& OutWallHit, FVector& OutTraceDirection)
{
	if (!OwningCharacter || !MGSMovementComponent) return false;

	// 트레이스 방향 설정
	OutTraceDirection = OwningCharacter->GetActorForwardVector();
	float ForwardDistance = 0.0f;

	// 속도에 따른 가변 거리 계산 
	// 속도가 0일 때 75cm, 속도가 500일 때 150cm 정도로 동적 조절
	float CurrentSpeed = OwningCharacter->GetVelocity().Size2D();
	// [수정] 공중 상태(Falling)와 지상 상태(Walking)의 감지 거리 차별화
	if (MGSMovementComponent->IsFalling())
	{
		// 공중일 때는 훨씬 짧은 거리에서만 감지 (예: 50cm ~ 100cm)
		// 점프 중 의도치 않게 먼 벽을 잡는 것을 방지합니다.
		ForwardDistance = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 500.f),
			FVector2D(50.f, 100.f),
			CurrentSpeed
		);
	}
	else
	{
		// 지상일 때는 기존 거리 유지 (예: 150cm ~ 250cm)
		ForwardDistance = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 500.f),
			FVector2D(150.f, 250.f),
			CurrentSpeed
		);
	}
	// traversal channel 설정
	ECollisionChannel TraversalChannel = ECC_GameTraceChannel1;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningCharacter);

	// 트레이스 시작/끝 지점 설정
	FVector Start = OwningCharacter->GetActorLocation();
	FVector End = Start + (OutTraceDirection * ForwardDistance);

	// 캡슐 트레이스 설정 (캐릭터 캡슐보다 약간 작게 설정)
	float TraceRadius = 30.f;
	float TraceHalfHeight = 60.f;

	bool bHit = GetWorld()->SweepSingleByChannel(
		OutWallHit,
		Start,
		End,
		FQuat::Identity,
		TraversalChannel,
		FCollisionShape::MakeCapsule(TraceRadius, TraceHalfHeight),
		Params
	);

	// 디버그 드로잉
	if (bDrawDebug)
	{
		DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, TraceHalfHeight + (ForwardDistance * 0.5f), TraceRadius,
		                 FRotationMatrix::MakeFromZ(OutTraceDirection).ToQuat(), bHit ? FColor::Green : FColor::Red,
		                 false, 1.0f);
	}

	// 벽 판정 검증
	if (bHit && OutWallHit.GetActor())
	{
		// 수직에 가까운 벽인지 확인
		float WallAngle = FMath::RadiansToDegrees(
			FMath::Acos(FVector::DotProduct(OutWallHit.ImpactNormal, FVector::UpVector)));

		// 약 75도 이상의 가파른 면만 '벽'으로 인정
		if (WallAngle > 75.0f)
		{
			return true;
		}
	}

	return false;
}

bool UMGSTraversalComponent::FindFrontLedge(const FHitResult& WallHit, const FVector& TraceDirection,
                                            FVector& OutLedgeLocation, FVector& OutLedgeNormal)
{
	if (!OwningCharacter) return false;

	// 수직 스캔 기준점 (X, Y)
	FVector ScanTarget = WallHit.ImpactPoint + (TraceDirection * 15.0f);

	// 수직 트레이스 시작/끝 지점 (Z축)
	float CapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float CharacterBaseZ = OwningCharacter->GetActorLocation().Z - CapsuleHalfHeight;

	// 300 높이까지 확인
	FVector VerticalStart = ScanTarget;
	VerticalStart.Z = CharacterBaseZ + 300.0f;

	// 발바닥보다 살짝 위까지만확인
	FVector VerticalEnd = ScanTarget;
	VerticalEnd.Z = CharacterBaseZ + 30.0f;

	// 트레이스 설정
	ECollisionChannel TraversalChannel = ECC_GameTraceChannel1; // Traversal 전용 채널
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningCharacter);

	FHitResult LedgeHit;

	// LineTrace 대신 반경 10cm의 SphereTrace를 사용하여 "손으로 짚을 만한 면적"이 있는지 안정적으로 찾습니다.
	bool bHit = GetWorld()->SweepSingleByChannel(
		LedgeHit,
		VerticalStart,
		VerticalEnd,
		FQuat::Identity,
		TraversalChannel,
		FCollisionShape::MakeSphere(10.0f),
		Params
	);

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), VerticalStart, 10.f, 8, FColor::Yellow, false, 1.0f);
		if (bHit) DrawDebugPoint(GetWorld(), LedgeHit.ImpactPoint, 15.0f, FColor::Cyan, false, 1.0f);
	}

	// 결과 검증 및 데이터 저장
	if (bHit)
	{
		// 평평한 면인지 확인 (법선 벡터가 위쪽을 향하는지)
		float FloorAngle = FMath::RadiansToDegrees(
			FMath::Acos(FVector::DotProduct(LedgeHit.ImpactNormal, FVector::UpVector)));

		if (FloorAngle < 35.0f)
		{
			OutLedgeLocation = LedgeHit.ImpactPoint;
			OutLedgeNormal = LedgeHit.ImpactNormal;

			// 최종 높이 계산 및 검증
			float FinalHeight = OutLedgeLocation.Z - CharacterBaseZ;

			if (FinalHeight <= 275.0f)
			{
				return true;
			}
		}
	}
	return false;
}

bool UMGSTraversalComponent::FindBackLedge(const FVector& FrontLedgeLocation, const FVector& TraceDirection,
	FVector& OutBackLedgeLocation, float& OutObstacleDepth)
{
	// 스캔 설정: 15cm 간격으로 최대 10번 스캔 
	const float ScanStep = 15.0f;
	const int32 MaxIterations = 10;

	// 시작 Z 높이는 FrontLedge보다 살짝 위에서 시작합니다.
	float RayStartZ = FrontLedgeLocation.Z + 30.0f;

	// FrontLedge의 Z값을 기준으로 어느 정도까지를 "같은 평면(장애물 위쪽)"으로 볼 것인지 오차 범위 설정
	const float HeightTolerance = 25.0f;

	// 트레이스 설정
	ECollisionChannel TraversalChannel = ECC_GameTraceChannel1;
	FCollisionQueryParams Params;
	if (OwningCharacter) Params.AddIgnoredActor(OwningCharacter);

	// 반복 스캔 시작
	FVector CurrentScanOrigin = FrontLedgeLocation;
	FVector LastValidHitPoint = FrontLedgeLocation;
	bool bFoundBackEdge = false;

	for (int32 i = 1; i <= MaxIterations; ++i)
	{
		// 전방으로 ScanStep 만큼 전진
		CurrentScanOrigin += (TraceDirection * ScanStep);

		FVector Start = CurrentScanOrigin;
		Start.Z = RayStartZ;

		// 레이 길이는 FrontLedge 기준 아래로 100cm 정도만 쏩니다
		FVector End = CurrentScanOrigin;
		End.Z = FrontLedgeLocation.Z - 100.0f;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			TraversalChannel,
			Params
		);

		if (bDrawDebug)
		{
			DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f);
		}

		// 레이가 아무것도 맞추지 못한 경우 (장애물이 끝남 -> 절벽)
		if (!bHit)
		{
			bFoundBackEdge = true;
			break;
		}

		// 맞긴 맞았는데, 높이가 FrontLedge 높이보다 확 낮아진 경우
		if (FMath::Abs(HitResult.ImpactPoint.Z - FrontLedgeLocation.Z) > HeightTolerance)
		{
			bFoundBackEdge = true;
			break;
		}

		// 여전히 장애물 윗면을 맞춘 경우 
		LastValidHitPoint = HitResult.ImpactPoint;
	}

	// 결과 정산
	if (bFoundBackEdge)
	{
		// 반복문이 끊겼다는 건 장애물이 끝났다는 의미입니다.
		// 끊기기 직전의 유효했던 타격 지점이 뒤쪽 모서리가 됩니다.
		OutBackLedgeLocation = LastValidHitPoint;

		// 두께(Depth) 계산:
		// 3D 거리(Size)가 아닌 XY 평면 상의 2D 거리(Size2D)로 계산해야 정확한 물리적 두께가 나옵니다.
		OutObstacleDepth = (OutBackLedgeLocation - FrontLedgeLocation).Size2D();
		return true;
	}
	OutBackLedgeLocation = FrontLedgeLocation; // 의미 없는 값
	OutObstacleDepth = ScanStep * MaxIterations; // 매우 큰 값 반환 (Mantle 발동 유도)

	return false; // 확실한 BackLedge를 찾진 못함 (하지만 두께가 두껍다는 건 알아냄)
}

bool UMGSTraversalComponent::FindBackFloor(const FVector& BackLedgeLocation, const FVector& TraceDirection,
	FVector& OutBackFloorLocation, float& OutBackLedgeHeight)
{
	if (!OwningCharacter) return false;

	// 바닥 스캔을 위한 시작점 계산
	FVector FloorScanTarget = BackLedgeLocation + (TraceDirection * 35.0f);

	// 수직 트레이스 시작/끝 지점 설정
	FVector Start = FloorScanTarget;
	Start.Z = BackLedgeLocation.Z + 20.0f; // 모서리보다 살짝 위에서 시작

	FVector End = FloorScanTarget;
	// 얼마나 깊은 곳까지 착지 가능한 바닥으로 인정할 것인가?
	End.Z = BackLedgeLocation.Z - 250.0f;

	// 트레이스 설정
	ECollisionChannel TraversalChannel = ECC_GameTraceChannel1;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningCharacter);

	FHitResult FloorHit;
	// 발이 닿을 평평한 면을 찾기 위해 SphereTrace 사용
	bool bHit = GetWorld()->SweepSingleByChannel(
		FloorHit,
		Start,
		End,
		FQuat::Identity,
		TraversalChannel,
		FCollisionShape::MakeSphere(15.0f), // 발 크기 정도의 구체
		Params
	);

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), Start, 15.f, 8, FColor::Magenta, false, 1.0f);
		if (bHit) DrawDebugPoint(GetWorld(), FloorHit.ImpactPoint, 15.0f, FColor::Orange, false, 1.0f);
	}

	// 결과 검증
	if (bHit)
	{
		// 평평한 바닥인지 확인 (경사로에 착지하면 미끄러짐)
		float FloorAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FloorHit.ImpactNormal, FVector::UpVector)));

		// 착지 지점은 앞 모서리보다 좀 더 관대하게(약 45도) 경사를 허용할 수 있습니다.
		if (FloorAngle < 45.0f)
		{
			OutBackFloorLocation = FloorHit.ImpactPoint;

			// 4-2. BackLedgeHeight 계산 (Chooser Table 입력용)
			OutBackLedgeHeight = BackLedgeLocation.Z - OutBackFloorLocation.Z;

			return true;
		}
	}

	// 바닥을 찾지 못했거나 너무 가파른 경사인 경우
	// Chooser Table에서 HasBackFloor = False 로 들어가게 됩니다.
	OutBackFloorLocation = FVector::ZeroVector;
	OutBackLedgeHeight = 0.0f;
	return false;
}

bool UMGSTraversalComponent::CheckTopRoom(const FVector& FrontLedge, const FVector& BackLedge, float Radius, float HalfHeight, FHitResult& OutHit)
{
	// 시작점과 끝점 설정 (블루프린트의 HasRoomCheck_FrontLedge/BackLedge 대응)
	// 캐릭터가 장애물 위를 통과할 때의 높이로 보정합니다.
	FVector Start = FrontLedge + FVector(0, 0, HalfHeight + 2.0f);
	FVector End = BackLedge + FVector(0, 0, HalfHeight + 2.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningCharacter);

	// 캡슐 스윕으로 경로상 장애물 확인
	return GetWorld()->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel1, // Traversal 전용 채널
		FCollisionShape::MakeCapsule(Radius, HalfHeight),
		Params
	);
}

EMGSTraversalActionType UMGSTraversalComponent::DetermineActionType(float Height, float Depth, bool bHasBackLedge,
	bool bHasBackFloor, float BackLedgeHeight) const
{
	// 장애물 두께가 59cm를 초과하면 무조건 Mantle 
	if (Depth > 59.0f)
	{
		return EMGSTraversalActionType::Mantle;
	}

	// 장애물 두께가 얇은 경우 (0 ~ 59cm)
	if (bHasBackLedge)
	{
		// 너머에 착지할 바닥이 있는 경우
		if (bHasBackFloor)
		{
			// 바닥과의 높이 차이가 50cm 이상이면 뛰어넘는 Hurdle
			if (BackLedgeHeight >= 50.0f)
			{
				return EMGSTraversalActionType::Hurdle;
			}
			// 바닥과의 높이 차이가 거의 없으면(0~10cm) 걸릴 위험이 있으니 Mantle
			else if (BackLedgeHeight >= 0.0f && BackLedgeHeight <= 10.0f)
			{
				return EMGSTraversalActionType::Mantle;
			}

			// 그 외의 애매한 높이(10~50cm)는 Vault로 처리
			return EMGSTraversalActionType::Vault;
		}
		// 너머에 바로 착지할 바닥이 없는 경우 (낭떠러지 등)
		// 이미지 분석 결과: HasBackFloor (False) -> Vault (손 짚고 넘어가서 낙하)
		else
		{
			return EMGSTraversalActionType::Vault;
		}
	}

	// 위 조건에 해당하지 않거나 감지에 실패한 경우
	return EMGSTraversalActionType::None;
}
