/*
 * 파일명 : DA_StartupEnemy.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "DataAssets/Startup/DA_StartupEnemy.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/GA/EnemyGameplayAbility.h"

void UDA_StartupEnemy::GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level)
{
	Super::GiveToAbilitySystemComponent(ASC, Level);
	
	// 적 GA 목록이 비어있지 않으면
	if (!EnemyCombatAbilities.IsEmpty())
	{
		// 적 GA 목록 순회
		for (const TSubclassOf<UEnemyGameplayAbility>& AbilityClass : EnemyCombatAbilities)
		{
			if (!AbilityClass)
			{
				continue;
			}
			
			// Spec 생성
			FGameplayAbilitySpec AbilitySpec(AbilityClass);
			AbilitySpec.SourceObject = ASC->GetAvatarActor();
			AbilitySpec.Level = Level;
			
			// ASC에 어빌리티 부여
			ASC->GiveAbility(AbilitySpec);
		}
	}
}


