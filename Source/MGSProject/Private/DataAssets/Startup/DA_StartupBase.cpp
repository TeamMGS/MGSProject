/*
 * 파일명 : DA_Startup.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-01
 */

#include "DataAssets/Startup/DA_StartupBase.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/GA/BaseGameplayAbility.h"

void UDA_StartupBase::GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level)
{
	// ASC 정보가 유효한지 체크
	check(ASC);
	
	// 두 종류 모드 부여
	GrantAbilities(ActivateOnGivenAbilities, ASC, Level);
	GrantAbilities(ReactiveAbilities, ASC, Level);
}

void UDA_StartupBase::GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs,
	UMGSAbilitySystemComponent* InASC, int32 Level)
{
	// 게임플레이 어빌리티가 없으면 탈출
	if (GAs.IsEmpty())
	{
		return;
	}
	
	for (const TSubclassOf<UBaseGameplayAbility> Ability : GAs)
	{
		// 게임플레이 스펙을 만들어서 데이터 주입
		FGameplayAbilitySpec Spec(Ability);
		Spec.SourceObject = InASC->GetAvatarActor();
		Spec.Level = Level;
		
		// 주입된 스펙을 사용할 어빌리티 시스템 컴포넌트에 전달
		InASC->GiveAbility(Spec);
	}
}
