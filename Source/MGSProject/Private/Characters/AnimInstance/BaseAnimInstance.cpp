/*
 * 파일명 : BaseAnimInstance.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-05
 */
#include "Characters/AnimInstance/BaseAnimInstance.h"
#include "Characters/BaseCharacter.h"

void UBaseAnimInstance::UpdateMotionMatchingNode(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	MMHandler.Update(this, Node);
}

void UBaseAnimInstance::PostSelectionMotionMatchingNode(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	MMHandler.PostSelection(this, Node);
}

bool UBaseAnimInstance::ShouldEnableAimOffset() const
{
	// 몽타주 재생 여부 확인
	// DefaultSlot에서 재생 중인 몽타주의 가중치가 0.5 이상이면 AO를 끔.
	if (GetSlotMontageLocalWeight(FName("DefaultSlot")) >= 0.5f)
	{
		return false;
	}

	// 회전 모드 확인
	// 캐릭터가 카메라 방향을 바라보는(Strafe) 모드일 때만 AO를 킴.
	if (CharacterData.RotationMode != EMGSRotationMode::LookingDirection)
	{
		return false;
	}

	// 카메라와의 각도 차이(Yaw) 임계값 확인
	// LocomotionState의 bIsMoving 변수를 활용하여 임계값을 결정합니다.
	const float AngleThreshold = LocomotionState.bIsMoving ? 180.0f : 115.0f;

	// 이전에 EssentialValues 구조체에서 계산해둔 AOValue.X (Yaw 각도 차이)를 사용합니다.
	if (FMath::Abs(EssentialValues.AOValue.X) > AngleThreshold)
	{
		return false;
	}
	
	return true;
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character || DeltaSeconds <= 0.f) return;
	
	CharacterData.Update(Character);

	// 나머지 모듈들도 같은 방식으로 호출
	EssentialValues.Update(this, CharacterData, DeltaSeconds);
	
	TrajectoryHandler.Update(this, EssentialValues, DeltaSeconds);
	
	LocomotionState.Update(CharacterData, TrajectoryHandler);
}
