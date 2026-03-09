/*
 * 파일명 : MGSLocomotionComponent.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-06
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
 */

#include "Components/StateTag/MGSLocomotionComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/MovementComponent/MGSCharacterMovementComponent.h"
#include "GAS/MGSGameplayTags.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"

UMGSLocomotionComponent::UMGSLocomotionComponent()
{
	// 델리게이트를 통한 movementcomponent로 상태변화 통신
	PrimaryComponentTick.bCanEverTick = false;
	
	LastMovementStateTag = FGameplayTag::EmptyTag;
	LastGaitTag = FGameplayTag::EmptyTag;
	LastStanceTag = FGameplayTag::EmptyTag;
	LastModeTag = FGameplayTag::EmptyTag;

}

void UMGSLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningCharacter = Cast<ABaseCharacter>(GetOwner());
	if (OwningCharacter)
	{
		// 기본 CMC를 가져와서 커스텀 클래스로 캐스팅 후 저장
		MGSMovementComponent = Cast<UMGSCharacterMovementComponent>(OwningCharacter->GetCharacterMovement());

		if (MGSMovementComponent)
		{
			OwningCharacter->OnCharacterMovementUpdated.AddDynamic(this, &UMGSLocomotionComponent::OnMovementUpdated);
		}
	}
	if (UMGSAbilitySystemComponent* ASC = OwningCharacter->GetMGSAbilitySystemComponent())
	{
		// 당장은 플레이어에게 최고 품질인 Dense 태그를 기본값으로 부여합니다.
		// 나중에 여기서 카메라와의 거리를 계산하여 태그를 바꿀 예정입니다.
		ASC->AddLooseGameplayTag(MGSGameplayTags::State_Player_LOD_Dense);
	}
}

void UMGSLocomotionComponent::OnMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	UpdateMovementTags(DeltaSeconds);
}

void UMGSLocomotionComponent::UpdateMovementTags(float DeltaSeconds)
{
	if (!OwningCharacter || !MGSMovementComponent) return;
	UMGSAbilitySystemComponent* ASC = OwningCharacter->GetMGSAbilitySystemComponent();
	if (!ASC) return;
	
	// 1. Idle vs Moving 판정 (속도와 입력 기준)
	const bool bIsMoving = MGSMovementComponent->Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER ||
						   MGSMovementComponent->GetCurrentAcceleration().SizeSquared2D() > KINDA_SMALL_NUMBER;

	FGameplayTag NewMovementState = bIsMoving ? MGSGameplayTags::State_Player_Movement_Moving
											  : MGSGameplayTags::State_Player_Movement_Idle;

	// 2. 태그 업데이트 (LastMovementStateTag는 헤더에 추가 필요)
	if (NewMovementState != LastMovementStateTag)
	{
		if (LastMovementStateTag.IsValid()) ASC->RemoveLooseGameplayTag(LastMovementStateTag);
		ASC->AddLooseGameplayTag(NewMovementState);
		LastMovementStateTag = NewMovementState;
	}
	
	// Gait 최적화 업데이트
	FGameplayTag NewGait = MGSMovementComponent->GetDesiredGait();

	// 상태가 바뀌었을 때만 태그 작업 수행
	if (NewGait != LastGaitTag)
	{
		// 이전 태그가 유효했다면 제거
		if (LastGaitTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(LastGaitTag);
		}

		// 새 태그 추가
		ASC->AddLooseGameplayTag(NewGait);
		LastGaitTag = NewGait; // 현재 상태 기억
	}

	// Stance 최적화 업데이트
	FGameplayTag NewStance = MGSMovementComponent->IsCrouching() ? MGSGameplayTags::State_Player_Stance_Crouch : MGSGameplayTags::State_Player_Stance_Stand;
	if (NewStance != LastStanceTag)
	{
		if (LastStanceTag.IsValid()) ASC->RemoveLooseGameplayTag(LastStanceTag);
		ASC->AddLooseGameplayTag(NewStance);
		LastStanceTag = NewStance;
	}
	
	// MovementMode 업데이트
	FGameplayTag NewMode = (MGSMovementComponent->IsFalling() || MGSMovementComponent->IsFlying()) ? MGSGameplayTags::State_Player_Mode_InAir : MGSGameplayTags::State_Player_Mode_OnGround;
	if (NewMode != LastModeTag)
	{
		if (LastModeTag.IsValid()) ASC->RemoveLooseGameplayTag(LastModeTag);
		ASC->AddLooseGameplayTag(NewMode);
		LastModeTag = NewMode;
	}
	
	// 착지 태그 제거
	if (LandingTagTimer > 0.f)
	{
		LandingTagTimer -= DeltaSeconds;
		if (LandingTagTimer <= 0.f)
		{
			ASC->RemoveLooseGameplayTag(MGSGameplayTags::State_Player_JustLanded_Light);
			ASC->RemoveLooseGameplayTag(MGSGameplayTags::State_Player_JustLanded_Heavy);
		}
	}
	// 공중 체류 시간 계산
	if (MGSMovementComponent->IsFalling())
	{
		CurrentAirTime += DeltaSeconds;
	}
	else
	{
		CurrentAirTime = 0.f;
	}
}

void UMGSLocomotionComponent::HandleOnJumped()
{
	if (!OwningCharacter) return;

	// 점프 순간의 수평 속도(XY)를 캡처하여 저장
	GroundSpeedBeforeJump = OwningCharacter->GetVelocity().Size2D();

	// 점프 상태임을 나타내는 태그 부여 (필요 시)
	if (UMGSAbilitySystemComponent* ASC = OwningCharacter->GetMGSAbilitySystemComponent())
	{
		// 'WantsToJump' 같은 태그가 있다면 여기서 처리하거나,
		// GAS 어빌리티(GA_Jump)에서 태그를 붙여주도록 설계합니다.
	}
}

void UMGSLocomotionComponent::HandleOnLanded(const FVector& LandVelocity)
{
	if (!OwningCharacter) return;
	// 공중체류시간 0.15초 미만
	if (CurrentAirTime < 0.15f)
	{
		CurrentAirTime = 0.f;
		return;
	}
	UMGSAbilitySystemComponent* ASC = OwningCharacter->GetMGSAbilitySystemComponent();
	if (!ASC) return;

	// 착지 충격량 확인 
	const float FallSpeed = FMath::Abs(LandVelocity.Z);

	// Heavy vs Light 판정 및 태그 부여
	if (FallSpeed >= HeavyLandSpeedThreshold)
	{
		ASC->AddLooseGameplayTag(MGSGameplayTags::State_Player_JustLanded_Heavy);
	}
	else
	{
		ASC->AddLooseGameplayTag(MGSGameplayTags::State_Player_JustLanded_Light);
	}
	
	LandingTagTimer = 0.2f; // 0.2초 동안만 착지 애니메이션을 허용함
}

