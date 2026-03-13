/*
 * 파일명 : DA_StartupEnemy.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "DataAssets/Startup/DA_StartupEnemy.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/GA/EnemyGameplayAbility.h"

void UDA_StartupEnemy::GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level)
{
	Super::GiveToAbilitySystemComponent(ASC, Level);

	// Enemy GA 목록이 비어있으면
	if (EnemyActionAbilities.IsEmpty())
	{
		return;
	}
	
	// GA 목록 순회
	for (const TSubclassOf<UEnemyGameplayAbility> Ability : EnemyActionAbilities)
	{
		// Spec 생성
		FGameplayAbilitySpec Spec(Ability);
		Spec.SourceObject = ASC->GetAvatarActor();
		Spec.Level = Level;
		
		// ASC에 어빌리티 부여
		ASC->GiveAbility(Spec);
	}
}
