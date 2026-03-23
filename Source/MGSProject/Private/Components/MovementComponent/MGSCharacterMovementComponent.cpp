/*
 * 파일명 : MGSCharacterMovementComponent.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-06
 * 수정자 : 김동석
 * 수정일 : 2026-03-11
 */

#include "Components/MovementComponent/MGSCharacterMovementComponent.h"
#include "Characters/BaseCharacter.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

void UMGSCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	MaxAcceleration = CalculateMaxAcceleration();
	BrakingDecelerationWalking = CalculateBrakingDeceleration();
	GroundFriction = CalculateGroundFriction();
	MaxWalkSpeed = CalculateMaxSpeed();
	MaxWalkSpeedCrouched = CalculateMaxCrouchSpeed();
	
	UpdateRotationMode();
}

// sprint, run, walk 상태 태그
FGameplayTag UMGSCharacterMovementComponent::GetDesiredGait() const
{
	// 정지 상태 체크
	if (Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
	{
		return FGameplayTag::EmptyTag;
	}

	// 캐릭터의 ASC 가져오기
	ABaseCharacter* BaseChar = Cast<ABaseCharacter>(GetCharacterOwner());
	if (!BaseChar) return MGSGameplayTags::State_Player_Gait_Run;

	UMGSAbilitySystemComponent* ASC = BaseChar->GetMGSAbilitySystemComponent();
	if (!ASC) return MGSGameplayTags::State_Player_Gait_Run;

	// 질주 태그가 있는가?
	if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Sprint) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Sprint))
	{
		return MGSGameplayTags::State_Player_Gait_Sprint;
	}

	// 걷기 태그가 있는가? 
	if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Movement_Walk) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Walk))
	{
		return MGSGameplayTags::State_Player_Gait_Walk;
	}

	// 아무 태그도 없다면 기본값은 '달리기(Run)'
	return MGSGameplayTags::State_Player_Gait_Run;
}

// 쉬프트를 누르고, 시야가 정면 50도 이내일때만 질주
bool UMGSCharacterMovementComponent::CanSprint() const
{
	FVector InputVector = PawnOwner->IsLocallyControlled() ?
					  PawnOwner->GetPendingMovementInputVector() : GetCurrentAcceleration();

	if (InputVector.IsNearlyZero()) return false;

	// Yaw 각도 차이 체크 (50도 이내)
	const float InputYaw = InputVector.ToOrientationRotator().Yaw;
	const float ActorYaw = GetCharacterOwner()->GetActorRotation().Yaw;
	const float DeltaYaw = FMath::Abs(FRotator::NormalizeAxis(InputYaw - ActorYaw));

	// '질주 의도' 태그 확인
	UMGSAbilitySystemComponent* ASC = Cast<ABaseCharacter>(GetCharacterOwner())->GetMGSAbilitySystemComponent();
	bool bWantsToSprint = ASC && ASC->HasMatchingGameplayTag(MGSGameplayTags::InputTag_Sprint);

	return (DeltaYaw < 50.0f) && bWantsToSprint;
}

// 속도를 서서히 늘리기 위한 가속도 증감 함수
float UMGSCharacterMovementComponent::CalculateMaxAcceleration() const
{
	// 현재 2D 속도 가져오기 (GASP의 VSizeXY)
	const float CurrentSpeed2D = Velocity.Size2D();

	// 속도에 비례한 가속도 매핑
	// 속도가 300 미만이면 가속도 800
	// 속도가 700 이상이면 가속도 300
	// 그 사이는 선형 보간
	const float MappedAcceleration = FMath::GetMappedRangeValueClamped(
		FVector2D(300.f, 700.f), // In Range (속도)
		FVector2D(800.f, 300.f), // Out Range (가속도)
		CurrentSpeed2D
	);

	// 현재의 Gait 태그를 기반으로 최종 가속도 선택
	ABaseCharacter* BaseChar = Cast<ABaseCharacter>(GetCharacterOwner());
	UMGSAbilitySystemComponent* ASC = BaseChar ? BaseChar->GetMGSAbilitySystemComponent() : nullptr;

	if (ASC)
	{
		// Sprint 상태일 때: 매핑된 가변 가속도 반환
		if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Sprint) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Sprint))
		{
			return MappedAcceleration;
		}
		// Walk나 Run 상태일 때: 고정 가속도 800 반환 (GASP 기본값)
		else if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Walk) ||
				 ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Run) ||
				 ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Walk))
		{
			return 800.f;
		}
	}

	// 기본값 (태그를 찾지 못했을 때)
	return 800.f;
}

// 입력벡터가 있으면 500, 입력을 뗐으면 2000 브레이크
float UMGSCharacterMovementComponent::CalculateBrakingDeceleration() const
{
	// 현재 입력 벡터가 존재하는지 확인
	bool bHasInput = false;

	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		// 로컬 플레이어인 경우
		bHasInput = !PawnOwner->GetPendingMovementInputVector().IsNearlyZero();
	}
	else
	{
		// AI나 리플리케이트 된 캐릭터인 경우
		bHasInput = !GetCurrentAcceleration().IsNearlyZero();
	}

	// 입력 여부에 따른 제동력 반환
	return bHasInput ? 500.f : 2000.f;
}

// sprint일때 갑자기 제동을 걸거나 방향을 꺾으려하면 조금 더 미끄러지는 관성현상
float UMGSCharacterMovementComponent::CalculateGroundFriction() const
{
	// 현재 2D 속도 가져오기
	const float CurrentSpeed2D = Velocity.Size2D();

	// 캐릭터의 상태(Tag) 확인
	ABaseCharacter* BaseChar = Cast<ABaseCharacter>(GetCharacterOwner());
	UMGSAbilitySystemComponent* ASC = BaseChar ? BaseChar->GetMGSAbilitySystemComponent() : nullptr;

	if (ASC)
	{
		// [Sprint 상태일 때] 속도에 따라 마찰력을 5.0 -> 3.0으로 줄임
		if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Sprint) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Sprint))
		{
			return FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, 500.f),   // In Range (속도)
				FVector2D(5.0f, 3.0f),   // Out Range (마찰력)
				CurrentSpeed2D
			);
		}
		// [Walk 또는 Run 상태일 때] 고정 마찰력 5.0
		else if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Walk) ||
				 ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Run) ||
				 ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Walk))
		{
			return 5.0f;
		}
	}

	// 기본값
	return 5.0f;
}

// 방향에따라 (앞 옆 뒤) 서 있을때 최대속도를 결정하는 함수
float UMGSCharacterMovementComponent::CalculateMaxSpeed() const
{
	// 현재 어떤 보행 등급(Tag)인지 확인하여 기준 속도 벡터 선택
	FVector TargetSpeedVector = RunSpeeds; // 기본값

	ABaseCharacter* BaseChar = Cast<ABaseCharacter>(GetCharacterOwner());
	UMGSAbilitySystemComponent* ASC = BaseChar ? BaseChar->GetMGSAbilitySystemComponent() : nullptr;

	if (ASC)
	{
		if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Sprint) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Sprint)) TargetSpeedVector = SprintSpeeds;
		else if (ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Player_Gait_Walk) || ASC->HasMatchingGameplayTag(MGSGameplayTags::State_Enemy_Movement_Walk)) TargetSpeedVector = WalkSpeeds;
		else TargetSpeedVector = RunSpeeds;
	}

	// 방향성 이동(Strafe)을 사용하는지 확인 (bUseControllerDesiredRotation)
	// 캐릭터가 카메라 방향을 고정하고 움직이는 모드일 때만 방향별 속도 차등 적용
	if (bUseControllerDesiredRotation)
	{
		// 현재 이동 방향(Degree) 계산 (-180 ~ 180)
		float Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, GetCharacterOwner()->GetActorRotation());

		// 커브를 통해 0~180도를 0~2 범위로 변환 (0:정면, 1:측면, 2:후면)
		float StrafeSpeedMap = 0.f;
		if (StrafeSpeedMapCurve)
		{
			StrafeSpeedMap = StrafeSpeedMapCurve->GetFloatValue(FMath::Abs(Direction));
		}

		// 0~1 사이면 정면->측면 보간, 1~2 사이면 측면->후면 보간
		if (StrafeSpeedMap < 1.0f)
		{
			return FMath::Lerp(TargetSpeedVector.X, TargetSpeedVector.Y, StrafeSpeedMap);
		}
		else
		{
			return FMath::Lerp(TargetSpeedVector.Y, TargetSpeedVector.Z, StrafeSpeedMap - 1.0f);
		}
	}

	// 방향성 이동이 아니면(Orient to Movement) 항상 정면 속도(X) 반환
	return TargetSpeedVector.X;
}

// 방향에따라 (앞 옆 뒤) 앉았을때 최대속도를 결정하는 함수
float UMGSCharacterMovementComponent::CalculateMaxCrouchSpeed() const
{
	// 앉은 상태에서는 무조건 CrouchSpeeds를 기준으로 삼습니다.
	FVector TargetSpeedVector = CrouchSpeeds;

	// 방향성 이동(Strafe)을 사용하는지 확인
	if (bUseControllerDesiredRotation)
	{
		// 현재 이동 방향(Degree) 계산 (-180 ~ 180)
		float Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, GetCharacterOwner()->GetActorRotation());

		// 커브를 통해 0~180도를 0~2 범위로 변환
		float StrafeSpeedMap = 0.f;
		if (StrafeSpeedMapCurve)
		{
			StrafeSpeedMap = StrafeSpeedMapCurve->GetFloatValue(FMath::Abs(Direction));
		}

		// 0~1 사이면 정면->측면 보간, 1~2 사이면 측면->후면 보간
		if (StrafeSpeedMap < 1.0f)
		{
			return FMath::Lerp(TargetSpeedVector.X, TargetSpeedVector.Y, StrafeSpeedMap);
		}
		else
		{
			return FMath::Lerp(TargetSpeedVector.Y, TargetSpeedVector.Z, StrafeSpeedMap - 1.0f);
		}
	}

	// 방향성 이동이 아니면 항상 정면 속도(X) 반환
	return TargetSpeedVector.X;
}

void UMGSCharacterMovementComponent::UpdateRotationMode()
{
	ABaseCharacter* BaseChar = Cast<ABaseCharacter>(GetCharacterOwner());
	UMGSAbilitySystemComponent* ASC = BaseChar ? BaseChar->GetMGSAbilitySystemComponent() : nullptr;
	if (!ASC) return;

	// [게 걸음 / 조준 모드] -> 카메라 방향을 응시함
	bOrientRotationToMovement = false;
	bUseControllerDesiredRotation = true;

	// 공중에 떠 있을 때는 천천히 회전 (Yaw: 200), 지상이면 즉시 회전 (Yaw: -1)
	if (IsFalling())
	{
		RotationRate = FRotator(0.0f, 200.0f, 0.0f);
	}
	else
	{
		RotationRate = FRotator(0.0f, -1.0f, 0.0f); // -1은 즉각적인 회전을 의미함
	}
}
