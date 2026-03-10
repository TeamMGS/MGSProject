/*
 * 파일명 : MGSStructType.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#include "MGSStructType.h"

#include "GAS/GA/PlayerGameplayAbility.h"

bool FPlayerAbilitySet::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
