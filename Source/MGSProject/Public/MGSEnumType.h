/*
 * 파일명 : MGSEnumType.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 김동석
 * 수정일 : 2026-03-06
 */

#pragma once

#include "MGSEnumType.generated.h"

// 이동 모드 (언리얼 기본 EMovementMode 대응)
UENUM(BlueprintType)
enum class EMGSMovementMode : uint8
{
	OnGround, InAir
};

// 자세 (서기/앉기)
UENUM(BlueprintType)
enum class EMGSStance : uint8
{
	Stand, Crouch
};

// 회전 방식 (속도 방향/시선 방향)
UENUM(BlueprintType)
enum class EMGSRotationMode : uint8
{
	VelocityDirection, LookingDirection
};

// 보행 속도 등급
UENUM(BlueprintType)
enum class EMGSGait : uint8
{
	Walk, Run, Sprint
};

UENUM(BlueprintType)
enum class EMGSMovementState : uint8 {
	Idle, Moving
};

// 파쿠르 액션 타입 (GASP의 E_TraversalAction 대응)
UENUM(BlueprintType)
enum class EMGSTraversalActionType : uint8
{
	None,
	Hurdle,  // 낮은 장애물
	Vault,   // 중간 장애물
	Mantle   // 높은 장애물
};

/**
 * 나레이션 상황 정의
 */
UENUM(BlueprintType)
enum class ENarrationSituation : uint8
{
	None,
	GameStart,           // 게임 시작 안내
	MissionSuccess,      // 미션 목표 달성
	ExtractionReady,     // 탈출지 활성화
	ExtractionSuccess,   // 탈출 성공 (게임 클리어)
	CombatStarted,       // 교전 발생
	LowHealth            // 체력 부족 경고
};