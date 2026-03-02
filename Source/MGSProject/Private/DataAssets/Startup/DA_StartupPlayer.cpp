/*
 * 파일명 : DA_StartupPlayer.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "DataAssets/Startup/DA_StartupPlayer.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/GA/PlayerGameplayAbility.h"
#include "MGSStructType.h"

void UDA_StartupPlayer::GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level)
{
	Super::GiveToAbilitySystemComponent(ASC, Level);
	
	for (const FPlayerAbilitySet& AbilitySet : PlayerStartupAbilitySets)
	{
		if (!AbilitySet.IsValid())
		{
			continue;
		}
		
		FGameplayAbilitySpec Spec(AbilitySet.AbilityToGrant);
		Spec.SourceObject = ASC->GetAvatarActor();
		Spec.Level = Level;
		// 동적태그 생성
		Spec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		
		ASC->GiveAbility(Spec);
	}
}
