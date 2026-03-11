/*
 * 파일명 : BaseAnimInstance.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-05
 * 수정자 : 김동석
 * 수정일 : 2026-03-09
 */
#include "Characters/AnimInstance/BaseAnimInstance.h"
#include "Characters/BaseCharacter.h"
#include "GAS/MGSGameplayTags.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"

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

	// to do 조준모드일때는 뒷걸음질, 아닐때는 이동방향으로
	//회전 모드 확인
	//캐릭터가 카메라 방향을 바라보는(Strafe) 모드일 때만 AO를 킴.
	// if (CharacterData.RotationMode != EMGSRotationMode::LookingDirection)
	// {
	// 	return false;
	// }

	// 카메라와의 각도 차이(Yaw) 임계값 확인
	// LocomotionState의 bIsMoving 변수를 활용하여 임계값을 결정합니다.
	const bool bIsMoving = (LocomotionState.MovementStateTag == MGSGameplayTags::State_Player_Movement_Moving);
	const float AngleThreshold = bIsMoving ? 180.0f : 115.0f;

	// 이전에 EssentialValues 구조체에서 계산해둔 AOValue.X (Yaw 각도 차이)를 사용합니다.
	if (FMath::Abs(EssentialValues.AOValue.X) > AngleThreshold)
	{
		return false;
	}
	
	return true;
}

bool UBaseAnimInstance::HasLocomotionTag(FGameplayTag TagToCheck) const
{
	if (CharacterData.GameplayTags.HasTag(TagToCheck)) return true;
	
	if (LocomotionState.MovementStateTag.MatchesTag(TagToCheck)) return true;
	
	if (LocomotionState.LocomotionActionTag.MatchesTag(TagToCheck)) return true;

	return false;
}

FQuat UBaseAnimInstance::GetDesiredFacing() const
{
	// 미래 0.5초 시점의 궤적 데이터를 가져옵니다.
	FTransformTrajectorySample OutSample;
	UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(
		TrajectoryHandler.Trajectory,
		0.5f,      
		OutSample,
		false     
	);
	return OutSample.Facing;
}

bool UBaseAnimInstance::IsTurnInPlaceSteeringEnabled(const FAnimNodeReference& Node) const
{
	// 현재 선택된 DB 태그에 "TurnInPlace"가 있는지 확인
	bool bIsTurnInPlace = MMHandler.CurrentDatabaseTags.Contains(FName("TurnInPlace"));
	// 애니메이션이 활성 상태인지 확인
	bool bIsAnimActive = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimIsActive(Node);

	return bIsTurnInPlace && bIsAnimActive;
}

bool UBaseAnimInstance::IsNormalSteeringEnabled(const FAnimNodeReference& Node) const
{
	bool bIsTurnInPlace = MMHandler.CurrentDatabaseTags.Contains(FName("TurnInPlace"));
	bool bIsMovingOrAir = (LocomotionState.MovementStateTag == MGSGameplayTags::State_Player_Movement_Moving) ||
						  CharacterData.GameplayTags.HasTag(MGSGameplayTags::State_Player_Mode_InAir);
	bool bIsAnimActive = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimIsActive(Node);

	// 제자리 회전이 아니고 움직이는 중일 때만 일반 조향 활성화
	return !bIsTurnInPlace && bIsMovingOrAir && bIsAnimActive;
}

EOrientationWarpingSpace UBaseAnimInstance::Get_OrientationWarpingWarpingSpace() const
{
	bool bIsOffsetRootActive = true;

	return bIsOffsetRootActive ? EOrientationWarpingSpace::RootBoneTransform
							   : EOrientationWarpingSpace::ComponentTransform;
}

FFootPlacementPlantSettings UBaseAnimInstance::Get_FootPlacementPlantSettings() const
{
	const bool bIsStopping = MMHandler.CurrentDatabaseTags.Contains(FName("Stops"));

	return bIsStopping ? ProceduralSettings.PlantSettings_Stops
					   : ProceduralSettings.PlantSettings_Default;
}

FFootPlacementInterpolationSettings UBaseAnimInstance::Get_FootPlacementInterpolationSettings() const
{
	const bool bIsStopping = MMHandler.CurrentDatabaseTags.Contains(FName("Stops"));

	return bIsStopping ? ProceduralSettings.InterpolationSettings_Stops
					   : ProceduralSettings.InterpolationSettings_Default;
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
	
	LocomotionState.Update(this, CharacterData, EssentialValues, TrajectoryHandler, MMHandler);
}
